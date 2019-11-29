cmake_minimum_required(VERSION 2.8)

project(silicon)
find_package(Boost REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory("tests")


install(DIRECTORY silicon DESTINATION include
  FILES_MATCHING PATTERN "*.hh")
install(DIRECTORY silicon DESTINATION include
  FILES_MATCHING PATTERN "*.hpp")
