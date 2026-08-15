cmake_minimum_required(VERSION 3.16)
project(colored_water_sort_gui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(water_sort_gui WIN32 ../src/gui_main.cpp)
target_link_libraries(water_sort_gui PRIVATE user32 gdi32)

if(MSVC)
    target_compile_options(water_sort_gui PRIVATE /W4 /permissive-)
else()
    target_compile_options(water_sort_gui PRIVATE -Wall -Wextra -Wpedantic)
endif()

