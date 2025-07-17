#include "BacktestEngine.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <numeric>
#include <unordered_map>

BacktestResult BacktestEngine::run(const std::vector<DataPoint> &data, const Expression &strategy, double capital)
{
    std::vector<double> returns;
    std::vector<TradeResult> tradeResults;
    std::vector<double> equity;
    std::vector<double> signals;
    std::vector<double> drawdowns;

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

    // First pass: collect all signals for analysis
    signals.reserve(data.size());
    for (size_t i = 0; i < data.size(); ++i) {
        double signal = strategy.evaluate(data[i], data, i);
        signals.push_back(std::isfinite(signal) ? signal : 0.0);
    }

    // Analyze signal characteristics
    std::vector<double> validSignals;
    for (double s : signals) {
        if (std::isfinite(s)) validSignals.push_back(s);
    }

    bool isBooleanSignal = true;
    double maxSignal = 0, minSignal = 0;
    
    if (!validSignals.empty()) {
        maxSignal = *std::max_element(validSignals.begin(), validSignals.end());
        minSignal = *std::min_element(validSignals.begin(), validSignals.end());

        for (double s : validSignals) {
            if (s != 0.0 && s != 1.0) {
                isBooleanSignal = false;
                break;
            }
        }
    }

    // Apply signal smoothing if enabled
    if (m_useSmoothing) {
        for (size_t i = 0; i < signals.size(); ++i) {
            signals[i] = smoothSignal(signals, i, m_smoothingWindow);
        }
    }

    // Track consecutive wins/losses for advanced metrics
    int consecutiveWins = 0, consecutiveLosses = 0;
    int maxConsecutiveWins = 0, maxConsecutiveLosses = 0;
    
    equity.push_back(currentCapital);

    // Main backtesting loop
    for (size_t i = 1; i < data.size(); ++i) {
        double signal = signals[i];
        double position = calculatePositionSmart(signal, isBooleanSignal, maxSignal, minSignal);
        
        // Apply risk management
        position = applyRiskManagement(position, currentCapital, peakCapital);

        double priceReturn = (data[i].close - data[i-1].close) / data[i-1].close;
        if (!std::isfinite(priceReturn)) continue;

        // Calculate transaction costs
        double positionChange = std::abs(position - previousPosition);
        double transactionCost = positionChange * (m_commission + m_slippage) * currentCapital;

        // Calculate P&L
        double positionSize = position * m_maxPositionSize;
        double tradePnL = positionSize * priceReturn * currentCapital - transactionCost;

        double percentReturn = tradePnL / currentCapital;
        returns.push_back(percentReturn);
        totalPnL += tradePnL;
        currentCapital += tradePnL;
        equity.push_back(currentCapital);

        // Drawdown calculation
        if (currentCapital > peakCapital) {
            peakCapital = currentCapital;
        }
        double drawdown = (peakCapital - currentCapital) / peakCapital;
        drawdowns.push_back(drawdown);
        if (drawdown > maxDrawdown) {
            maxDrawdown = drawdown;
        }

        // Track trades and consecutive wins/losses
        if (std::abs(position - previousPosition) > 0.001) {
            trades++;
            TradeResult tr;
            tr.timestamp = data[i].timestamp;
            tr.entryPrice = data[i-1].close;
            tr.exitPrice = data[i].close;
            tr.pnl = tradePnL;
            tradeResults.push_back(tr);

            if (tradePnL > 0) {
                wins++;
                consecutiveWins++;
                consecutiveLosses = 0;
                maxConsecutiveWins = std::max(maxConsecutiveWins, consecutiveWins);
            } else {
                consecutiveLosses++;
                consecutiveWins = 0;
                maxConsecutiveLosses = std::max(maxConsecutiveLosses, consecutiveLosses);
            }
        }

        previousPosition = position;
    }

    // Calculate enhanced metrics
    double mean = 0, variance = 0, sharpe = 0, volatility = 0;

    if (!returns.empty()) {
        mean = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();
        
        for (double r : returns) {
            variance += (r - mean) * (r - mean);
        }
        variance /= returns.size();

        double stddev = std::sqrt(variance);
        volatility = stddev * std::sqrt(252);

        if (stddev > 1e-10) {
            sharpe = (mean / stddev) * std::sqrt(252);
        }
    }

    double winRate = trades > 0 ? static_cast<double>(wins) / trades : 0;

    // Create enhanced result with additional metrics
    BacktestResult result;
    result.sharpe = sharpe;
    result.winRate = winRate;
    result.totalReturn = totalPnL;
    result.maxDrawdown = maxDrawdown;
    result.volatility = volatility;
    result.tradeResults = tradeResults;
    
    // Calculate additional metrics
    result.pnlScore = calculatePnLScore(tradeResults, capital);
    result.correlationAnalysis = calculateCorrelationAnalysis(data, strategy);
    result.riskMetrics = calculateRiskMetrics(returns, equity);

    return result;
}

