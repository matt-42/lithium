#include <iod/metajson/metajson.hh>
#include <iod/silicon/sql_orm.hh>
#include <iod/sqlite/sqlite.hh>
#include <cassert>

IOD_SYMBOL(age)
IOD_SYMBOL(login)

int main() {

  using namespace iod;

  auto db = sqlite_database("iod_sqlite_test.db");
  auto schema = sql_orm_shema("users")
                 .fields(s::id(s::auto_increment, s::primary_key) = int(), s::age = int(), s::name = std::string(),
                         s::login = std::string())
                 .callbacks(
                   s::after_insert = [] (auto p) { std::cout << "inserted " << json_encode(p) << std::endl; },
                   s::after_destroy = [] (auto p) { std::cout << "removed " << json_encode(p) << std::endl;})
                  ;                  
  auto orm = schema.connect(db);
  auto c = db.get_connection();

  c("DROP TABLE IF EXISTS users;")();
  orm.create_table_if_not_exists();


  // Insert.
  int john_id = orm.insert(s::name = "John", s::age = 42, s::login = "lol");
  assert(orm.count() == 1);
  auto u = orm.find_by_id(john_id);
  assert(u.id = john_id and u.name == "John" and u.age == 42 and u.login == "lol");

  // Update.
  u.name = "John2";
  u.age = 31;
  u.login = "foo";
  orm.update(u);
  assert(orm.count() == 1);
  auto u2 = orm.find_by_id(john_id);
  assert(u2.name == "John2" and u2.age == 31 and u2.login == "foo");

  // Remove.
  orm.remove(u2);
  assert(orm.count() == 0);
}
