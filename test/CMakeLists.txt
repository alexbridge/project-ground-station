
function(add_producer name source)
    add_executable(${name} ${source})
    target_link_libraries(${name} PRIVATE libs app_commons telemetry)
    target_include_directories(${name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
endfunction()

add_producer(af_unix_dgram_producer producer/af-unix-dgram-telemetry-producer.cpp)
add_producer(udp_producer producer/udp-telemetry-producer.cpp)

