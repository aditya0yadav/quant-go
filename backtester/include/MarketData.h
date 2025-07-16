#pragma once

#include <string>
#include <map>
#include <memory>
#include "DataLoader.h"

class MarketData {
private:
    // Nested map: symbol -> timeframe -> DataLoader
    std::map<std::string, std::map<std::string, std::unique_ptr<DataLoader>>> dataMap;

public:
    void load(const std::string& symbol, const std::string& timeframe, const std::string& filename, const std::string& configFile) {
        auto loader = std::make_unique<DataLoader>();
        loader->loadMapping(configFile);
        loader->loadCSV(filename, symbol, timeframe);

        dataMap[symbol][timeframe] = std::move(loader);
    }

    const DataLoader* getLoader(const std::string& symbol, const std::string& timeframe) const {
        auto it = dataMap.find(symbol);
        if (it != dataMap.end()) {
            auto tf_it = it->second.find(timeframe);
            if (tf_it != it->second.end()) {
                return tf_it->second.get();
            }
        }
        return nullptr;
    }
};
