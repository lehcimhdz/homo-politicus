cmake_minimum_required(VERSION 3.10)
project(HomoPoliticus)

# C++ Standard 17 is good for modern C++ features
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directory for our header files
include_directories(include)

# Gather all source files
file(GLOB SOURCES "src/*.cpp")

# Create the executable
add_executable(HomoPoliticus ${SOURCES})
