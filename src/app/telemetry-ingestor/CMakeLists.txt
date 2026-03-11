
add_executable(telemetry_ingestor telemetry-ingestor.cpp)

target_link_libraries(telemetry_ingestor PRIVATE libs)
target_link_libraries(telemetry_ingestor PRIVATE telemetry)
