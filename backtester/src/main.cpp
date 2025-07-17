#include "DataLoader.h"
#include "BacktestEngine.h"
#include "Expression.h"
#include "Parser.h"
#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <filesystem>
#include <algorithm>
#include <cmath>
#include "nlohmann/json.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace Color {
    const std::string RESET = "\033[0m";
    const std::string BRIGHT_CYAN = "\033[96m";
    const std::string BRIGHT_GREEN = "\033[92m";
    const std::string BRIGHT_YELLOW = "\033[93m";
    const std::string BRIGHT_RED = "\033[91m";
    const std::string BRIGHT_WHITE = "\033[97m";
    const std::string BRIGHT_BLUE = "\033[94m";
    const std::string BRIGHT_MAGENTA = "\033[95m";
    const std::string PROMPT = "\033[1m\033[95m";
    const std::string INFO = "\033[94m";
    const std::string SUCCESS = "\033[1m\033[92m";
    const std::string ERROR = "\033[1m\033[91m";
    const std::string WARNING = "\033[1m\033[93m";
    const std::string ACCENT = "\033[96m";
    const std::string SUBTLE = "\033[90m";
    const std::string BOLD = "\033[1m";
    const std::string DIM = "\033[2m";
}

void printSeparator(const std::string& title = "", char sepChar = '=', int width = 80) {
    if (title.empty()) {
        std::cout << Color::SUBTLE << std::string(width, sepChar) << Color::RESET << "\n";
    } else {
        int padding = (width - title.length() - 2) / 2;
        std::cout << Color::SUBTLE << std::string(padding, sepChar)
                  << Color::RESET << " " << Color::BRIGHT_CYAN << Color::BOLD << title << Color::RESET
                  << " " << Color::SUBTLE << std::string(padding, sepChar) << Color::RESET << "\n";
    }
}

void printBox(const std::string& content, const std::string& color = Color::INFO, int width = 80) {
    std::cout << Color::SUBTLE << "╭" << std::string(width - 2, '─') << "╮" << Color::RESET << "\n";
    std::cout << Color::SUBTLE << "│" << color << std::setw(width - 2) << std::left << content 
              << Color::SUBTLE << "│" << Color::RESET << "\n";
    std::cout << Color::SUBTLE << "╰" << std::string(width - 2, '─') << "╯" << Color::RESET << "\n";
}

void printMetricRow(const std::string& label, double value, const std::string& unit = "", 
                   bool isPercentage = false, int precision = 4) {
    std::cout << Color::ACCENT << "│ " << std::setw(32) << std::left << label << Color::RESET;
    
    std::string color = Color::INFO;
    if (isPercentage) {
        if (value > 0) color = Color::BRIGHT_GREEN;
        else if (value < 0) color = Color::BRIGHT_RED;
        std::cout << color << std::setw(12) << std::right << std::fixed << std::setprecision(2) 
                  << value << "%" << Color::RESET;
    } else {
        if (value > 0) color = Color::BRIGHT_GREEN;
        else if (value < 0) color = Color::BRIGHT_RED;
        std::cout << color << std::setw(12) << std::right << std::fixed << std::setprecision(precision) 
                  << value << unit << Color::RESET;
    }
    std::cout << Color::SUBTLE << " │" << Color::RESET << "\n";
}

void printBanner() {
    std::cout << "\n";
    printSeparator("", '═', 80);
    std::cout << Color::BRIGHT_CYAN << Color::BOLD << "          🚀 ADVANCED BACKTESTING SYSTEM 🚀          " << Color::RESET << "\n";
    printSeparator("", '═', 80);
    std::cout << Color::INFO << "   Professional Trading Strategy Analysis & Optimization   " << Color::RESET << "\n";
    printSeparator("", '─', 80);
    std::cout << "\n";
}

void printOverallPerformance(const BacktestResult& result) {
    printSeparator("📊 OVERALL PERFORMANCE", '─', 80);
    
    std::cout << Color::SUBTLE << "╭" << std::string(48, '─') << "╮" << Color::RESET << "\n";
    printMetricRow("Sharpe Ratio", result.sharpe);
    printMetricRow("Win Rate", result.winRate * 100, "", true);
    printMetricRow("Total Return", result.totalReturn, " $");
    printMetricRow("Return Percentage", (result.totalReturn / 10000.0 - 1.0) * 100, "", true);
    printMetricRow("Maximum Drawdown", result.maxDrawdown * 100, "", true);
    printMetricRow("Volatility", result.volatility * 100, "", true);
    printMetricRow("Total Trades", static_cast<double>(result.tradeResults.size()), "", false, 0);
    std::cout << Color::SUBTLE << "╰" << std::string(48, '─') << "╯" << Color::RESET << "\n\n";
}

