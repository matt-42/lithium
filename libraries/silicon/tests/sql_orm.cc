#include <iod/silicon/sql_orm.hh>
#include <iod/sqlite/sqlite.hh>
#include <iod/metajson/metajson.hh>

IOD_SYMBOL(age)
IOD_SYMBOL(login)
IOD_SYMBOL(login2)
IOD_SYMBOL(login3)
IOD_SYMBOL(login4)


int main() {
  using namespace iod;


  auto db = sqlite_database("iod_sqlite_test.db");
  auto orm = sql_orm(db, "users")
                 .fields(s::id(s::auto_increment, s::primary_key) = int(), 
                 s::name = std::string(),
                 s::login = std::string(),
                 s::login2 = std::string(),
                 s::login3 = std::string(),
                 s::login4 = std::string(),
                         s::age = int());

  auto c = db.get_connection();
  c("DROP TABLE IF EXISTS users;")();

  //c("CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY AUTOINCREMENT, "
  //  "name VARCHAR, age INTEGER, login VARCHAR, login2 VARCHAR, login3 VARCHAR, login4 VARCHAR);")();

  auto orm_connection = orm.get_connection();
  orm.create_table_if_not_exists();
  auto users = std::vector{make_metamap(s::name = "John", s::age = 42, s::login = "x", s::login2 = "x", s::login3 = "x254", s::login4 = "x"),
                        make_metamap(s::name = "John2", s::age = 43, s::login = "x", s::login2 = "x", s::login3 = "x4", s::login4 = "x")};

  orm_connection.insert(users[0]);
  orm_connection.insert(s::name = "John2", s::age = 43, s::login = "x", s::login2 = "x", s::login3 = "x4", s::login4 = "x");

  int i = 0;
  orm_connection.forall([&](auto u) {
    assert(users[i].age == u.age);
    assert(users[i].login == u.login);
    assert(users[i].name == u.name);
    std::cout << json_encode(u) << std::endl;
    i++;
  });


  c("select id, name, age from users") | [](int id, std::string name, int age) {
    std::cout << id << " " << name << " " << age << std::endl;
  };
}
