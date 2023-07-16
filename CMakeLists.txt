cmake_minimum_required(VERSION 3.25)
project(NumbersPuzzle C)

set(CMAKE_C_STANDARD 11)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

add_executable(NumbersPuzzle main.c)
target_link_libraries(NumbersPuzzle curses)