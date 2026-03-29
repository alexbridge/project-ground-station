cmake_minimum_required(VERSION 3.16)
project(ground_station)

set(CMAKE_CXX_STANDARD 17)
# sub-module force to 17
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# disable gnu extensions
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Warnings
add_compile_options(
    -Wall -Wextra -Wpedantic -Werror
    -Wshadow -Wconversion -Wsign-conversion
    -Wnull-dereference -Wdouble-promotion
)

# Hardening
add_compile_options(-fstack-protector-strong)
add_compile_definitions(_FORTIFY_SOURCE=2)

# Debug: sanitizers
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

# External for sub-modules
add_subdirectory(src/libs)
add_subdirectory(src/telemetry)
add_subdirectory(src/app)
add_subdirectory(test)

