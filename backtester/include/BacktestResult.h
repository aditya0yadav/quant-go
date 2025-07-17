#pragma once
#include <vector>
#include <string>

struct TradeResult {
    std::string timestamp;
    double entryPrice;
    double exitPrice;
    double pnl;
    double size;
    std::string side; // "LONG" or "SHORT"
    double commission;
    double slippage;
    int holdingPeriod;
    double maxFavorableExcursion; // MFE
    double maxAdverseExcursion;   // MAE
};

struct PnLScore {
    double totalPnL;
    double avgPnL;
    double pnlStdDev;
    double pnlSkewness;
    double pnlKurtosis;
    double profitFactor;
    double recoveryFactor;
    double calmarRatio;
    double sortinoRatio;
    double maxConsecutiveWins;
    double maxConsecutiveLosses;
    double avgWinSize;
    double avgLossSize;
    double winLossRatio;
    double hitRatio;
    double payoffRatio;
    double largestWin;
    double largestLoss;
    double avgTradeDuration;
    double profitabilityIndex;
    double expectancy;
    double opportunityRatio;
};

struct CorrelationAnalysis {
    double signalPriceCorrelation;
    double signalReturnCorrelation;
    double signalVolumeCorrelation;
    double autocorrelationLag1;
    double autocorrelationLag5;
    double betaToMarket;
    double informationRatio;
    double treynorRatio;
    std::vector<double> rollingCorrelation;
    double avgRollingCorrelation;
    double correlationStability;
    double signalStrength;
    double predictivePower;
    double regimeStability;
    double signalDecay;
};

struct RiskMetrics {
    double valueAtRisk95;
    double valueAtRisk99;
    double expectedShortfall;
    double maxDrawdown;
    double avgDrawdown;
    double drawdownDuration;
    double ulcerIndex;
    double gainToPainRatio;
    double sterlingRatio;
    double burkeRatio;
    double downstdDev;
    double upsidePotential;
    double omegaRatio;
    double tailRatio;
    double skewnessRisk;
    double kurtosisRisk;
    double conditionalDrawdown;
};

struct BacktestResult {
    double sharpe;
    double winRate;
    double totalReturn;
    double maxDrawdown;
    double volatility;
    std::vector<TradeResult> tradeResults;

    PnLScore pnlScore;
    CorrelationAnalysis correlationAnalysis;
    RiskMetrics riskMetrics;
};