void printPnLAnalysis(const PnLScore& pnl) {
    printSeparator("💰 P&L ANALYSIS", '─', 80);
    
    std::cout << Color::SUBTLE << "╭" << std::string(48, '─') << "╮" << Color::RESET << "\n";
    printMetricRow("Total P&L", pnl.totalPnL, " $");
    printMetricRow("Average P&L", pnl.avgPnL, " $");
    printMetricRow("P&L Standard Deviation", pnl.pnlStdDev, " $");
    printMetricRow("P&L Skewness", pnl.pnlSkewness);
    printMetricRow("P&L Kurtosis", pnl.pnlKurtosis);
    printMetricRow("Profit Factor", pnl.profitFactor);
    printMetricRow("Recovery Factor", pnl.recoveryFactor);
    printMetricRow("Calmar Ratio", pnl.calmarRatio);
    printMetricRow("Sortino Ratio", pnl.sortinoRatio);
    std::cout << Color::SUBTLE << "╰" << std::string(48, '─') << "╯" << Color::RESET << "\n\n";
}

void printTradeAnalysis(const PnLScore& pnl) {
    printSeparator("🎯 TRADE ANALYSIS", '─', 80);
    
    std::cout << Color::SUBTLE << "╭" << std::string(48, '─') << "╮" << Color::RESET << "\n";
    printMetricRow("Hit Ratio", pnl.hitRatio * 100, "", true);
    printMetricRow("Average Win Size", pnl.avgWinSize, " $");
    printMetricRow("Average Loss Size", pnl.avgLossSize, " $");
    printMetricRow("Win/Loss Ratio", pnl.winLossRatio);
    printMetricRow("Payoff Ratio", pnl.payoffRatio);
    printMetricRow("Largest Win", pnl.largestWin, " $");
    printMetricRow("Largest Loss", pnl.largestLoss, " $");
    printMetricRow("Max Consecutive Wins", pnl.maxConsecutiveWins, "", false, 0);
    printMetricRow("Max Consecutive Losses", pnl.maxConsecutiveLosses, "", false, 0);
    printMetricRow("Average Trade Duration", pnl.avgTradeDuration, " periods", false, 1);
    std::cout << Color::SUBTLE << "╰" << std::string(48, '─') << "╯" << Color::RESET << "\n\n";
}

void printRiskMetrics(const RiskMetrics& risk) {
    printSeparator("⚠️ RISK METRICS", '─', 80);
    
    std::cout << Color::SUBTLE << "╭" << std::string(48, '─') << "╮" << Color::RESET << "\n";
    printMetricRow("Value at Risk (95%)", risk.valueAtRisk95 * 100, "", true);
    printMetricRow("Value at Risk (99%)", risk.valueAtRisk99 * 100, "", true);
    printMetricRow("Expected Shortfall", risk.expectedShortfall * 100, "", true);
    printMetricRow("Maximum Drawdown", risk.maxDrawdown * 100, "", true);
    printMetricRow("Average Drawdown", risk.avgDrawdown * 100, "", true);
    printMetricRow("Drawdown Duration", risk.drawdownDuration, " periods");
    printMetricRow("Ulcer Index", risk.ulcerIndex * 100, "", true);
    printMetricRow("Gain to Pain Ratio", risk.gainToPainRatio);
    printMetricRow("Sterling Ratio", risk.sterlingRatio);
    printMetricRow("Burke Ratio", risk.burkeRatio);
    printMetricRow("Omega Ratio", risk.omegaRatio);
    printMetricRow("Tail Ratio", risk.tailRatio);
    std::cout << Color::SUBTLE << "╰" << std::string(48, '─') << "╯" << Color::RESET << "\n\n";
}

