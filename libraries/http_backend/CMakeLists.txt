cmake_minimum_required(VERSION 2.8)

project(silicon)
find_package(Boost REQUIRED filesystem context)

include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory("examples")
add_subdirectory("tests")
add_subdirectory("benchmarks")


install(DIRECTORY http_backend DESTINATION include/li
  FILES_MATCHING PATTERN "*.hh")
install(DIRECTORY http_backend DESTINATION include/li
  FILES_MATCHING PATTERN "*.hpp")
