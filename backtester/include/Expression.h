#pragma once
#include "DataPoint.h"
#include <memory>
#include <vector>
#include <string>
#include <cmath>

class Expression {
public:
    virtual double evaluate(const DataPoint& dp, const std::vector<DataPoint>& history, size_t currentIndex) const = 0;
    virtual ~Expression() = default;
};

class Field {
public:
    enum class Type { OPEN, CLOSE, HIGH, LOW, VOLUME };

    Field(Type t) : type(t) {}

    double evaluate(const DataPoint& dp) const {
        switch(type) {
            case Type::OPEN: return dp.open;
            case Type::CLOSE: return dp.close;
            case Type::HIGH: return dp.high;
            case Type::LOW: return dp.low;
            case Type::VOLUME: return dp.volume;
        }
        return 0.0;
    }

private:
    Type type;
};

class FieldExpr : public Expression {
public:
    FieldExpr(Field f) : field(f) {}

    double evaluate(const DataPoint& dp, const std::vector<DataPoint>& history, size_t currentIndex) const override {
        return field.evaluate(dp);
    }

private:
    Field field;
};

class ConstantExpr : public Expression {
public:
    ConstantExpr(double val) : value(val) {}

    double evaluate(const DataPoint& dp, const std::vector<DataPoint>& history, size_t currentIndex) const override {
        return value;
    }

private:
    double value;
};

class UnaryExpression : public Expression {
public:
    enum class Op { NEG };

    UnaryExpression(Op o, std::unique_ptr<Expression> e)
        : op(o), expr(std::move(e)) {}

    double evaluate(const DataPoint& dp, const std::vector<DataPoint>& history, size_t currentIndex) const override {
        switch(op) {
            case Op::NEG: return -expr->evaluate(dp, history, currentIndex);
        }
        return 0.0;
    }

private:
    Op op;
    std::unique_ptr<Expression> expr;
};

class BinaryExpression : public Expression {
public:
    enum class Op { ADD, SUB, MUL, DIV, GT, LT, GTE, LTE, EQ, NEQ, AND, OR };

    BinaryExpression(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r, Op o)
        : left(std::move(l)), right(std::move(r)), op(o) {}

    double evaluate(const DataPoint& dp, const std::vector<DataPoint>& history, size_t currentIndex) const override {
        double lv = left->evaluate(dp, history, currentIndex);
        double rv = right->evaluate(dp, history, currentIndex);
        
        switch(op) {
            case Op::ADD: return lv + rv;
            case Op::SUB: return lv - rv;
            case Op::MUL: return lv * rv;
            case Op::DIV: return rv != 0 ? lv / rv : 0.0;
            case Op::GT: return lv > rv ? 1.0 : 0.0;
            case Op::LT: return lv < rv ? 1.0 : 0.0;
            case Op::GTE: return lv >= rv ? 1.0 : 0.0;
            case Op::LTE: return lv <= rv ? 1.0 : 0.0;
            case Op::EQ: return std::abs(lv - rv) < 1e-9 ? 1.0 : 0.0;
            case Op::NEQ: return std::abs(lv - rv) >= 1e-9 ? 1.0 : 0.0;
            case Op::AND: return (lv != 0.0 && rv != 0.0) ? 1.0 : 0.0;
            case Op::OR: return (lv != 0.0 || rv != 0.0) ? 1.0 : 0.0;
        }
        return 0.0;
    }

private:
    std::unique_ptr<Expression> left, right;
    Op op;
};

class FunctionExpression : public Expression {
public:
    enum class FuncType { SMA, EMA, LOG, EXP, ABS, SQRT };

    FunctionExpression(FuncType f, std::unique_ptr<Expression> e, int period = 0)
        : func(f), expr(std::move(e)), period(period) {}

    double evaluate(const DataPoint& dp, const std::vector<DataPoint>& history, size_t currentIndex) const override {
        switch(func) {
            case FuncType::SMA:
                return calculateSMA(history, currentIndex);
            case FuncType::EMA:
                return calculateEMA(history, currentIndex);
            case FuncType::LOG: {
                double val = expr->evaluate(dp, history, currentIndex);
                return val > 0 ? std::log(val) : 0.0;
            }
            case FuncType::EXP: {
                double val = expr->evaluate(dp, history, currentIndex);
                return std::exp(val);
            }
            case FuncType::ABS: {
                double val = expr->evaluate(dp, history, currentIndex);
                return std::abs(val);
            }
            case FuncType::SQRT: {
                double val = expr->evaluate(dp, history, currentIndex);
                return val >= 0 ? std::sqrt(val) : 0.0;
            }
        }
        return 0.0;
    }

private:
    FuncType func;
    std::unique_ptr<Expression> expr;
    int period;

    double calculateSMA(const std::vector<DataPoint>& history, size_t currentIndex) const {
        if (period <= 0 || currentIndex < period - 1) return 0.0;
        
        double sum = 0.0;
        for (int i = 0; i < period; ++i) {
            sum += expr->evaluate(history[currentIndex - i], history, currentIndex - i);
        }
        return sum / period;
    }

    double calculateEMA(const std::vector<DataPoint>& history, size_t currentIndex) const {
        if (period <= 0 || currentIndex == 0) {
            return expr->evaluate(history[currentIndex], history, currentIndex);
        }
        
        double alpha = 2.0 / (period + 1);
        double currentValue = expr->evaluate(history[currentIndex], history, currentIndex);
        
        if (currentIndex < period - 1) {
            // Not enough data for full EMA, use SMA
            return calculateSMA(history, currentIndex);
        }
        
        // Calculate previous EMA
        double prevEMA = calculateEMA(history, currentIndex - 1);
        return alpha * currentValue + (1 - alpha) * prevEMA;
    }
};

class ConditionalExpression : public Expression {
public:
    ConditionalExpression(std::unique_ptr<Expression> cond, 
                         std::unique_ptr<Expression> trueExpr, 
                         std::unique_ptr<Expression> falseExpr)
        : condition(std::move(cond)), trueBranch(std::move(trueExpr)), falseBranch(std::move(falseExpr)) {}

    double evaluate(const DataPoint& dp, const std::vector<DataPoint>& history, size_t currentIndex) const override {
        double condValue = condition->evaluate(dp, history, currentIndex);
        if (condValue != 0.0) {
            return trueBranch->evaluate(dp, history, currentIndex);
        } else {
            return falseBranch->evaluate(dp, history, currentIndex);
        }
    }

private:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Expression> trueBranch;
    std::unique_ptr<Expression> falseBranch;
};