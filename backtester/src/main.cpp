#include "DataLoader.h"
#include "BacktestEngine.h"
#include "Expression.h"
#include "Parser.h"
#include <iostream>
#include <string>

int main() {
    DataLoader loader;
    loader.loadMapping("../config/column_config.json");
    loader.loadCSV("../data/BTCUSDT.csv");

    const auto& data = loader.getData();

    std::cout << "Enhanced Backtesting System\n";
    std::cout << "===========================\n";
    std::cout << "Available fields: open, close, high, low, volume\n";
    std::cout << "Available functions: SMA(expr, period), EMA(expr, period), log(expr), exp(expr), abs(expr), sqrt(expr)\n";
    std::cout << "Available operators: +, -, *, /, >, <, >=, <=, ==, !=, &&, ||\n";
    std::cout << "Examples:\n";
    std::cout << "  - close > open\n";
    std::cout << "  - SMA(close, 20) > SMA(close, 50)\n";
    std::cout << "  - close > SMA(close, 20) && volume > 1000\n";
    std::cout << "  - (close - open) / open > 0.02\n";
    std::cout << "  - log(close) - log(open) > 0.01\n\n";

    while (true) {
        std::cout << "Enter strategy expression or 'exit': ";
        std::string input;
        std::getline(std::cin, input);

        if (input == "exit" || input.empty()) break;

        try {
            auto strategy = Parser::parseExpression(input);

            BacktestEngine engine;
            BacktestResult result = engine.run(data, *strategy, 10000);

            std::cout << "\nBacktest Results:\n";
            std::cout << "================\n";
            std::cout << "Sharpe Ratio: " << result.sharpe << "\n";
            std::cout << "Win Rate: " << (result.winRate * 100) << "%\n";
            std::cout << "Total Return: $" << result.totalReturn << "\n";
            std::cout << "Return %: " << (result.totalReturn / 10000.0 * 100) << "%\n\n";
        }
        catch (const std::exception& ex) {
            std::cerr << "Error: " << ex.what() << std::endl << std::endl;
        }
    }

    return 0;
}