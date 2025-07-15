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

public:
    void loadMapping(const std::string& configFile);
    void loadCSV(const std::string& filename);
    const std::vector<DataPoint>& getData() const;
};
