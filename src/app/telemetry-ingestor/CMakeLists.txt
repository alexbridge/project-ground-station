
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)

add_executable(telemetry_ingestor telemetry-ingestor.cpp)

target_link_libraries(telemetry_ingestor PRIVATE libs)
target_link_libraries(telemetry_ingestor PRIVATE telemetry)

target_include_directories(telemetry_ingestor PRIVATE ${PQXX_INCLUDE_DIRS})
target_link_libraries(telemetry_ingestor PRIVATE ${PQXX_LIBRARIES})