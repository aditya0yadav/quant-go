#include "Parser.h"
#include <stdexcept>
#include <sstream>
#include <cctype>
#include <cmath>

namespace Parser {

std::vector<Token> Tokenizer::tokenize(const std::string& input) {
    std::vector<Token> tokens;
    size_t i = 0;
    
    while (i < input.length()) {
        char c = input[i];
        
        // Skip whitespace
        if (std::isspace(c)) {
            i++;
            continue;
        }
        
        // Numbers
        if (isDigit(c) || c == '.') {
            std::string num;
            while (i < input.length() && (isDigit(input[i]) || input[i] == '.')) {
                num += input[i++];
            }
            tokens.emplace_back(Token::Type::CONSTANT, num, std::stod(num));
            continue;
        }
        
        // Identifiers (fields and functions)
        if (isAlpha(c)) {
            std::string ident;
            while (i < input.length() && isAlphaNum(input[i])) {
                ident += input[i++];
            }
            
            // Check if it's a field
            if (ident == "open" || ident == "close" || ident == "high" || 
                ident == "low" || ident == "volume") {
                tokens.emplace_back(Token::Type::FIELD, ident);
            } else {
                tokens.emplace_back(Token::Type::FUNCTION, ident);
            }
            continue;
        }
        
        // Multi-character operators
        if (i + 1 < input.length()) {
            std::string twoChar = input.substr(i, 2);
            if (twoChar == ">=" || twoChar == "<=" || twoChar == "==" || 
                twoChar == "!=" || twoChar == "&&" || twoChar == "||") {
                tokens.emplace_back(Token::Type::OPERATOR, twoChar);
                i += 2;
                continue;
            }
        }
        
        // Single character tokens
        if (isOperator(c)) {
            tokens.emplace_back(Token::Type::OPERATOR, std::string(1, c));
            i++;
        } else if (c == '(') {
            tokens.emplace_back(Token::Type::LPAREN, "(");
            i++;
        } else if (c == ')') {
            tokens.emplace_back(Token::Type::RPAREN, ")");
            i++;
        } else if (c == ',') {
            tokens.emplace_back(Token::Type::COMMA, ",");
            i++;
        } else {
            throw std::invalid_argument("Unexpected character: " + std::string(1, c));
        }
    }
    
    tokens.emplace_back(Token::Type::END, "");
    return tokens;
}

bool Tokenizer::isOperator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || 
           c == '>' || c == '<' || c == '=' || c == '!';
}

bool Tokenizer::isDigit(char c) {
    return c >= '0' && c <= '9';
}

bool Tokenizer::isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Tokenizer::isAlphaNum(char c) {
    return isAlpha(c) || isDigit(c);
}

std::unique_ptr<Expression> ExpressionParser::parse(const std::string& input) {
    Tokenizer tokenizer;
    tokens = tokenizer.tokenize(input);
    pos = 0;
    return parseExpression();
}

Token& ExpressionParser::current() {
    return tokens[pos];
}

Token& ExpressionParser::peek() {
    return tokens[pos + 1];
}

void ExpressionParser::advance() {
    if (pos < tokens.size() - 1) pos++;
}

bool ExpressionParser::match(Token::Type type) {
    if (current().type == type) {
        advance();
        return true;
    }
    return false;
}

std::unique_ptr<Expression> ExpressionParser::parseExpression() {
    return parseLogicalOr();
}

std::unique_ptr<Expression> ExpressionParser::parseLogicalOr() {
    auto left = parseLogicalAnd();
    
    while (current().type == Token::Type::OPERATOR && current().value == "||") {
        advance();
        auto right = parseLogicalAnd();
        left = std::make_unique<BinaryExpression>(std::move(left), std::move(right), BinaryExpression::Op::OR);
    }
    
    return left;
}

std::unique_ptr<Expression> ExpressionParser::parseLogicalAnd() {
    auto left = parseComparison();
    
    while (current().type == Token::Type::OPERATOR && current().value == "&&") {
        advance();
        auto right = parseComparison();
        left = std::make_unique<BinaryExpression>(std::move(left), std::move(right), BinaryExpression::Op::AND);
    }
    
    return left;
}

std::unique_ptr<Expression> ExpressionParser::parseComparison() {
    auto left = parseArithmetic();
    
    while (current().type == Token::Type::OPERATOR && 
           (current().value == ">" || current().value == "<" || 
            current().value == ">=" || current().value == "<=" || 
            current().value == "==" || current().value == "!=")) {
        std::string op = current().value;
        advance();
        auto right = parseArithmetic();
        left = std::make_unique<BinaryExpression>(std::move(left), std::move(right), parseOperator(op));
    }
    
    return left;
}

std::unique_ptr<Expression> ExpressionParser::parseArithmetic() {
    auto left = parseTerm();
    
    while (current().type == Token::Type::OPERATOR && 
           (current().value == "+" || current().value == "-")) {
        std::string op = current().value;
        advance();
        auto right = parseTerm();
        left = std::make_unique<BinaryExpression>(std::move(left), std::move(right), parseOperator(op));
    }
    
    return left;
}

