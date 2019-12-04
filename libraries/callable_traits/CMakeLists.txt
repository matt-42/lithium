cmake_minimum_required(VERSION 2.8)

project(callable_traits)

enable_testing()


install(DIRECTORY callable_traits DESTINATION include/li
  FILES_MATCHING PATTERN "*.hh")
install(DIRECTORY callable_traits DESTINATION include/li
  FILES_MATCHING PATTERN "*.hpp")
