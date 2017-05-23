cmake_minimum_required(VERSION 2.8)
project(anl_bench)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c tinycthread.c)
add_executable(anl_bench ${SOURCE_FILES})