cmake_minimum_required(VERSION 2.8)

project(sqlite)



enable_testing()

add_executable(sqlite_test test.cc)
target_link_libraries(sqlite_test sqlite3)

add_test(sqlite_test sqlite_test)

install(DIRECTORY sqlite DESTINATION include/iod
  FILES_MATCHING PATTERN "*.hh")
install(DIRECTORY sqlite DESTINATION include/iod
  FILES_MATCHING PATTERN "*.hpp")
