#include <li/json/json.hh>
#include <li/sql/sql_orm.hh>
#include <li/sql/sqlite.hh>
#include <li/sql/mysql.hh>
#include <cassert>

#include "symbols.hh"

int main() {

  using namespace li;

  auto test_with_db = [] (auto& db) {
    auto schema = sql_orm_schema("users_orm_test")
                  .fields(s::id(s::auto_increment, s::primary_key) = int(), s::age(s::read_only) = int(), 
                          s::name = std::string(),
                          s::login = std::string())
                  .callbacks(
                    s::after_insert = [] (auto p) { std::cout << "inserted " << json_encode(p) << std::endl; },
                    s::after_destroy = [] (auto p) { std::cout << "removed " << json_encode(p) << std::endl;})
                    ;
    auto orm = schema.connect(db);
    auto c = db.get_connection();

    orm.drop_table_if_exists();
    orm.create_table_if_not_exists();

    // find one.
    assert(!orm.find_one(s::id = 1));

    // Insert.
    long long int john_id = orm.insert(s::name = "John", s::age = 42, s::login = "lol");
    assert(orm.count() == 1);
    std::cout << john_id << std::endl;
    auto u = orm.find_one(s::id = john_id);
    assert(u);
    assert(u->id = john_id and u->name == "John" and u->age == 42 and u->login == "lol");

    // Update.
    u->name = "John2";
    u->age = 31;
    u->login = "foo";
    orm.update(*u);
    assert(orm.count() == 1);
    auto u2 = orm.find_one(s::id = john_id);
    assert(u2);
    // age field is read only. It should not have been affected by the update.
    assert(u2->name == "John2" and u2->age == 42 and u2->login == "foo");

    // Remove.
    orm.remove(*u2);
    assert(orm.count() == 0);

  };


  auto sqlite_db = sqlite_database("iod_sqlite_test_orm.db");
  test_with_db(sqlite_db);

  auto mysql_db = mysql_database(s::host = "127.0.0.1",
                                s::database = "silicon_test",
                                s::user = "root",
                                s::password = "sl_test_password",
                                s::port = 14550,
                                s::charset = "utf8");
  test_with_db(mysql_db);


}