void printCorrelationAnalysis(const CorrelationAnalysis& corr) {
    printSeparator("🔗 CORRELATION ANALYSIS", '─', 80);
    
    std::cout << Color::SUBTLE << "╭" << std::string(48, '─') << "╮" << Color::RESET << "\n";
    printMetricRow("Signal-Price Correlation", corr.signalPriceCorrelation);
    printMetricRow("Signal-Return Correlation", corr.signalReturnCorrelation);
    printMetricRow("Signal-Volume Correlation", corr.signalVolumeCorrelation);
    printMetricRow("Autocorrelation Lag 1", corr.autocorrelationLag1);
    printMetricRow("Autocorrelation Lag 5", corr.autocorrelationLag5);
    printMetricRow("Beta to Market", corr.betaToMarket);
    printMetricRow("Information Ratio", corr.informationRatio);
    printMetricRow("Treynor Ratio", corr.treynorRatio);
    printMetricRow("Average Rolling Correlation", corr.avgRollingCorrelation);
    printMetricRow("Correlation Stability", corr.correlationStability);
    printMetricRow("Signal Strength", corr.signalStrength);
    printMetricRow("Predictive Power", corr.predictivePower);
    std::cout << Color::SUBTLE << "╰" << std::string(48, '─') << "╯" << Color::RESET << "\n\n";
}

void printTradeHistory(const std::vector<TradeResult>& trades, int maxTrades = 10) {
    if (trades.empty()) return;
    
    printSeparator("📋 RECENT TRADES", '─', 80);
    
    std::cout << Color::SUBTLE << "╭" << std::string(78, '─') << "╮" << Color::RESET << "\n";
    std::cout << Color::ACCENT << "│" << Color::BOLD << std::setw(12) << "Timestamp" 
              << std::setw(12) << "Entry" << std::setw(12) << "Exit" 
              << std::setw(12) << "P&L" << std::setw(12) << "Side" 
              << std::setw(12) << "Duration" << Color::RESET << Color::SUBTLE << " │" << Color::RESET << "\n";
    std::cout << Color::SUBTLE << "├" << std::string(78, '─') << "┤" << Color::RESET << "\n";
    
    int displayCount = std::min(maxTrades, static_cast<int>(trades.size()));
    for (int i = 0; i < displayCount; ++i) {
        const auto& trade = trades[i];
        std::string color = trade.pnl > 0 ? Color::BRIGHT_GREEN : Color::BRIGHT_RED;
        
        std::cout << Color::SUBTLE << "│" << Color::RESET;
        std::cout << std::setw(12) << trade.timestamp.substr(0, 10);
        std::cout << std::setw(12) << std::fixed << std::setprecision(2) << trade.entryPrice;
        std::cout << std::setw(12) << std::fixed << std::setprecision(2) << trade.exitPrice;
        std::cout << color << std::setw(12) << std::fixed << std::setprecision(2) << trade.pnl << Color::RESET;
        std::cout << std::setw(12) << trade.side;
        std::cout << std::setw(12) << trade.holdingPeriod;
        std::cout << Color::SUBTLE << " │" << Color::RESET << "\n";
    }
    
    std::cout << Color::SUBTLE << "╰" << std::string(78, '─') << "╯" << Color::RESET << "\n\n";
    
    if (trades.size() > maxTrades) {
        std::cout << Color::DIM << "... and " << (trades.size() - maxTrades) << " more trades" << Color::RESET << "\n\n";
    }
}

void printSummaryCard(const BacktestResult& result) {
    printSeparator("📈 STRATEGY SUMMARY", '─', 80);
    
    double totalReturn = (result.totalReturn / 10000.0 - 1.0) * 100;
    std::string performance = totalReturn > 0 ? "PROFITABLE" : "LOSING";
    std::string perfColor = totalReturn > 0 ? Color::BRIGHT_GREEN : Color::BRIGHT_RED;
    
    std::cout << Color::SUBTLE << "╭" << std::string(78, '─') << "╮" << Color::RESET << "\n";
    std::cout << Color::SUBTLE << "│" << Color::BOLD << Color::BRIGHT_CYAN << std::setw(78) << std::left 
              << "  🎯 STRATEGY PERFORMANCE: " + performance << Color::RESET << Color::SUBTLE << "│" << Color::RESET << "\n";
    std::cout << Color::SUBTLE << "├" << std::string(78, '─') << "┤" << Color::RESET << "\n";
    std::cout << Color::SUBTLE << "│" << Color::RESET << " Total Return: " << perfColor << std::setw(20) << std::right 
              << std::fixed << std::setprecision(2) << totalReturn << "%" << Color::RESET << std::setw(30) << " "
              << Color::SUBTLE << "│" << Color::RESET << "\n";
    std::cout << Color::SUBTLE << "│" << Color::RESET << " Sharpe Ratio: " << Color::BRIGHT_CYAN << std::setw(20) << std::right 
              << std::fixed << std::setprecision(4) << result.sharpe << Color::RESET << std::setw(31) << " "
              << Color::SUBTLE << "│" << Color::RESET << "\n";
    std::cout << Color::SUBTLE << "│" << Color::RESET << " Max Drawdown: " << Color::BRIGHT_RED << std::setw(20) << std::right 
              << std::fixed << std::setprecision(2) << result.maxDrawdown * 100 << "%" << Color::RESET << std::setw(30) << " "
              << Color::SUBTLE << "│" << Color::RESET << "\n";
    std::cout << Color::SUBTLE << "│" << Color::RESET << " Total Trades: " << Color::BRIGHT_YELLOW << std::setw(20) << std::right 
              << result.tradeResults.size() << Color::RESET << std::setw(32) << " "
              << Color::SUBTLE << "│" << Color::RESET << "\n";
    std::cout << Color::SUBTLE << "╰" << std::string(78, '─') << "╯" << Color::RESET << "\n\n";
}

