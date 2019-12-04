cmake_minimum_required(VERSION 2.8)

project(silicon)
find_package(Boost REQUIRED filesystem)

include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory("tests")


install(DIRECTORY http_backend DESTINATION include/li
  FILES_MATCHING PATTERN "*.hh")
install(DIRECTORY http_backend DESTINATION include/li
  FILES_MATCHING PATTERN "*.hpp")
