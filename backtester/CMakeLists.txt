cmake_minimum_required(VERSION 3.10)
project(backtester)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

file(GLOB SOURCES "src/*.cpp")

add_executable(backtester ${SOURCES})
target_include_directories(backtester PRIVATE ${PROJECT_SOURCE_DIR}/include)