void printConfigFormatHelp() {
    std::cout << Color::ACCENT << "📝 Example Config File Format:" << Color::RESET << "\n";
    std::cout << Color::SUBTLE << R"(
{
    "timestamp": { "name": "timestamp", "present": true },
    "open": { "name": "open", "present": true },
    "high": { "name": "high", "present": true },
    "low": { "name": "low", "present": true },
    "close": { "name": "close", "present": true },
    "volume": { "name": "volume", "present": true }
}
    )" << Color::RESET << "\n";
}

void printMenu() {
    printSeparator("MAIN MENU", '─', 80);
    std::cout << Color::PROMPT << "📈 [1]" << Color::RESET << " Run Comprehensive Backtest\n";
    std::cout << Color::PROMPT << "💾 [2]" << Color::RESET << " Export Last Result (JSON)\n";
    std::cout << Color::PROMPT << "❓ [3]" << Color::RESET << " Show Help\n";
    std::cout << Color::PROMPT << "📝 [4]" << Color::RESET << " Show Config Format Example\n";
    std::cout << Color::PROMPT << "📂 [5]" << Color::RESET << " Select Data CSV File\n";
    std::cout << Color::PROMPT << "⚙️  [6]" << Color::RESET << " Select Config JSON File\n";
    std::cout << Color::PROMPT << "💼 [7]" << Color::RESET << " Load Multi-Symbol Portfolio\n";
    std::cout << Color::PROMPT << "🚪 [8]" << Color::RESET << " Exit\n";
    printSeparator("", '─', 80);
}

bool fileExists(const std::string& path) {
    return fs::exists(path) && fs::is_regular_file(path);
}

void saveUserPreferences(const std::string& dataPath, const std::string& configPath) {
    json prefs;
    prefs["dataPath"] = dataPath;
    prefs["configPath"] = configPath;
    std::ofstream out("user_prefs.json");
    out << prefs.dump(4);
    out.close();
}

void loadUserPreferences(std::string& dataPath, std::string& configPath) {
    if (!fileExists("user_prefs.json")) return;
    std::ifstream in("user_prefs.json");
    json prefs;
    in >> prefs;
    dataPath = prefs.value("dataPath", dataPath);
    configPath = prefs.value("configPath", configPath);
}

void printComprehensiveResults(const BacktestResult& result) {
    std::cout << "\n";
    printSeparator("🎉 COMPREHENSIVE BACKTEST RESULTS", '═', 80);
    
    // Summary card first
    printSummaryCard(result);
    
    // Overall performance
    printOverallPerformance(result);
    
    // P&L Analysis
    printPnLAnalysis(result.pnlScore);
    
    // Trade Analysis
    printTradeAnalysis(result.pnlScore);
    
    // Risk Metrics
    printRiskMetrics(result.riskMetrics);
    
    // Correlation Analysis
    printCorrelationAnalysis(result.correlationAnalysis);
    
    // Recent trades
    printTradeHistory(result.tradeResults, 10);
    
    printSeparator("END OF RESULTS", '═', 80);
    std::cout << "\n";
}

