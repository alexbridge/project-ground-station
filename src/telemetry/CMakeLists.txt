add_library(telemetry STATIC
    storage/telemetry-storage.cpp
)

# local deps
target_link_libraries(telemetry PRIVATE libs)

# expose lib
target_include_directories(telemetry PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include(FetchContent)
FetchContent_Declare(
    libpqxx
    GIT_REPOSITORY https://github.com/jtv/libpqxx.git
    GIT_TAG 7.10.5
)
FetchContent_MakeAvailable(libpqxx)

target_link_libraries(telemetry PRIVATE libpqxx::pqxx)

