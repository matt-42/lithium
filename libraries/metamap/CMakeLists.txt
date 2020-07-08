cmake_minimum_required(VERSION 2.8)

project(metamap)

enable_testing()

li_add_executable(make tests/make.cc)
add_test(make make)

li_add_executable(cat tests/cat.cc)
add_test(cat cat)

li_add_executable(intersection tests/intersection.cc)
add_test(intersection intersection)

li_add_executable(substract tests/substract.cc)
add_test(substract substract)

li_add_executable(tuple tests/tuple.cc)
add_test(tuple tuple)
