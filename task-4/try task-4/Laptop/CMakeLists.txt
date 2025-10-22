cmake_minimum_required(VERSION 3.10)
project(LaptopTransfer)

# Pakai C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Tambah folder src sebagai include
include_directories(${PROJECT_SOURCE_DIR}/src)

# Executable
add_executable(laptop
    src/main.cpp
    src/Laptop.cpp
)
