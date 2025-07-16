#pragma once
#include <vector>

struct TradeResult {
    int timestamp;
    double entryPrice;
    double exitPrice;
    double pnl;
};

struct BacktestResult {
    double sharpe;
    double winRate;
    double totalReturn;
    double maxDrawdown;
    double volatility;
    std::vector<TradeResult> tradeResults;
};
