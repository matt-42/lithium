cmake_minimum_required(VERSION 3.0)

project(iod_symbol)

enable_testing()

add_executable(li_symbol_generator symbol/symbol_generator.cc)

li_add_executable(test_symbol tests/test.cc)
add_test(test_symbol test_symbol)

install(TARGETS li_symbol_generator DESTINATION bin)
