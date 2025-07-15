#pragma once
#include "DataPoint.h"
#include "BacktestResult.h"
#include "Expression.h"
#include <vector>

class BacktestEngine {
public:
    BacktestResult run(const std::vector<DataPoint>& data, const Expression& strategy, double capital);
    
private:
    double calculatePosition(double signal, double capital);
};