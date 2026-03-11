cmake_minimum_required(VERSION 3.16)
project(ground_station)

set(CMAKE_CXX_STANDARD 17)
# sub-module force to 17
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# disable gnu extensions
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# External for sub-modules
add_subdirectory(src/libs)
add_subdirectory(src/telemetry)
add_subdirectory(src/app/telemetry-api)
add_subdirectory(src/app/telemetry-ingestor)

