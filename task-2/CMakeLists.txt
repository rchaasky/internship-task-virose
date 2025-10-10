cmake_minimum_required(VERSION 3.15)

# Project Name
project("RobotConfig")

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(rapidjson_include_dir "${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include/")

include_directories(${rapidjson_include_dir})

add_executable(test Test.cpp)

# target_link_libraries()
