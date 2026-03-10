add_library(libs
    STATIC
        socket/udp-socket.cpp
        socket/af-unix-socket.cpp
)

# expose lib
target_include_directories(libs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# External transient
target_link_libraries(libs PUBLIC fmt::fmt)
target_link_libraries(libs PUBLIC spdlog::spdlog)