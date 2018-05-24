cmake_minimum_required(VERSION 3.8)

project(containers)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "-march=native -Wall -Wextra -Wconversion -Wshadow -Wcast-qual -pedantic -pedantic-errors")
set(CMAKE_CXX_FLAGS_DEBUG  "-g3 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

find_package(benchmark REQUIRED)
find_package(Threads REQUIRED)

include_directories(include/)
include_directories(SYSTEM libs/include/)

add_executable(test_set test/catch_main.cpp test/set.cpp)

add_executable(test_utility test/catch_main.cpp test/utility.cpp)

add_executable(bench_set bench/set.cpp)
target_link_libraries(bench_set benchmark Threads::Threads)
