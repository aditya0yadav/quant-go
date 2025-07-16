#pragma once

#include <string>
#include <vector>
#include <map>
#include "DataPoint.h"
#include "nlohmann/json.hpp"

class DataLoader {
private:
    std::vector<DataPoint> data;
    std::map<std::string, int> columnIndex; 
    std::map<std::string, nlohmann::json> columnConfig; 

    std::string symbol;
    std::string timeframe;

public:
    void loadMapping(const std::string& configFile);
    void loadCSV(const std::string& filename, const std::string& symbolName, const std::string& tf);
    
    const std::vector<DataPoint>& getData() const;

    const std::string& getSymbol() const { return symbol; }
    const std::string& getTimeframe() const { return timeframe; }
};
