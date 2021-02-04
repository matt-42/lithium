cmake_minimum_required(VERSION 3.0)

project(li::sql)

find_package(SQLite3 REQUIRED)
find_package(MYSQL REQUIRED)
find_package(PostgreSQL REQUIRED)

include_directories(${SQLite3_INCLUDE_DIRS})
include_directories(${MYSQL_INCLUDE_DIR})
include_directories(${PostgreSQL_INCLUDE_DIRS})

enable_testing()

#add_definitions( -fsanitize=address -fno-omit-frame-pointer )
#set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
#set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

li_add_executable(sqlite_test tests/sqlite.cc)
target_link_libraries(sqlite_test ${SQLite3_LIBRARIES})
add_test(sqlite_test sqlite_test)

li_add_executable(mysql_test tests/mysql.cc)
target_link_libraries(mysql_test ${MYSQL_LIBRARY})
add_test(mysql_test mysql_test)

li_add_executable(pgsql_test tests/pgsql.cc)
target_link_libraries(pgsql_test ${PostgreSQL_LIBRARIES})
add_test(pgsql_test pgsql_test)

li_add_executable(orm_test tests/sql_orm.cc)
target_link_libraries(orm_test ${MYSQL_LIBRARY} ${PostgreSQL_LIBRARIES} ${SQLite3_LIBRARIES})
add_test(orm_test orm_test)
