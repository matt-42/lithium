
#include "symbols.hh"
#include <cassert>
#include <iostream>
#include <li/sql/mysql.hh>

#include "generic_sql_tests.hh"

int main() {
  using namespace li;

  // try {
    auto database = mysql_database(s::host = "127.0.0.1", s::database = "mysql_test", s::user = "root",
                            s::password = "lithium_test", s::port = 14550, s::charset = "utf8");

    generic_sql_tests(database);
    auto con = database.connect();
    
    con("DROP table if exists users_test_mysql;");
    con("CREATE TABLE users_test_mysql (id int,name TEXT,age int);");

    // Prepared statement.
    auto insert_user = con.prepare("INSERT into users_test_mysql(id, name, age) values (?,?,?);");

    // Execute the statement.
    insert_user(1, "John", 42);
    insert_user(2, "Bob", 24);

    int count = con("SELECT count(*) from users_test_mysql").read<int>();
    assert(count == 2);

    // Read optional prepared statement.
    auto test_optional_stmt = con.prepare("SELECT id from users_test_mysql where id = ?");
    assert(test_optional_stmt(42).read_optional<int>().has_value() == false);
    assert(test_optional_stmt(2).read_optional<int>().value() == 2);

    // Read optional non prepared query.
    assert(con("SELECT id from users_test_mysql where id = 2").read_optional<int>().value() == 2);
    assert(con("SELECT id from users_test_mysql where id = 42").read_optional<int>().has_value() == false);

    int i = 0;
    // Prepared statement map.
    con.prepare("Select id,name from users_test_mysql;")().map([&](int id, std::string name) {
      if (i == 0) {
        assert(id == 1);
        assert(name == "John");
      }
      if (i == 1) {
        assert(id == 2);
        assert(name == "Bob");
      }
      i++;
    });

    i = 0;
    // Non prepared query map.
    con("Select id,name from users_test_mysql;").map([&](int id, std::string name) {
      if (i == 0) {
        assert(id == 1);
        assert(name == "John");
      }
      if (i == 1) {
        assert(id == 2);
        assert(name == "Bob");
      }
      i++;
    });

    con("DELETE from users_test_mysql");
    std::string test_str_patern = "lkjlkjdgfad0875g9f658g8w97f32orjw0r89ofuhq07fy0rjgq3478fyqh03g7y0b347fyj08yg034f78yj047yh078fy0fyj40";
    std::string test_str = "";
    for (int k = 0; k < 10; k++)
    {
      test_str += test_str_patern;
      insert_user(k, test_str, 42);
    }

    i = 1;
    con.prepare("Select id,name from users_test_mysql;")().map([&](int id, std::string name) {
      assert(name.size() == 100*i);
      i++;
    });

  // } catch (const std::exception& e) {
  //   std::cerr << "error during test: " << e.what() << std::endl;
  //   throw e;
  //   return 1;
  // }
}
