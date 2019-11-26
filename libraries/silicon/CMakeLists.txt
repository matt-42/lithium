cmake_minimum_required(VERSION 2.8)

project(silicon)

add_subdirectory("tests")
install(DIRECTORY silicon DESTINATION include
  FILES_MATCHING PATTERN "*.hh")
install(DIRECTORY silicon DESTINATION include
  FILES_MATCHING PATTERN "*.hpp")
