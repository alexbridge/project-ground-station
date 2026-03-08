add_library(telemetry STATIC
    storage/telemetry-storage.cpp
    )

target_include_directories(telemetry PUBLIC ${CMAKE_SOURCE_DIR}/src/telemetry)