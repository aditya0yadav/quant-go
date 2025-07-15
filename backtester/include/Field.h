#pragma once
#include "DataPoint.h"

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
