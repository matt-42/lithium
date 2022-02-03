
#include "symbols.hh"
#include <cassert>
#include <iostream>
#include <lithium_pgsql.hh>
#include "generic_sql_tests.hh"

int main() {
  using namespace li;

  auto database = pgsql_database(s::host = "127.0.0.1", s::database = "postgres", s::user = "postgres",
                          s::password = "lithium_test", s::port = 32768, s::charset = "utf8");

  generic_sql_tests(database);
  database.clear_connections();
}
