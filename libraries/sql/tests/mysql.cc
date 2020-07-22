
#include "symbols.hh"
#include <cassert>
#include <iostream>
#include <lithium_mysql.hh>

#include "generic_sql_tests.hh"

int main() {
  using namespace li;

  auto database =
      mysql_database(s::host = "127.0.0.1", s::database = "mysql_test", s::user = "root",
                     s::password = "lithium_test", s::port = 14550, s::charset = "utf8");

  generic_sql_tests(database);

  database.clear_connections();
}
