cmake_minimum_required(VERSION 3.0)

project(li::sql)

li_add_executable(sqlite_test tests/sqlite.cc)
add_test(sqlite_test sqlite_test)

li_add_executable(mysql_test tests/mysql.cc)
add_test(mysql_test mysql_test)

li_add_executable(pgsql_test tests/pgsql.cc)
add_test(pgsql_test pgsql_test)

li_add_executable(orm_test tests/sql_orm.cc)
add_test(orm_test orm_test)
