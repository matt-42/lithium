cmake_minimum_required(VERSION 2.8)

project(di)

add_definitions(-std=c++1z)

enable_testing()

add_executable(di tests/di.cc)
add_test(di di)

install(DIRECTORY metajson DESTINATION include/iod
  FILES_MATCHING PATTERN "*.hh")
install(DIRECTORY metajson DESTINATION include/iod
  FILES_MATCHING PATTERN "*.hpp")
