add_library(libs
    STATIC
        socket/udp-socket.cpp
        socket/af-unix-socket.cpp
)

# expose lib
target_include_directories(libs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# External transient
include(FetchContent)
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 10.2.1
)
FetchContent_MakeAvailable(fmt)

# Tell spdlog to use external fmt
set(SPDLOG_FMT_EXTERNAL ON CACHE BOOL "" FORCE)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(spdlog)

target_link_libraries(libs PUBLIC fmt::fmt)
target_link_libraries(libs PUBLIC spdlog::spdlog)