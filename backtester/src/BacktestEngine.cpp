#include "BacktestEngine.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>

BacktestResult BacktestEngine::run(const std::vector<DataPoint> &data, const Expression &strategy, double capital)
{
    std::vector<double> returns;
    std::vector<TradeResult> tradeResults;

    double totalPnL = 0;
    int wins = 0, trades = 0;
    double currentCapital = capital;
    double peakCapital = capital;
    double maxDrawdown = 0;

    double previousPosition = 0;

    // Safety check
    if (data.size() < 2) {
        return {0, 0, 0, 0, 0, tradeResults};
    }

    // First pass: analyze signal distribution to determine signal type
    std::vector<double> sampleSignals;
    for (size_t i = 1; i < std::min(data.size(), static_cast<size_t>(1000)); ++i) {
        double signal = strategy.evaluate(data[i], data, i);
        if (std::isfinite(signal)) {
            sampleSignals.push_back(signal);
        }
    }

    // Determine signal characteristics
    bool isBooleanSignal = true;
    double maxSignal = 0, minSignal = 0;
    
    if (!sampleSignals.empty()) {
        maxSignal = *std::max_element(sampleSignals.begin(), sampleSignals.end());
        minSignal = *std::min_element(sampleSignals.begin(), sampleSignals.end());

        for (double s : sampleSignals) {
            if (s != 0.0 && s != 1.0) {
                isBooleanSignal = false;
                break;
            }
        }
    }

    for (size_t i = 1; i < data.size(); ++i) {
        double signal = strategy.evaluate(data[i], data, i);
        if (!std::isfinite(signal))
            signal = 0;

        double position = calculatePositionSmart(signal, isBooleanSignal, maxSignal, minSignal);

        double priceReturn = (data[i].close - data[i-1].close) / data[i-1].close;
        if (!std::isfinite(priceReturn))
            continue;

        double maxPositionSize = 0.1; // Max 10% capital per trade
        double positionSize = position * maxPositionSize;
        double tradePnL = positionSize * priceReturn * currentCapital;

        double percentReturn = tradePnL / currentCapital;
        returns.push_back(percentReturn);
        totalPnL += tradePnL;
        currentCapital += tradePnL;

        // Drawdown calculation
        if (currentCapital > peakCapital)
            peakCapital = currentCapital;
        double drawdown = (peakCapital - currentCapital) / peakCapital;
        if (drawdown > maxDrawdown)
            maxDrawdown = drawdown;

        if (std::abs(position - previousPosition) > 0.001) {
            trades++;
            if (tradePnL > 0)
                wins++;

            TradeResult tr;
            tr.timestamp = data[i].timestamp;
            tr.entryPrice = data[i-1].close;
            tr.exitPrice = data[i].close;
            tr.pnl = tradePnL;
            tradeResults.push_back(tr);
        }

        previousPosition = position;
    }

    // Sharpe and volatility calculation
    double mean = 0, variance = 0, sharpe = 0, volatility = 0;

    if (!returns.empty()) {
        for (double r : returns)
            mean += r;
        mean /= returns.size();

        for (double r : returns)
            variance += (r - mean) * (r - mean);
        variance /= returns.size();

        double stddev = std::sqrt(variance);
        volatility = stddev * std::sqrt(252);

        if (stddev > 1e-10)
            sharpe = (mean / stddev) * std::sqrt(252);
    }

    double winRate = trades > 0 ? static_cast<double>(wins) / trades : 0;

    return {sharpe, winRate, totalPnL, maxDrawdown, volatility, tradeResults};
}

double BacktestEngine::calculatePosition(double signal)
{
    // This is the old function - keeping for compatibility
    return calculatePositionSmart(signal, false, 0, 0);
}

double BacktestEngine::calculatePositionSmart(double signal, bool isBooleanSignal, double maxSignal, double minSignal)
{
    // Handle invalid signals
    if (!std::isfinite(signal)) {
        return 0.0;
    }
    
    if (isBooleanSignal) {
        // For boolean signals (0 or 1)
        return signal; // 0 = no position, 1 = full position
    } else {
        // For continuous signals, normalize based on observed range
        if (std::abs(maxSignal - minSignal) < 1e-10) {
            return 0.0; // No variation in signal
        }
        
        // Normalize to [-1, 1] range
        double normalized = 2.0 * (signal - minSignal) / (maxSignal - minSignal) - 1.0;
        return std::max(-1.0, std::min(1.0, normalized));
    }
}