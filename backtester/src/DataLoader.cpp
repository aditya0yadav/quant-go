#include "DataLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

void DataLoader::loadMapping(const std::string &configFile)
{
    std::ifstream f(configFile);
    if (!f.is_open())
    {
        std::cerr << "Error opening config file: " << configFile << std::endl;
        return;
    }

    json j;
    f >> j;

    for (auto &el : j.items())
    {
        std::string field = el.key();
        columnConfig[field]["name"] = el.value().value("name", "");
        columnConfig[field]["present"] = el.value().value("present", false);
        columnConfig[field]["default"] = el.value().value("default", 0.0);
    }

    f.close();
}

void DataLoader::loadCSV(const std::string &filename, const std::string& symbolName, const std::string& tf)
{
    // Store symbol and timeframe as member variables
    symbol = symbolName;
    timeframe = tf;

    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    std::string line;
    if (!std::getline(file, line))
    {
        std::cerr << "Error reading header line from file: " << filename << std::endl;
        return;
    }

    // Parse header to map columns
    std::stringstream headerStream(line);
    std::string headerItem;
    int index = 0;
    while (std::getline(headerStream, headerItem, ','))
    {
        for (auto &[key, val] : columnConfig)
        {
            if (val["name"] == headerItem)
            {
                columnIndex[key] = index;
            }
        }
        index++;
    }

    // Read each data line
    while (std::getline(file, line))
    {
        if (line.empty())
            continue;

        std::stringstream ss(line);
        std::vector<std::string> tokens;
        std::string token;
        while (std::getline(ss, token, ','))
        {
            tokens.push_back(token);
        }

        DataPoint dp;

        dp.open = (columnIndex.count("open") && columnIndex["open"] < tokens.size())
                      ? std::stod(tokens[columnIndex["open"]])
                      : columnConfig["open"]["default"].get<double>();

        dp.high = (columnIndex.count("high") && columnIndex["high"] < tokens.size())
                      ? std::stod(tokens[columnIndex["high"]])
                      : columnConfig["high"]["default"].get<double>();

        dp.low = (columnIndex.count("low") && columnIndex["low"] < tokens.size())
                     ? std::stod(tokens[columnIndex["low"]])
                     : columnConfig["low"]["default"].get<double>();

        dp.close = (columnIndex.count("close") && columnIndex["close"] < tokens.size())
                       ? std::stod(tokens[columnIndex["close"]])
                       : columnConfig["close"]["default"].get<double>();

        dp.volume = (columnIndex.count("volume") && columnIndex["volume"] < tokens.size())
                        ? std::stod(tokens[columnIndex["volume"]])
                        : columnConfig["volume"]["default"].get<double>();

        data.push_back(dp);
    }

    file.close();
}

const std::vector<DataPoint> &DataLoader::getData() const
{
    return data;
}
