#include <iostream>
#include <thread>

#include <lithium.hh>

#include "symbols.hh"
#include "test.hh"

using namespace li;

int main() {


  auto test_with_db = [](auto& db, int port) {

    auto user_schema =
        sql_orm_schema<std::decay_t<decltype(db)>>(db, "users")
            .fields(s::id(s::auto_increment, s::primary_key) = int(), s::name = std::string(),
                    s::age = int(), s::address = std::string(), s::city = std::string())
            .callbacks(s::before_insert = [](auto& user, http_request&, http_response&) {
              user.city = "Paris";
            });

    auto orm = user_schema.connect();
    std::cout << "Connected to db!" << std::endl;
    orm.drop_table_if_exists();
    orm.create_table_if_not_exists();

    assert(orm.count() == 0);

    http_api api;

    // Crud for the User object.
    api.add_subapi("/user", sql_crud_api(user_schema));

    api.print_routes();
    // std::cout << api_description(api) << std::endl;

    // Start server.
    http_serve(api, port, s::non_blocking);
    auto c = http_client("http://localhost:" + boost::lexical_cast<std::string>(port));

    // Insert.
    auto to_insert = mmm(s::name = "matt", s::age = 12, s::address = "USA", s::city = "Paris");
    auto insert_r = c.post("/user/create", s::post_parameters = to_insert);
    insert_r = c.post("/user/create", s::post_parameters = to_insert);

    std::cout << insert_r.body << std::endl;

    //  assert(insert_r.status == 200);
    auto pid = mmm(s::id = int());
    assert(json_decode(insert_r.body, pid).good());
    int id = pid.id;
    std::cout << id << std::endl;
    // Get by id.
    auto get_r = c.post("/user/find_by_id", s::post_parameters = mmm(s::id = id));
    std::cout << get_r.body << std::endl;

    assert(get_r.status == 200);
    auto matt = user_schema.all_fields();
    json_decode(get_r.body, matt);
    assert(matt.name == "matt");
    assert(matt.age == 12);
    assert(matt.city == "Paris");
    assert(matt.address == "USA");

    auto get_r2 = c.post("/user/find_by_id", s::post_parameters = mmm(s::id = 42));
    std::cout << json_encode(get_r2) << std::endl;
    ;
    assert(get_r2.status == 404);

    // // Update
    auto update_r =
        c.post("/user/update", s::post_parameters = mmm(s::id = id, s::name = "john", s::age = 42,
                                                        s::address = "Canad a", s::city = "xxx"));
    auto get_r3 = c.post("/user/find_by_id", s::post_parameters = mmm(s::id = id));
    std::cout << json_encode(get_r3) << std::endl;
    assert(get_r3.status == 200);
    auto john = user_schema.all_fields();
    json_decode(get_r3.body, john);
    assert(john.id == id);
    assert(john.age == 42);
    assert(john.address == "Canad a");
    assert(john.city == "xxx");
    CHECK_EQUAL("update name", john.name, "john");

    std::cout << json_encode(get_r3.body) << std::endl;
    ;

    // // Destroy.
    // auto destroy_r = c(HTTP_POST, "/user/destroy", s::post = mmm(s::id = id));
    // assert(destroy_r.status == 200);

    // auto get_r4 = c(HTTP_GET, "/user/get_by_id", s::get = mmm(s::id = id));
    // assert(get_r4.status == 404);

    // exit(0);
  };

  auto sqlite_db = sqlite_database("iod_sqlite_crud_test.db");
  test_with_db(sqlite_db, 12358);

  auto mysql_db =
      mysql_database(s::host = "127.0.0.1", s::database = "mysql_test", s::user = "root",
                     s::password = "lithium_test", s::port = 14550, s::charset = "utf8");
  test_with_db(mysql_db, 12359);

  auto pgsql_db = pgsql_database(s::host = "localhost", s::database = "postgres", s::user = "postgres",
                            s::password = "lithium_test", s::port = 32768, s::charset = "utf8");
  test_with_db(pgsql_db, 12361);

}
