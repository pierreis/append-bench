cmake_minimum_required(VERSION 2.8)
project(anl_bench)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c tinycthread.c)
add_executable(anl_bench ${SOURCE_FILES})

find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
  set_property(TARGET anl_bench PROPERTY COMPILE_OPTIONS "-pthread")
  set_property(TARGET anl_bench PROPERTY INTERFACE_COMPILE_OPTIONS "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
  target_link_libraries(anl_bench "${CMAKE_THREAD_LIBS_INIT}")
endif()