
#include <lithium.hh>
#include "generic_sql_tests.hh"


int main() {
  auto db = li::sqlite_database("iod_sqlite_test.db");

  generic_sql_tests(db);
}