PnLScore BacktestEngine::calculatePnLScore(const std::vector<TradeResult>& trades, double initialCapital) const {
    PnLScore score = {};
    
    if (trades.empty()) return score;

    std::vector<double> pnls;
    std::vector<double> winningTrades;
    std::vector<double> losingTrades;
    double totalPnL = 0;
    double totalWins = 0;
    double totalLosses = 0;

    for (const auto& trade : trades) {
        pnls.push_back(trade.pnl);
        totalPnL += trade.pnl;
        
        if (trade.pnl > 0) {
            winningTrades.push_back(trade.pnl);
            totalWins += trade.pnl;
        } else {
            losingTrades.push_back(trade.pnl);
            totalLosses += std::abs(trade.pnl);
        }
    }

    score.totalPnL = totalPnL;
    score.avgPnL = totalPnL / trades.size();
    
    // Calculate standard deviation
    double variance = 0;
    for (double pnl : pnls) {
        variance += (pnl - score.avgPnL) * (pnl - score.avgPnL);
    }
    score.pnlStdDev = std::sqrt(variance / trades.size());

    // Calculate skewness and kurtosis
    score.pnlSkewness = calculateSkewness(pnls);
    score.pnlKurtosis = calculateKurtosis(pnls);

    // Calculate ratios
    score.profitFactor = totalLosses > 0 ? totalWins / totalLosses : std::numeric_limits<double>::infinity();
    score.hitRatio = static_cast<double>(winningTrades.size()) / trades.size();
    
    if (!winningTrades.empty()) {
        score.avgWinSize = totalWins / winningTrades.size();
    }
    if (!losingTrades.empty()) {
        score.avgLossSize = totalLosses / losingTrades.size();
    }
    
    score.winLossRatio = (score.avgLossSize > 0) ? score.avgWinSize / score.avgLossSize : 0;
    score.payoffRatio = score.winLossRatio;

    // Calculate consecutive wins/losses
    int consecutiveWins = 0, consecutiveLosses = 0;
    int maxWins = 0, maxLosses = 0;
    
    for (const auto& trade : trades) {
        if (trade.pnl > 0) {
            consecutiveWins++;
            consecutiveLosses = 0;
            maxWins = std::max(maxWins, consecutiveWins);
        } else {
            consecutiveLosses++;
            consecutiveWins = 0;
            maxLosses = std::max(maxLosses, consecutiveLosses);
        }
    }
    
    score.maxConsecutiveWins = maxWins;
    score.maxConsecutiveLosses = maxLosses;

    return score;
}

