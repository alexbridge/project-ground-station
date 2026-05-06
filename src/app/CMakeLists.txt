
add_executable(telemetry_api
    telemetry-api/telemetry-udp-api-main.cpp
    telemetry-api/telemetry-udp-api.cpp
)

target_link_libraries(telemetry_api PRIVATE libs)
target_link_libraries(telemetry_api PRIVATE telemetry)

add_executable(telemetry_ingestor 
    telemetry-ingestor/telemetry-ingestor-main.cpp
    telemetry-ingestor/telemetry-ingestor.cpp
)

target_link_libraries(telemetry_ingestor PRIVATE libs)
target_link_libraries(telemetry_ingestor PRIVATE telemetry)

target_include_directories(telemetry_api PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(telemetry_ingestor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(app_commons INTERFACE)
target_include_directories(app_commons INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})