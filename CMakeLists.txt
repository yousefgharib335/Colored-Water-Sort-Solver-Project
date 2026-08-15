cmake_minimum_required(VERSION 3.16)
project(colored_water_sort_solver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(water_sort_solver src/main.cpp)

if(MSVC)
    target_compile_options(water_sort_solver PRIVATE /W4 /permissive-)
else()
    target_compile_options(water_sort_solver PRIVATE -Wall -Wextra -Wpedantic)
endif()

