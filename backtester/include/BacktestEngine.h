#pragma once
#include "DataPoint.h"
#include "BacktestResult.h"
#include "Expression.h"
#include <vector>
#include <memory>
#include "BacktestResult.h"


class BacktestEngine {
public:
    BacktestResult run(const std::vector<DataPoint>& data, const Expression& strategy, double capital);
    
    // Enhanced analysis methods
    PnLScore calculatePnLScore(const std::vector<TradeResult>& trades, double initialCapital) const;
    CorrelationAnalysis calculateCorrelationAnalysis(const std::vector<DataPoint>& data, 
                                                   const Expression& strategy) const;
    RiskMetrics calculateRiskMetrics(const std::vector<double>& returns, 
                                   const std::vector<double>& equity) const;
    
    // Configuration methods
    void setRiskParameters(double maxPositionSize, double maxDrawdownLimit, bool useRiskManagement);
    void setTransactionCosts(double commission, double slippage);
    void setSignalSmoothing(int windowSize, bool useSmoothing);
    
    // Portfolio analysis
    std::vector<double> calculateRollingMetrics(const std::vector<double>& returns, 
                                              int windowSize) const;
    double calculateInformationRatio(const std::vector<double>& returns, 
                                   const std::vector<double>& benchmark) const;
    
private:
    // Position sizing methods
    double calculatePosition(double signal);
    double calculatePositionSmart(double signal, bool isBooleanSignal, double maxSignal, double minSignal);
    double calculateKellyPosition(double signal, double winRate, double avgWin, double avgLoss);
    double applyRiskManagement(double position, double currentCapital, double peakCapital);
    
    // Signal processing
    double smoothSignal(const std::vector<double>& signals, size_t index, int windowSize) const;
    void detectSignalRegime(const std::vector<double>& signals, std::vector<int>& regimes) const;
    
    // Statistical helpers
    double calculateCorrelation(const std::vector<double>& x, const std::vector<double>& y) const;
    double calculateSkewness(const std::vector<double>& data) const;
    double calculateKurtosis(const std::vector<double>& data) const;
    double calculatePercentile(std::vector<double> data, double percentile) const;
    double calculateAutocorrelation(const std::vector<double>& data, int lag) const;
    
    // Risk management parameters
    double m_maxPositionSize = 0.1;
    double m_maxDrawdownLimit = 0.2;
    bool m_useRiskManagement = true;
    double m_commission = 0.001;
    double m_slippage = 0.0005;
    bool m_useSmoothing = false;
    int m_smoothingWindow = 5;
};