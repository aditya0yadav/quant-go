#include "DataLoader.h"
#include "BacktestEngine.h"
#include "Expression.h"
#include "Parser.h"
#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace Color {
    // Basic colors
    const std::string RESET     = "\033[0m";
    const std::string BLACK     = "\033[30m";
    const std::string RED       = "\033[31m";
    const std::string GREEN     = "\033[32m";
    const std::string YELLOW    = "\033[33m";
    const std::string BLUE      = "\033[34m";
    const std::string MAGENTA   = "\033[35m";
    const std::string CYAN      = "\033[36m";
    const std::string WHITE     = "\033[37m";
    
    // Bright colors
    const std::string BRIGHT_BLACK   = "\033[90m";
    const std::string BRIGHT_RED     = "\033[91m";
    const std::string BRIGHT_GREEN   = "\033[92m";
    const std::string BRIGHT_YELLOW  = "\033[93m";
    const std::string BRIGHT_BLUE    = "\033[94m";
    const std::string BRIGHT_MAGENTA = "\033[95m";
    const std::string BRIGHT_CYAN    = "\033[96m";
    const std::string BRIGHT_WHITE   = "\033[97m";
    
    // Background colors
    const std::string BG_BLACK   = "\033[40m";
    const std::string BG_RED     = "\033[41m";
    const std::string BG_GREEN   = "\033[42m";
    const std::string BG_YELLOW  = "\033[43m";
    const std::string BG_BLUE    = "\033[44m";
    const std::string BG_MAGENTA = "\033[45m";
    const std::string BG_CYAN    = "\033[46m";
    const std::string BG_WHITE   = "\033[47m";
    
    // Text styles
    const std::string BOLD      = "\033[1m";
    const std::string DIM       = "\033[2m";
    const std::string ITALIC    = "\033[3m";
    const std::string UNDERLINE = "\033[4m";
    const std::string BLINK     = "\033[5m";
    const std::string REVERSE   = "\033[7m";
    const std::string STRIKETHROUGH = "\033[9m";
    
    // Custom themed colors for better UX
    const std::string HEADER    = BOLD + BRIGHT_CYAN;
    const std::string SUCCESS   = BOLD + BRIGHT_GREEN;
    const std::string ERROR     = BOLD + BRIGHT_RED;
    const std::string WARNING   = BOLD + BRIGHT_YELLOW;
    const std::string INFO      = BRIGHT_BLUE;
    const std::string PROMPT    = BOLD + BRIGHT_MAGENTA;
    const std::string SUBTLE    = BRIGHT_BLACK;
    const std::string ACCENT    = BRIGHT_CYAN;
    const std::string HIGHLIGHT = BOLD + YELLOW + BG_BLACK;
}

void printSeparator(const std::string& title = "", char sepChar = '=') {
    const int width = 60;
    if (title.empty()) {
        std::cout << Color::SUBTLE << std::string(width, sepChar) << Color::RESET << "\n";
    } else {
        int padding = (width - title.length() - 2) / 2;
        std::cout << Color::SUBTLE << std::string(padding, sepChar) 
                  << Color::RESET << " " << Color::HEADER << title << Color::RESET 
                  << " " << Color::SUBTLE << std::string(padding, sepChar) << Color::RESET << "\n";
    }
}

void printBanner() {
    std::cout << "\n";
    printSeparator();
    std::cout << Color::HEADER << "          🚀 LIVE BACKTESTING SYSTEM 🚀          " << Color::RESET << "\n";
    printSeparator();
    std::cout << Color::INFO << "          Advanced Trading Strategy Analyzer          " << Color::RESET << "\n";
    printSeparator("", '-');
    std::cout << "\n";
}

