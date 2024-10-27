cmake_minimum_required(VERSION 3.0)
project(cash)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add executable
add_executable(cash src/cash.cpp
        src/cash.h)