json exportComprehensiveResults(const BacktestResult& result) {
    json j;
    
    // Basic metrics
    j["basic"]["sharpe"] = result.sharpe;
    j["basic"]["winRate"] = result.winRate;
    j["basic"]["totalReturn"] = result.totalReturn;
    j["basic"]["returnPercent"] = (result.totalReturn / 10000.0 - 1.0) * 100;
    j["basic"]["maxDrawdown"] = result.maxDrawdown;
    j["basic"]["volatility"] = result.volatility;
    j["basic"]["totalTrades"] = result.tradeResults.size();
    
    // P&L Score
    j["pnlScore"] = {
        {"totalPnL", result.pnlScore.totalPnL},
        {"avgPnL", result.pnlScore.avgPnL},
        {"pnlStdDev", result.pnlScore.pnlStdDev},
        {"pnlSkewness", result.pnlScore.pnlSkewness},
        {"pnlKurtosis", result.pnlScore.pnlKurtosis},
        {"profitFactor", result.pnlScore.profitFactor},
        {"recoveryFactor", result.pnlScore.recoveryFactor},
        {"calmarRatio", result.pnlScore.calmarRatio},
        {"sortinoRatio", result.pnlScore.sortinoRatio},
        {"hitRatio", result.pnlScore.hitRatio},
        {"avgWinSize", result.pnlScore.avgWinSize}
    };
    
    // Risk Metrics
    j["riskMetrics"] = {
        {"valueAtRisk95", result.riskMetrics.valueAtRisk95},
        {"valueAtRisk99", result.riskMetrics.valueAtRisk99},
        {"expectedShortfall", result.riskMetrics.expectedShortfall},
        {"maxDrawdown", result.riskMetrics.maxDrawdown},
        {"avgDrawdown", result.riskMetrics.avgDrawdown},
        {"drawdownDuration", result.riskMetrics.drawdownDuration},
        {"ulcerIndex", result.riskMetrics.ulcerIndex},
        {"gainToPainRatio", result.riskMetrics.gainToPainRatio},
        {"sterlingRatio", result.riskMetrics.sterlingRatio},
        {"burkeRatio", result.riskMetrics.burkeRatio},
        {"omegaRatio", result.riskMetrics.omegaRatio},
        {"tailRatio", result.riskMetrics.tailRatio}
    };
    
    // Correlation Analysis
    j["correlationAnalysis"] = {
        {"signalPriceCorrelation", result.correlationAnalysis.signalPriceCorrelation},
        {"signalReturnCorrelation", result.correlationAnalysis.signalReturnCorrelation},
        {"signalVolumeCorrelation", result.correlationAnalysis.signalVolumeCorrelation},
        {"autocorrelationLag1", result.correlationAnalysis.autocorrelationLag1},
        {"autocorrelationLag5", result.correlationAnalysis.autocorrelationLag5},
        {"betaToMarket", result.correlationAnalysis.betaToMarket},
        {"informationRatio", result.correlationAnalysis.informationRatio},
        {"treynorRatio", result.correlationAnalysis.treynorRatio},
        {"avgRollingCorrelation", result.correlationAnalysis.avgRollingCorrelation},
        {"correlationStability", result.correlationAnalysis.correlationStability},
        {"signalStrength", result.correlationAnalysis.signalStrength},
        {"predictivePower", result.correlationAnalysis.predictivePower}
    };
    
    // Trade results (last 50 trades)
    j["recentTrades"] = json::array();
    int maxTrades = std::min(50, static_cast<int>(result.tradeResults.size()));
    for (int i = 0; i < maxTrades; ++i) {
        const auto& trade = result.tradeResults[i];
        j["recentTrades"].push_back({
            {"timestamp", trade.timestamp},
            {"entryPrice", trade.entryPrice},
            {"exitPrice", trade.exitPrice},
            {"pnl", trade.pnl},
            {"side", trade.side},
            {"holdingPeriod", trade.holdingPeriod}
        });
    }
    
    return j;
}