void printHelp() {
    std::cout << Color::ACCENT << "📊 Available Data Fields:" << Color::RESET << "\n";
    std::cout << Color::SUBTLE << "   • " << Color::RESET << Color::BRIGHT_WHITE << "open" << Color::RESET << Color::SUBTLE << ", " << Color::RESET;
    std::cout << Color::BRIGHT_WHITE << "close" << Color::RESET << Color::SUBTLE << ", " << Color::RESET;
    std::cout << Color::BRIGHT_WHITE << "high" << Color::RESET << Color::SUBTLE << ", " << Color::RESET;
    std::cout << Color::BRIGHT_WHITE << "low" << Color::RESET << Color::SUBTLE << ", " << Color::RESET;
    std::cout << Color::BRIGHT_WHITE << "volume" << Color::RESET << "\n\n";
    
    std::cout << Color::ACCENT << "🔧 Available Functions:" << Color::RESET << "\n";
    std::cout << Color::SUBTLE << "   • " << Color::RESET << Color::BRIGHT_YELLOW << "SMA(expr, period)" << Color::RESET << Color::SUBTLE << " - Simple Moving Average" << Color::RESET << "\n";
    std::cout << Color::SUBTLE << "   • " << Color::RESET << Color::BRIGHT_YELLOW << "EMA(expr, period)" << Color::RESET << Color::SUBTLE << " - Exponential Moving Average" << Color::RESET << "\n";
    std::cout << Color::SUBTLE << "   • " << Color::RESET << Color::BRIGHT_YELLOW << "log(expr)" << Color::RESET << Color::SUBTLE << ", " << Color::RESET;
    std::cout << Color::BRIGHT_YELLOW << "exp(expr)" << Color::RESET << Color::SUBTLE << ", " << Color::RESET;
    std::cout << Color::BRIGHT_YELLOW << "abs(expr)" << Color::RESET << Color::SUBTLE << ", " << Color::RESET;
    std::cout << Color::BRIGHT_YELLOW << "sqrt(expr)" << Color::RESET << "\n\n";
    
    std::cout << Color::ACCENT << "⚡ Available Operators:" << Color::RESET << "\n";
    std::cout << Color::SUBTLE << "   • Arithmetic: " << Color::RESET << Color::BRIGHT_GREEN << "+ - * /" << Color::RESET << "\n";
    std::cout << Color::SUBTLE << "   • Comparison: " << Color::RESET << Color::BRIGHT_GREEN << "> < >= <= == !=" << Color::RESET << "\n";
    std::cout << Color::SUBTLE << "   • Logical: " << Color::RESET << Color::BRIGHT_GREEN << "&& ||" << Color::RESET << "\n\n";
    
    std::cout << Color::ACCENT << "💡 Strategy Examples:" << Color::RESET << "\n";
    std::cout << Color::SUBTLE << "   1. " << Color::RESET << Color::BRIGHT_WHITE << "close > open" << Color::RESET << Color::SUBTLE << " (Bullish candle)" << Color::RESET << "\n";
    std::cout << Color::SUBTLE << "   2. " << Color::RESET << Color::BRIGHT_WHITE << "SMA(close, 20) > SMA(close, 50)" << Color::RESET << Color::SUBTLE << " (Golden cross)" << Color::RESET << "\n";
    std::cout << Color::SUBTLE << "   3. " << Color::RESET << Color::BRIGHT_WHITE << "close > SMA(close, 20) && volume > 1000" << Color::RESET << Color::SUBTLE << " (Breakout with volume)" << Color::RESET << "\n";
    std::cout << Color::SUBTLE << "   4. " << Color::RESET << Color::BRIGHT_WHITE << "(close - open) / open > 0.02" << Color::RESET << Color::SUBTLE << " (2% price increase)" << Color::RESET << "\n";
    std::cout << "\n";
}

void printMenu() {
    printSeparator("MENU", '-');
    std::cout << Color::PROMPT << "📈 [1]" << Color::RESET << " Run Backtest\n";
    std::cout << Color::PROMPT << "💾 [2]" << Color::RESET << " Export Last Result (JSON)\n";
    std::cout << Color::PROMPT << "❓ [3]" << Color::RESET << " Show Help\n";
    std::cout << Color::PROMPT << "🚪 [4]" << Color::RESET << " Exit\n";
    printSeparator("", '-');
}

