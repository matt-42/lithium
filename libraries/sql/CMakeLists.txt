cmake_minimum_required(VERSION 2.8)

project(sqlite)

find_package(SQLite3 REQUIRED)
find_package(MYSQL REQUIRED)

include_directories(${SQLite3_INCLUDE_DIRS})
include_directories(${MYSQL_INCLUDE_DIR})

enable_testing()

add_executable(sqlite_test tests/sqlite.cc)
target_link_libraries(sqlite_test ${SQLite3_LIBRARIES})
add_test(sqlite_test sqlite_test)

add_executable(mysql_test tests/mysql.cc)
target_link_libraries(mysql_test ${MYSQL_LIBRARY})
add_test(mysql_test mysql_test)

add_executable(orm_test tests/sql_orm.cc)
target_link_libraries(orm_test ${MYSQL_LIBRARY} ${SQLite3_LIBRARIES})
add_test(orm_test orm_test)

install(DIRECTORY sql DESTINATION include/li
  FILES_MATCHING PATTERN "*.hh")
install(DIRECTORY sql DESTINATION include/li
  FILES_MATCHING PATTERN "*.hpp")