CorrelationAnalysis BacktestEngine::calculateCorrelationAnalysis(const std::vector<DataPoint>& data, 
                                                               const Expression& strategy) const {
    CorrelationAnalysis analysis = {};
    
    if (data.size() < 2) return analysis;

    std::vector<double> signals;
    std::vector<double> priceChanges;
    std::vector<double> returns;
    std::vector<double> volumes;

    // Collect data for correlation analysis
    for (size_t i = 1; i < data.size(); ++i) {
        double signal = strategy.evaluate(data[i], data, i);
        if (std::isfinite(signal)) {
            signals.push_back(signal);
            
            double priceChange = data[i].close - data[i-1].close;
            priceChanges.push_back(priceChange);
            
            double return_val = (data[i].close - data[i-1].close) / data[i-1].close;
            returns.push_back(return_val);
            
            volumes.push_back(data[i].volume);
        }
    }

    if (signals.size() < 2) return analysis;

    // Calculate correlations
    analysis.signalPriceCorrelation = calculateCorrelation(signals, priceChanges);
    analysis.signalReturnCorrelation = calculateCorrelation(signals, returns);
    analysis.signalVolumeCorrelation = calculateCorrelation(signals, volumes);

    // Calculate autocorrelations
    analysis.autocorrelationLag1 = calculateAutocorrelation(signals, 1);
    analysis.autocorrelationLag5 = calculateAutocorrelation(signals, 5);

    // Calculate rolling correlation
    const int windowSize = 50;
    analysis.rollingCorrelation.reserve(signals.size() - windowSize + 1);
    
    for (size_t i = windowSize - 1; i < signals.size(); ++i) {
        std::vector<double> windowSignals(signals.begin() + i - windowSize + 1, signals.begin() + i + 1);
        std::vector<double> windowReturns(returns.begin() + i - windowSize + 1, returns.begin() + i + 1);
        
        double corr = calculateCorrelation(windowSignals, windowReturns);
        analysis.rollingCorrelation.push_back(corr);
    }

    // Calculate average and stability of rolling correlation
    if (!analysis.rollingCorrelation.empty()) {
        analysis.avgRollingCorrelation = std::accumulate(analysis.rollingCorrelation.begin(), 
                                                       analysis.rollingCorrelation.end(), 0.0) / 
                                       analysis.rollingCorrelation.size();
        
        double variance = 0;
        for (double corr : analysis.rollingCorrelation) {
            variance += (corr - analysis.avgRollingCorrelation) * (corr - analysis.avgRollingCorrelation);
        }
        analysis.correlationStability = std::sqrt(variance / analysis.rollingCorrelation.size());
    }

    return analysis;
}

RiskMetrics BacktestEngine::calculateRiskMetrics(const std::vector<double>& returns, 
                                               const std::vector<double>& equity) const {
    RiskMetrics metrics = {};
    
    if (returns.empty() || equity.empty()) return metrics;

    // Calculate VaR
    std::vector<double> sortedReturns = returns;
    std::sort(sortedReturns.begin(), sortedReturns.end());
    
    metrics.valueAtRisk95 = calculatePercentile(sortedReturns, 0.05);
    metrics.valueAtRisk99 = calculatePercentile(sortedReturns, 0.01);

    // Calculate Expected Shortfall (Conditional VaR)
    int var95Index = static_cast<int>(sortedReturns.size() * 0.05);
    if (var95Index > 0) {
        double sum = 0;
        for (int i = 0; i < var95Index; ++i) {
            sum += sortedReturns[i];
        }
        metrics.expectedShortfall = sum / var95Index;
    }

    // Calculate drawdown metrics
    std::vector<double> drawdowns;
    double peak = equity[0];
    double totalDrawdownDuration = 0;
    int drawdownPeriods = 0;
    
    for (size_t i = 1; i < equity.size(); ++i) {
        if (equity[i] > peak) {
            peak = equity[i];
        }
        
        double drawdown = (peak - equity[i]) / peak;
        drawdowns.push_back(drawdown);
        
        if (drawdown > 0) {
            totalDrawdownDuration++;
        }
        
        if (drawdown > metrics.maxDrawdown) {
            metrics.maxDrawdown = drawdown;
        }
    }

    if (!drawdowns.empty()) {
        metrics.avgDrawdown = std::accumulate(drawdowns.begin(), drawdowns.end(), 0.0) / drawdowns.size();
        metrics.drawdownDuration = totalDrawdownDuration / static_cast<double>(drawdowns.size());
        
        // Calculate Ulcer Index
        double sumSquaredDrawdowns = 0;
        for (double dd : drawdowns) {
            sumSquaredDrawdowns += dd * dd;
        }
        metrics.ulcerIndex = std::sqrt(sumSquaredDrawdowns / drawdowns.size());
    }

    // Calculate additional ratios
    double totalReturn = (equity.back() - equity.front()) / equity.front();
    if (metrics.ulcerIndex > 0) {
        metrics.gainToPainRatio = totalReturn / metrics.ulcerIndex;
    }
    if (metrics.avgDrawdown > 0) {
        metrics.sterlingRatio = totalReturn / metrics.avgDrawdown;
    }

    return metrics;
}

void BacktestEngine::setRiskParameters(double maxPositionSize, double maxDrawdownLimit, bool useRiskManagement) {
    m_maxPositionSize = maxPositionSize;
    m_maxDrawdownLimit = maxDrawdownLimit;
    m_useRiskManagement = useRiskManagement;
}

void BacktestEngine::setTransactionCosts(double commission, double slippage) {
    m_commission = commission;
    m_slippage = slippage;
}

