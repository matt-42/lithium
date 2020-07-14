#include <cassert>
#include <lithium_mysql.hh>
#include <lithium_pgsql.hh>
#include <lithium_sqlite.hh>

#include "../../http_backend/tests/test.hh"

#include "symbols.hh"

int main() {

  using namespace li;

  auto test_with_db = [](auto& db) {
    auto schema = sql_orm_schema<decltype(db)>(db, "users_orm_test")
                      .fields(s::id(s::auto_increment, s::primary_key) = int(),
                              s::age(s::read_only) = int(125), s::login = std::string(),
                              s::name = std::string("default_name"))
                      .callbacks(s::after_insert =
                                     [](auto p, int data) {
                                       std::cout << "inserted " << json_encode(p) << std::endl;
                                       assert(data == 42);
                                     },
                                 s::after_remove =
                                     [](auto p, int data1, int data2) {
                                       std::cout << "removed " << json_encode(p) << std::endl;
                                       assert(data1 == 42);
                                       assert(data2 == 51);
                                     });
    auto c = db.connect();


    auto orm = schema.connect();

    orm.drop_table_if_exists();
    orm.create_table_if_not_exists();

    // find one.
    assert(!orm.find_one(s::id = 1));

    // Insert.
    long long int john_id = orm.insert(s::name = "John", s::age = 42, s::login = "lol", 42);
    assert(orm.count() == 1);
    //long long int john2_id2 = orm.insert(s::login = "lol", 42);
    std::cout << "last id: "<< john_id  << std::endl;
    auto u = orm.find_one(s::id = john_id);
    assert(u);
    std::cout << json_encode(u) << std::endl;
    assert(u->id = john_id and u->name == "John" and u->age == 42 and u->login == "lol");

    // Insert. Check if defaults value are kept.
    long long int john2_id = orm.insert(s::login = "lol", 42);
    auto john2 = orm.find_one(s::id = john2_id);
    CHECK_EQUAL("default value", john2->age, 125);
    CHECK_EQUAL("default value", john2->name, "default_name");
    orm.remove(*john2, 42, 51);

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
    orm.remove(*u2, 42, 51);
    assert(orm.count() == 0);

    orm.insert(s::name = "a", s::login = "a", s::age = 1, 42);
    orm.insert(s::name = "b", s::login = "b", s::age = 2, 42);
    orm.insert(s::name = "c", s::login = "c", s::age = 3, 42);
    assert(orm.count() == 3);

    int index = 0;
    orm.forall([&] (auto u) {
      assert(u.age == index + 1);
      assert(u.login[0] == 'a' + index);
      index++;
    });
  };

  auto sqlite_db = sqlite_database("iod_sqlite_test_orm.db");
  test_with_db(sqlite_db);

  auto mysql_db =
      mysql_database(s::host = "127.0.0.1", s::database = "mysql_test", s::user = "root",
                     s::password = "lithium_test", s::port = 14550, s::charset = "utf8");
  test_with_db(mysql_db);

  auto pgsql_db = pgsql_database(s::host = "localhost", s::database = "postgres", s::user = "postgres",
                            s::password = "lithium_test", s::port = 32768, s::charset = "utf8");
  test_with_db(pgsql_db);
}
