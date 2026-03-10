add_library(libs STATIC
    socket/udp-socket.cpp
    socket/af-unix-socket.cpp
    logging/logger.cpp
    )

target_include_directories(libs PUBLIC ${CMAKE_SOURCE_DIR}/src/libs)

# External
target_link_libraries(libs PUBLIC spdlog::spdlog)