int main(int argc, char* argv[]) {
    std::string dataPath = "../data/BTCUSDT_big.csv";
    std::string configPath = "../config/column_config.json";

    loadUserPreferences(dataPath, configPath);

    if (argc > 1) dataPath = argv[1];
    if (argc > 2) configPath = argv[2];

    DataLoader loader;
    BacktestResult lastResult;
    bool hasResult = false;

    printBanner();
    printConfigFormatHelp();

    while (true) {
        printMenu();
        std::cout << Color::PROMPT << "👉 Enter your choice: " << Color::RESET;
        std::string choice;
        std::getline(std::cin, choice);

        if (choice == "8" || choice == "exit" || choice == "quit") {
            saveUserPreferences(dataPath, configPath);
            std::cout << Color::SUCCESS << "\n👋 Preferences saved. Goodbye!\n" << Color::RESET;
            break;
        }
        else if (choice == "1") {
            if (!fileExists(dataPath) || !fileExists(configPath)) {
                std::cout << Color::ERROR << "❌ Invalid data or config path.\n" << Color::RESET;
                continue;
            }

            try {
                loader.loadMapping(configPath);
                loader.loadCSV(dataPath, "USER_DATA", "1m");
            } catch (const std::exception& ex) {
                std::cout << Color::ERROR << "❌ Load error: " << ex.what() << Color::RESET << "\n";
                continue;
            }

            std::cout << Color::PROMPT << "🎯 Enter your strategy expression: " << Color::RESET;
            std::string input;
            std::getline(std::cin, input);

            if (input.empty()) {
                std::cout << Color::WARNING << "⚠️  Empty input.\n" << Color::RESET;
                continue;
            }

            std::cout << Color::INFO << "🔄 Running comprehensive backtest analysis..." << Color::RESET << std::endl;

            try {
                auto strategy = Parser::parseExpression(input);
                BacktestEngine engine;
                
                // Configure engine for comprehensive analysis
                engine.setRiskParameters(1.0, 0.20, true);
                engine.setTransactionCosts(0.001, 0.0005);
                engine.setSignalSmoothing(3, true);
                
                lastResult = engine.run(loader.getData(), *strategy, 10000);
                hasResult = true;

                // Display comprehensive results
                printComprehensiveResults(lastResult);

            } catch (const std::exception& ex) {
                std::cout << Color::ERROR << "❌ Backtest error: " << ex.what() << Color::RESET << "\n";
            }
        }
        else if (choice == "2") {
            if (!hasResult) {
                std::cout << Color::WARNING << "⚠️  No results to export.\n" << Color::RESET;
                continue;
            }
            
            json exportData = exportComprehensiveResults(lastResult);
            std::ofstream out("comprehensive_backtest_result.json");
            out << exportData.dump(4);
            out.close();
            
            std::cout << Color::SUCCESS << "✅ Comprehensive results exported to 'comprehensive_backtest_result.json'\n" << Color::RESET;
        }
        else if (choice == "3") {
            printConfigFormatHelp();
        }
        else if (choice == "4") {
            printConfigFormatHelp();
        }
        else if (choice == "5") {
            std::cout << Color::PROMPT << "📂 Enter data CSV path: " << Color::RESET;
            std::string path;
            std::getline(std::cin, path);
            if (fileExists(path)) {
                dataPath = path;
                std::cout << Color::SUCCESS << "✅ Data path updated.\n" << Color::RESET;
            } else {
                std::cout << Color::ERROR << "❌ File does not exist.\n" << Color::RESET;
            }
        }
        else if (choice == "6") {
            std::cout << Color::PROMPT << "⚙️  Enter config JSON path: " << Color::RESET;
            std::string path;
            std::getline(std::cin, path);
            if (fileExists(path)) {
                configPath = path;
                std::cout << Color::SUCCESS << "✅ Config path updated.\n" << Color::RESET;
            } else {
                std::cout << Color::ERROR << "❌ File does not exist.\n" << Color::RESET;
            }
        }
        else if (choice == "7") {
            std::cout << Color::PROMPT << "💼 Enter directory containing multiple CSV files: " << Color::RESET;
            std::string dirPath;
            std::getline(std::cin, dirPath);
                        if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
                std::cout << Color::ERROR << "❌ Invalid directory.\n" << Color::RESET;
                continue;
            }

            std::vector<std::string> files;
            for (const auto& entry : fs::directory_iterator(dirPath)) {
                if (entry.is_regular_file() && entry.path().extension() == ".csv") {
                    files.push_back(entry.path().string());
                }
            }

            if (files.empty()) {
                std::cout << Color::WARNING << "⚠️  No CSV files found in the directory.\n" << Color::RESET;
                continue;
            }

            std::cout << Color::INFO << "📂 Found " << files.size() << " CSV files.\n" << Color::RESET;
            DataLoader multiLoader;
            for (const auto& file : files) {
                std::string symbol = fs::path(file).stem().string();
                try {
                    multiLoader.loadMapping(configPath);
                    multiLoader.loadCSV(file, symbol, "1m");
                    std::cout << Color::SUCCESS << "✅ Loaded " << symbol << "\n" << Color::RESET;
                } catch (const std::exception& ex) {
                    std::cout << Color::ERROR << "❌ Failed to load " << symbol << ": " << ex.what() << "\n" << Color::RESET;
                }
            }

            std::cout << Color::SUCCESS << "🎉 All symbols loaded into portfolio.\n" << Color::RESET;
        }
        else {
            std::cout << Color::WARNING << "⚠️  Invalid choice. Please try again.\n" << Color::RESET;
        }
    }

    return 0;
}
