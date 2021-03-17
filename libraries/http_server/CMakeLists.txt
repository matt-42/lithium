cmake_minimum_required(VERSION 3.0)

project(http_server)
find_package(Boost REQUIRED context)

include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory("examples")
add_subdirectory("tests")
add_subdirectory("benchmarks")

