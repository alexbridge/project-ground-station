
add_executable(telemetry_ingestor telemetry-ingestor.cpp)

target_link_libraries(telemetry_ingestor PRIVATE libs)
target_link_libraries(telemetry_ingestor PRIVATE telemetry)

# External
include(FetchContent)
FetchContent_Declare(
    libpqxx
    GIT_REPOSITORY https://github.com/jtv/libpqxx.git
    GIT_TAG 7.10.5
)
FetchContent_MakeAvailable(libpqxx)

target_link_libraries(telemetry_ingestor PRIVATE libpqxx::pqxx)