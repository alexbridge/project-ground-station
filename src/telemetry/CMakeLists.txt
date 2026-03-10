add_library(telemetry STATIC
    storage/telemetry-storage.cpp
)

# local deps
target_link_libraries(telemetry PRIVATE libs)

# expose lib
target_include_directories(telemetry PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

