cmake_minimum_required(VERSION 2.8)

project(mysql)

find_package(MYSQL REQUIRED)

message(${MYSQL_INCLUDE_DIR})
include_directories(${MYSQL_INCLUDE_DIR})
enable_testing()

add_executable(mysql_test test.cc)
target_link_libraries(mysql_test ${MYSQL_LIBRARY})

add_test(sqlite_test sqlite_test)

install(DIRECTORY sqlite DESTINATION include/iod
  FILES_MATCHING PATTERN "*.hh")
install(DIRECTORY sqlite DESTINATION include/iod
  FILES_MATCHING PATTERN "*.hpp")
