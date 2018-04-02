cmake_minimum_required(VERSION 3.9)
project(gpsAnalyser)

set(CMAKE_CXX_STANDARD 17)

include_directories(libs/rapidxml-1.13)
include_directories(include)

add_executable(gpsAnalyser src/main.cpp src/Track.cpp include/Track.h include/util.h)