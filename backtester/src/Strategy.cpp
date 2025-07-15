#include "Strategy.h"

Strategy::Strategy(std::unique_ptr<Expression> entry, std::unique_ptr<Expression> exit)
    : entryCondition(std::move(entry)), exitCondition(std::move(exit)) {}

const Expression& Strategy::getEntryCondition() const {
    return *entryCondition;
}

const Expression& Strategy::getExitCondition() const {
    return *exitCondition;
}
