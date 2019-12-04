cmake_minimum_required(VERSION 2.8)

project(iod_symbol)

enable_testing()

add_executable(li_symbol_generator symbol/symbol_generator.cc)

li_add_executable(test_symbol tests/test.cc)
add_test(test_symbol test_symbol)

install(DIRECTORY symbol DESTINATION include/li
  FILES_MATCHING PATTERN "*.hh")
install(DIRECTORY symbol DESTINATION include/li
  FILES_MATCHING PATTERN "*.hpp")