void printResults(const BacktestResult& result) {
    std::cout << "\n";
    printSeparator("BACKTEST RESULTS");
    
    // Format numbers with proper precision
    std::cout << std::fixed << std::setprecision(4);
    
    // Sharpe Ratio with color coding
    std::cout << Color::INFO << "📊 Sharpe Ratio: " << Color::RESET;
    if (result.sharpe > 1.0) {
        std::cout << Color::SUCCESS << result.sharpe << Color::RESET;
    } else if (result.sharpe > 0.5) {
        std::cout << Color::WARNING << result.sharpe << Color::RESET;
    } else {
        std::cout << Color::ERROR << result.sharpe << Color::RESET;
    }
    std::cout << "\n";
    
    // Win Rate with color coding
    std::cout << Color::INFO << "🎯 Win Rate: " << Color::RESET;
    double winRatePercent = result.winRate * 100;
    if (winRatePercent > 60) {
        std::cout << Color::SUCCESS << winRatePercent << "%" << Color::RESET;
    } else if (winRatePercent > 45) {
        std::cout << Color::WARNING << winRatePercent << "%" << Color::RESET;
    } else {
        std::cout << Color::ERROR << winRatePercent << "%" << Color::RESET;
    }
    std::cout << "\n";
    
    // Total Return with color coding
    std::cout << Color::INFO << "💰 Total Return: " << Color::RESET;
    if (result.totalReturn > 10000) {
        std::cout << Color::SUCCESS << "$" << result.totalReturn << Color::RESET;
    } else if (result.totalReturn > 9000) {
        std::cout << Color::WARNING << "$" << result.totalReturn << Color::RESET;
    } else {
        std::cout << Color::ERROR << "$" << result.totalReturn << Color::RESET;
    }
    std::cout << "\n";
    
    // Return percentage with color coding
    double returnPercent = (result.totalReturn / 10000.0 - 1.0) * 100;
    std::cout << Color::INFO << "📈 Return %: " << Color::RESET;
    if (returnPercent > 0) {
        std::cout << Color::SUCCESS << "+" << returnPercent << "%" << Color::RESET;
    } else {
        std::cout << Color::ERROR << returnPercent << "%" << Color::RESET;
    }
    std::cout << "\n";
    
    printSeparator();
    std::cout << "\n";
}

void exportResultsJSON(const BacktestResult& result, const std::string& filename) {
    json j;
    j["sharpe"] = result.sharpe;
    j["winRate"] = result.winRate;
    j["totalReturn"] = result.totalReturn;
    j["returnPercent"] = (result.totalReturn / 10000.0 - 1.0) * 100;
    
    std::ofstream out(filename);
    out << j.dump(4);
    out.close();
    
    std::cout << Color::SUCCESS << "✅ Results exported to " << Color::ACCENT << filename << Color::RESET << "\n";
}

int main() {
    DataLoader loader;

    // Load data with visual feedback
    std::cout << Color::INFO << "🔄 Loading configuration..." << Color::RESET << std::endl;
    loader.loadMapping("../config/column_config.json");
    
    std::cout << Color::INFO << "📊 Loading market data..." << Color::RESET << std::endl;
    loader.loadCSV("../data/BTCUSDT_big.csv", "BTCUSDT", "2m");
    const auto& data = loader.getData();
    
    std::cout << Color::SUCCESS << "✅ Data loaded successfully!" << Color::RESET << std::endl;

    printBanner();
    printHelp();

    BacktestResult lastResult;
    bool hasResult = false;

    while (true) {
        printMenu();
        std::cout << Color::PROMPT << "👉 Enter your choice: " << Color::RESET;

        std::string choice;
        std::getline(std::cin, choice);

        if (choice == "4" || choice == "exit" || choice == "quit") {
            std::cout << Color::SUCCESS << "\n👋 Thank you for using the Enhanced Backtesting System!\n" << Color::RESET;
            break;
        }
        else if (choice == "1") {
            std::cout << "\n" << Color::PROMPT << "🎯 Enter your strategy expression: " << Color::RESET;
            std::string input;
            std::getline(std::cin, input);

            if (input.empty()) {
                std::cout << Color::WARNING << "⚠️  Empty input. Please enter a valid strategy expression.\n" << Color::RESET;
                continue;
            }

            std::cout << Color::INFO << "🔄 Running backtest..." << Color::RESET << std::endl;

            try {
                auto strategy = Parser::parseExpression(input);
                BacktestEngine engine;
                lastResult = engine.run(data, *strategy, 10000);
                hasResult = true;

                printResults(lastResult);
                
                std::cout << Color::SUCCESS << "✅ Backtest completed successfully!" << Color::RESET << "\n";
            }
            catch (const std::exception& ex) {
                std::cout << "\n";
                printSeparator("ERROR", '!');
                std::cout << Color::ERROR << "❌ " << ex.what() << Color::RESET << "\n";
                printSeparator("", '!');
                std::cout << Color::SUBTLE << "💡 Tip: Check your syntax and try again\n" << Color::RESET;
            }
        }
        else if (choice == "2") {
            if (!hasResult) {
                std::cout << Color::WARNING << "⚠️  No backtest results to export. Please run a backtest first.\n" << Color::RESET;
                continue;
            }
            exportResultsJSON(lastResult, "last_backtest_result.json");
        }
        else if (choice == "3") {
            printHelp();
        }
        else {
            std::cout << Color::ERROR << "❌ Invalid choice. Please select 1-4.\n" << Color::RESET;
        }
    }

    return 0;
}