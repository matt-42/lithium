cmake_minimum_required(VERSION 2.8)

project(metamap)

add_definitions(-std=c++1z)

enable_testing()

add_executable(make tests/make.cc)
add_test(make make)

add_executable(cat tests/cat.cc)
add_test(cat cat)

add_executable(intersection tests/intersection.cc)
add_test(intersection intersection)

add_executable(substract tests/substract.cc)
add_test(substract substract)

add_executable(tuple tests/tuple.cc)
add_test(tuple tuple)


install(DIRECTORY metamap DESTINATION include/iod
  FILES_MATCHING PATTERN "*.hh")
install(DIRECTORY metamap DESTINATION include/iod
  FILES_MATCHING PATTERN "*.hpp")
