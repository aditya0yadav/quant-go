#pragma once
#include "Expression.h"
#include <memory>

class Strategy {
public:
    Strategy(std::unique_ptr<Expression> entry, std::unique_ptr<Expression> exit);

    const Expression& getEntryCondition() const;
    const Expression& getExitCondition() const;

private:
    std::unique_ptr<Expression> entryCondition;
    std::unique_ptr<Expression> exitCondition;
};
