
#include "symbols.hh"
#include <cassert>
#include <iostream>
#include <li/sql/mysql.hh>

using namespace s;

/* Mysql setup script:

CREATE TABLE users
(
id int,
name varchar(255),
age int
);
INSERT into users(id, name, age) values (1, "John", 42);
INSERT into users(id, name, age) values (2, "Bob", 24);
*/

// inline double get_time_in_seconds()
// {
//   timespec ts;
//   clock_gettime(CLOCK_REALTIME, &ts);
//   return double(ts.tv_sec) + double(ts.tv_nsec) / 1000000000.;
// }

int main() {
  using namespace li;

  try {
    auto m = mysql_database(s::host = "127.0.0.1", s::database = "mysql_test", s::user = "root",
                            s::password = "lithium_test", s::port = 14550, s::charset = "utf8");

    auto c = m.connect();

    c("DROP table if exists users_test_mysql;");
    c("CREATE TABLE users_test_mysql (id int,name varchar(255),age int);");

    // Prepared statement.
    auto insert_user = c.prepare("INSERT into users_test_mysql(id, name, age) values (?,?,?);");

    // Execute the statement.
    insert_user(1, "John", 42);
    insert_user(2, "Bob", 24);

    int count = c("SELECT count(*) from users_test_mysql").read<int>();
    std::cout << count << std::endl;
    assert(count == 2);

    auto test_optional_stmt = c.prepare("SELECT id from users_test_mysql where id = ?");
    assert(test_optional_stmt(42).read_optional<int>().has_value() == false);
    assert(test_optional_stmt(3).read_optional<int>().value() == 2);

    std::string test_str = "dgfad0875g9f658g8w97f32orjw0r89fuhq07fy0rjgq3478fyqh03g7y0b347fyj08yg034f78yj047yh078fy0fyj40";
    std::string str = c("SELECT '" + test_str+ "'").read<std::string>();
    assert(str = test_str);

    int i = 0;
    c.prepare("Select id,name from users_test_mysql;")().map([&](int id, std::string name) {
      if (i == 0) {
        assert(id == 1);
        assert(name == "John");
      }
      if (i == 1) {
        assert(id == 3);
        assert(name == "Bob");
      }
      i++;
    });
  
  } catch (const std::exception& e) {
    std::cout << "error during test: " << e.what() << std::endl;
    return 1;
  }
}