void BacktestEngine::setSignalSmoothing(int windowSize, bool useSmoothing) {
    m_smoothingWindow = windowSize;
    m_useSmoothing = useSmoothing;
}

double BacktestEngine::calculatePosition(double signal) {
    return calculatePositionSmart(signal, false, 0, 0);
}

double BacktestEngine::calculatePositionSmart(double signal, bool isBooleanSignal, double maxSignal, double minSignal) {
    if (!std::isfinite(signal)) {
        return 0.0;
    }
    
    if (isBooleanSignal) {
        return signal;
    } else {
        if (std::abs(maxSignal - minSignal) < 1e-10) {
            return 0.0;
        }
        
        double normalized = 2.0 * (signal - minSignal) / (maxSignal - minSignal) - 1.0;
        return std::max(-1.0, std::min(1.0, normalized));
    }
}

double BacktestEngine::applyRiskManagement(double position, double currentCapital, double peakCapital) {
    if (!m_useRiskManagement) return position;
    
    // Reduce position size if in drawdown
    double drawdown = (peakCapital - currentCapital) / peakCapital;
    if (drawdown > m_maxDrawdownLimit) {
        position *= (1.0 - drawdown / m_maxDrawdownLimit);
    }
    
    return position;
}

double BacktestEngine::smoothSignal(const std::vector<double>& signals, size_t index, int windowSize) const {
    if (windowSize <= 1 || index < static_cast<size_t>(windowSize - 1)) {
        return signals[index];
    }
    
    double sum = 0;
    for (int i = 0; i < windowSize; ++i) {
        sum += signals[index - i];
    }
    
    return sum / windowSize;
}

double BacktestEngine::calculateCorrelation(const std::vector<double>& x, const std::vector<double>& y) const {
    if (x.size() != y.size() || x.size() < 2) return 0.0;
    
    double meanX = std::accumulate(x.begin(), x.end(), 0.0) / x.size();
    double meanY = std::accumulate(y.begin(), y.end(), 0.0) / y.size();
    
    double numerator = 0, denomX = 0, denomY = 0;
    
    for (size_t i = 0; i < x.size(); ++i) {
        double diffX = x[i] - meanX;
        double diffY = y[i] - meanY;
        numerator += diffX * diffY;
        denomX += diffX * diffX;
        denomY += diffY * diffY;
    }
    
    double denom = std::sqrt(denomX * denomY);
    return (denom > 1e-10) ? numerator / denom : 0.0;
}

double BacktestEngine::calculateSkewness(const std::vector<double>& data) const {
    if (data.size() < 3) return 0.0;
    
    double mean = std::accumulate(data.begin(), data.end(), 0.0) / data.size();
    double variance = 0, skewness = 0;
    
    for (double x : data) {
        double diff = x - mean;
        variance += diff * diff;
        skewness += diff * diff * diff;
    }
    
    variance /= data.size();
    skewness /= data.size();
    
    double stddev = std::sqrt(variance);
    return (stddev > 1e-10) ? skewness / (stddev * stddev * stddev) : 0.0;
}

double BacktestEngine::calculateKurtosis(const std::vector<double>& data) const {
    if (data.size() < 4) return 0.0;
    
    double mean = std::accumulate(data.begin(), data.end(), 0.0) / data.size();
    double variance = 0, kurtosis = 0;
    
    for (double x : data) {
        double diff = x - mean;
        variance += diff * diff;
        kurtosis += diff * diff * diff * diff;
    }
    
    variance /= data.size();
    kurtosis /= data.size();
    
    double stddev = std::sqrt(variance);
    return (stddev > 1e-10) ? kurtosis / (variance * variance) - 3.0 : 0.0;
}

double BacktestEngine::calculatePercentile(std::vector<double> data, double percentile) const {
    if (data.empty()) return 0.0;
    
    std::sort(data.begin(), data.end());
    int index = static_cast<int>(data.size() * percentile);
    index = std::max(0, std::min(static_cast<int>(data.size() - 1), index));
    
    return data[index];
}

double BacktestEngine::calculateAutocorrelation(const std::vector<double>& data, int lag) const {
    if (data.size() <= static_cast<size_t>(lag)) return 0.0;
    
    std::vector<double> x(data.begin(), data.end() - lag);
    std::vector<double> y(data.begin() + lag, data.end());
    
    return calculateCorrelation(x, y);
}