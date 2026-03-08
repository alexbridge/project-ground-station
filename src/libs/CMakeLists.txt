add_library(libs STATIC
    socket/udp-socket.cpp
    socket/af-unix-socket.cpp
    )

target_include_directories(libs PUBLIC ${CMAKE_SOURCE_DIR}/src/libs)