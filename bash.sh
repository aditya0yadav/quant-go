#!/bin/bash

# Set project root folder name
PROJECT_NAME="backtester"

# Create root directory
mkdir -p $PROJECT_NAME

# Enter project directory
cd $PROJECT_NAME

# Create folders
mkdir -p include src data build tests

# Create README.md
echo "# Backtester Project" > README.md

# Create CMakeLists.txt with minimal starter content
cat <<EOL > CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(backtester)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

file(GLOB SOURCES "src/*.cpp")

add_executable(backtester \${SOURCES})
EOL

# Create include header files
touch include/DataPoint.h
touch include/DataLoader.h
touch include/BacktestEngine.h
touch include/Strategy.h

# Create src implementation files
touch src/main.cpp
touch src/DataLoader.cpp
touch src/BacktestEngine.cpp
touch src/Strategy.cpp

# Create sample data file
echo "timestamp,open,high,low,close,volume" > data/BTCUSDT.csv
echo "2022-01-01 00:00:00,46000.25,46120.50,45950.10,46080.30,125.5" >> data/BTCUSDT.csv

# Create test folder files
touch tests/test_dataloader.cpp

# Print success message
echo "✅ Project folder structure created successfully!"
