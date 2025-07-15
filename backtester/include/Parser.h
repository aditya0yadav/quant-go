#pragma once
#include "Expression.h"
#include <memory>
#include <string>
#include <vector>

namespace Parser {
    
    struct Token {
        enum class Type { 
            FIELD, CONSTANT, OPERATOR, FUNCTION, LPAREN, RPAREN, COMMA, END 
        };
        
        Type type;
        std::string value;
        double numValue;
        
        Token(Type t, const std::string& v = "", double n = 0) 
            : type(t), value(v), numValue(n) {}
    };

    class Tokenizer {
    public:
        std::vector<Token> tokenize(const std::string& input);
        
    private:
        bool isOperator(char c);
        bool isDigit(char c);
        bool isAlpha(char c);
        bool isAlphaNum(char c);
    };

    class ExpressionParser {
    public:
        std::unique_ptr<Expression> parse(const std::string& input);
        
    private:
        std::vector<Token> tokens;
        size_t pos;
        
        Token& current();
        Token& peek();
        void advance();
        bool match(Token::Type type);
        
        std::unique_ptr<Expression> parseExpression();
        std::unique_ptr<Expression> parseLogicalOr();
        std::unique_ptr<Expression> parseLogicalAnd();
        std::unique_ptr<Expression> parseComparison();
        std::unique_ptr<Expression> parseArithmetic();
        std::unique_ptr<Expression> parseTerm();
        std::unique_ptr<Expression> parseUnary();
        std::unique_ptr<Expression> parsePrimary();
        std::unique_ptr<Expression> parseFunction(const std::string& funcName);
        
        BinaryExpression::Op parseOperator(const std::string& opStr);
        Field::Type parseFieldType(const std::string& fieldStr);
        FunctionExpression::FuncType parseFunctionType(const std::string& funcStr);
    };


    std::unique_ptr<Expression> parseExpression(const std::string& input);
}