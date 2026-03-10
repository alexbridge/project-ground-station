cmake_minimum_required(VERSION 3.16)
project(ground_station)

set(CMAKE_CXX_STANDARD 17)

# External for sub-modules
include(FetchContent)
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 10.2.1
)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(spdlog)

add_subdirectory(src/libs)
add_subdirectory(src/telemetry)
add_subdirectory(src/app/telemetry-api)
add_subdirectory(src/app/telemetry-ingestor)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
