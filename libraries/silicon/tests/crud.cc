#include <iostream>
#include <thread>

#include <iod/http_client/http_client.hh>
#include <iod/silicon/silicon.hh>
#include <iod/sqlite/sqlite.hh>

#include "symbols.hh"

using namespace iod;

int main() {

  api<http_request, http_response> api;

  auto db = sqlite_database("/tmp/sl_test_crud.sqlite", s::synchronous = 1); // sqlite middleware.

  auto user_schema = sql_orm_schema("users")
                      .fields(s::id(s::autoset, s::primary_key) = int(), 
                              s::name = std::string(),
                              s::age = int(),
                              s::address = std::string(),
                              s::city = std::string())
                      .callbacks(s::before_insert = [](auto& user) { user.city = "Paris"; });

  auto orm = user_schema.connect(db);
  orm.drop_table_if_exists();
  orm.create_table_if_not_exists();


  // Crud for the User object.
  api.add_subapi("/user", sql_crud(db, user_schema));

  // std::cout << api_description(api) << std::endl;

  // Start server.
  auto server = http_serve(api, 12345, s::non_blocking);

  // // // Test.
  auto c = http_client("http://127.0.0.1:12345");

  // // // Insert.
  auto to_insert = make_metamap(s::name = "matt", s::age = 12, s::address = "USA");
  auto insert_r = c.post("/user/create", s::post_parameters = to_insert);
  insert_r = c.post("/user/create", s::post_parameters = to_insert);

  std::cout << insert_r.body << std::endl;

  //  assert(insert_r.status == 200);
  auto pid = make_metamap(s::id = int());
  assert(json_decode(insert_r.body, pid).good());
  int id = pid.id;
  // Get by id.
  auto get_r = c.post("/user/find_by_id", s::post_parameters = make_metamap(s::id = id));
  std::cout << get_r.body << std::endl;

  // ;
  // assert(get_r.status == 200);
  // auto matt = json_decode<User>(get_r.body);
  // assert(matt.name == "matt");
  // assert(matt.age == 12);
  // assert(matt.city == "Paris");
  // assert(matt.address == "USA");

  // auto get_r2 = c.get("/user/get_by_id", s::get_parameters = make_metamap(s::id = 42));
  // std::cout << get_r2.body << std::endl;
  // ;
  // assert(get_r2.status == 404);

  // // Update
  // auto update_r =
  //     c(POST, "/user/update",
  //       s::post_parameters = make_metamap(s::id = id, s::name = "john", s::age = 42, s::address = "Canad a"));
  // auto get_r3 = c(GET, "/user/get_by_id", s::get_parameters = make_metamap(s::id = id));
  // assert(get_r3.status == 200);
  // auto john = json_decode<User>(get_r.body);
  // assert(john.id == id);
  // assert(john.name == "john");
  // assert(john.age == 42);
  // assert(john.address == "Canad a");

  // std::cout << json_encode(get_r3.body) << std::endl;
  // ;

  // // Destroy.
  // auto destroy_r = c(POST, "/user/destroy", s::post = make_metamap(s::id = id));
  // assert(destroy_r.status == 200);

  // auto get_r4 = c(GET, "/user/get_by_id", s::get = make_metamap(s::id = id));
  // assert(get_r4.status == 404);

  // exit(0);
}