std::unique_ptr<Expression> ExpressionParser::parseTerm() {
    auto left = parseUnary();
    
    while (current().type == Token::Type::OPERATOR && 
           (current().value == "*" || current().value == "/")) {
        std::string op = current().value;
        advance();
        auto right = parseUnary();
        left = std::make_unique<BinaryExpression>(std::move(left), std::move(right), parseOperator(op));
    }
    
    return left;
}

std::unique_ptr<Expression> ExpressionParser::parseUnary() {
    if (current().type == Token::Type::OPERATOR && current().value == "-") {
        advance();
        auto expr = parseUnary();
        return std::make_unique<UnaryExpression>(UnaryExpression::Op::NEG, std::move(expr));
    }
    
    return parsePrimary();
}

std::unique_ptr<Expression> ExpressionParser::parsePrimary() {
    if (current().type == Token::Type::CONSTANT) {
        double value = current().numValue;
        advance();
        return std::make_unique<ConstantExpr>(value);
    }
    
    if (current().type == Token::Type::FIELD) {
        std::string fieldName = current().value;
        advance();
        auto fieldType = parseFieldType(fieldName);
        return std::make_unique<FieldExpr>(Field(fieldType));
    }
    
    if (current().type == Token::Type::FUNCTION) {
        std::string funcName = current().value;
        return parseFunction(funcName);
    }
    
    if (current().type == Token::Type::LPAREN) {
        advance(); // consume '('
        auto expr = parseExpression();
        if (!match(Token::Type::RPAREN)) {
            throw std::invalid_argument("Expected closing parenthesis");
        }
        return expr;
    }
    
    throw std::invalid_argument("Unexpected token: " + current().value);
}

std::unique_ptr<Expression> ExpressionParser::parseFunction(const std::string& funcName) {
    advance(); // consume function name
    
    if (!match(Token::Type::LPAREN)) {
        throw std::invalid_argument("Expected opening parenthesis after function name");
    }
    
    auto funcType = parseFunctionType(funcName);
    
    if (funcType == FunctionExpression::FuncType::SMA || funcType == FunctionExpression::FuncType::EMA) {
        auto expr = parseExpression();
        
        if (!match(Token::Type::COMMA)) {
            throw std::invalid_argument("Expected comma in function arguments");
        }
        
        if (current().type != Token::Type::CONSTANT) {
            throw std::invalid_argument("Expected period as second argument");
        }
        
        int period = static_cast<int>(current().numValue);
        advance();
        
        if (!match(Token::Type::RPAREN)) {
            throw std::invalid_argument("Expected closing parenthesis");
        }
        
        return std::make_unique<FunctionExpression>(funcType, std::move(expr), period);
    } else {
        auto expr = parseExpression();
        
        if (!match(Token::Type::RPAREN)) {
            throw std::invalid_argument("Expected closing parenthesis");
        }
        
        return std::make_unique<FunctionExpression>(funcType, std::move(expr));
    }
}

BinaryExpression::Op ExpressionParser::parseOperator(const std::string& opStr) {
    if (opStr == "+") return BinaryExpression::Op::ADD;
    if (opStr == "-") return BinaryExpression::Op::SUB;
    if (opStr == "*") return BinaryExpression::Op::MUL;
    if (opStr == "/") return BinaryExpression::Op::DIV;
    if (opStr == ">") return BinaryExpression::Op::GT;
    if (opStr == "<") return BinaryExpression::Op::LT;
    if (opStr == ">=") return BinaryExpression::Op::GTE;
    if (opStr == "<=") return BinaryExpression::Op::LTE;
    if (opStr == "==") return BinaryExpression::Op::EQ;
    if (opStr == "!=") return BinaryExpression::Op::NEQ;
    if (opStr == "&&") return BinaryExpression::Op::AND;
    if (opStr == "||") return BinaryExpression::Op::OR;
    throw std::invalid_argument("Unknown operator: " + opStr);
}

Field::Type ExpressionParser::parseFieldType(const std::string& fieldStr) {
    if (fieldStr == "open") return Field::Type::OPEN;
    if (fieldStr == "close") return Field::Type::CLOSE;
    if (fieldStr == "high") return Field::Type::HIGH;
    if (fieldStr == "low") return Field::Type::LOW;
    if (fieldStr == "volume") return Field::Type::VOLUME;
    throw std::invalid_argument("Unknown field: " + fieldStr);
}

FunctionExpression::FuncType ExpressionParser::parseFunctionType(const std::string& funcStr) {
    if (funcStr == "SMA" || funcStr == "sma") return FunctionExpression::FuncType::SMA;
    if (funcStr == "EMA" || funcStr == "ema") return FunctionExpression::FuncType::EMA;
    if (funcStr == "log" || funcStr == "LOG") return FunctionExpression::FuncType::LOG;
    if (funcStr == "exp" || funcStr == "EXP") return FunctionExpression::FuncType::EXP;
    if (funcStr == "abs" || funcStr == "ABS") return FunctionExpression::FuncType::ABS;
    if (funcStr == "sqrt" || funcStr == "SQRT") return FunctionExpression::FuncType::SQRT;
    throw std::invalid_argument("Unknown function: " + funcStr);
}

std::unique_ptr<Expression> parseExpression(const std::string& input) {
    ExpressionParser parser;
    return parser.parse(input);
}

}