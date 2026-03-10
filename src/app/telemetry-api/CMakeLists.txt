add_executable(telemetry_api telemetry-udp-api-main.cpp)

target_link_libraries(telemetry_api PRIVATE libs)
target_link_libraries(telemetry_api PRIVATE telemetry)
