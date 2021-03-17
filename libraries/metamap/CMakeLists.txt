cmake_minimum_required(VERSION 3.0)

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

li_add_executable(forward_tuple_as_metamap tests/forward_tuple_as_metamap.cc)
add_test(forward_tuple_as_metamap forward_tuple_as_metamap)
