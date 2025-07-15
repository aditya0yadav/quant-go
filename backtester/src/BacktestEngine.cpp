#include "BacktestEngine.h"
#include <cmath>
#include <iostream>

BacktestResult BacktestEngine::run(const std::vector<DataPoint>& data, const Expression& strategy, double capital) {
    std::vector<double> returns;
    double totalPnL = 0;
    int wins = 0, trades = 0;
    double currentCapital = capital;

    for (size_t i = 0; i < data.size(); ++i) {
        double signal = strategy.evaluate(data[i], data, i);
        
        if (i == 0) continue; 
        
        double position = calculatePosition(signal, currentCapital);
        
        if (std::abs(position) > 0.01) {
            double ret = (data[i].close - data[i].open) / data[i].open;
            double pnl = position * ret;
            
            returns.push_back(pnl);
            totalPnL += pnl;
            currentCapital += pnl;
            
            if (pnl > 0) wins++;
            trades++;
        }
    }

    // Calculate Sharpe ratio
    double mean = 0, stddev = 0;
    if (!returns.empty()) {
        for (double r : returns) mean += r;
        mean /= returns.size();

        for (double r : returns) stddev += (r - mean) * (r - mean);
        stddev = std::sqrt(stddev / returns.size());
    }

    double sharpe = stddev != 0 ? mean / stddev : 0;
    double winRate = trades != 0 ? (double)wins / trades : 0;

    return { sharpe, winRate, totalPnL };
}

double BacktestEngine::calculatePosition(double signal, double capital) {
    
    double maxPosition = capital;
    
    if (signal > 1.0) signal = 1.0;
    if (signal < -1.0) signal = -1.0;
    
    return signal * maxPosition;
}