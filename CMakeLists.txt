cmake_minimum_required(VERSION 3.16)
project(ground_station)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(src/libs)
add_subdirectory(src/telemetry)
add_subdirectory(src/app/telemetry-api)
add_subdirectory(src/app/telemetry-ingestor)
