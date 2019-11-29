cmake_minimum_required(VERSION 2.8)

project(sqlite)

find_package(SQLite3 REQUIRED)

include_directories(${SQLite3_INCLUDE_DIRS})
enable_testing()

add_executable(sqlite_test test.cc)
target_link_libraries(sqlite_test ${SQLite3_LIBRARIES})

add_test(sqlite_test sqlite_test)

install(DIRECTORY sqlite DESTINATION include/iod
  FILES_MATCHING PATTERN "*.hh")
install(DIRECTORY sqlite DESTINATION include/iod
  FILES_MATCHING PATTERN "*.hpp")
