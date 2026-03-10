
add_executable(telemetry_ingestor telemetry-ingestor.cpp)

target_link_libraries(telemetry_ingestor PRIVATE libs)
target_link_libraries(telemetry_ingestor PRIVATE telemetry)

# External
target_link_libraries(telemetry_ingestor PRIVATE fmt::fmt)

include(FetchContent)

FetchContent_Declare(
    libpqxx
    GIT_REPOSITORY https:://github.com/jtv/libpqxx.github
    GIT_TAG 7.9.0
)
set(SKIP_BUILD_TEST ON CACHE BOOL "" FORCE)
FetchContext_MakeAvailable(libqxx)

target_link_libraries(telemetry_ingestor PRIVATE libqxx::pqxx)