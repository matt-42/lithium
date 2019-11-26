#include <iostream>
#include <thread>

#include <iod/http_client/http_client.hh>
#include <iod/silicon/api.hh>
#include <iod/silicon/mhd.hh>
#include <iod/silicon/sql_crud.hh>
#include <iod/silicon/sql_orm.hh>
#include <iod/sqlite/sqlite.hh>

#include "symbols.hh"

using namespace s;

int main() {
  using namespace sl;

  http_api api;

  auto db = sqlite_database("/tmp/sl_test_crud.sqlite", s::synchronous = 1); // sqlite middleware.

  auto user_orm = sql_orm(db, "users")
                      .fields(s::id(s::auto_increment, s::primary_key) = int(), 
                              s::name = std::string(),
                              s::age = int(),
                              s::address = std::string(),
                              s::city(s::read_only) = std::string())
                      .callbacks(s::before_insert = [](auto& user) { u.city = "Paris"; });

  user_orm.create_table_if_not_exists();

  // Crud for the User object.
  api("user") = sql_crud(user_orm);

  // std::cout << api_description(api) << std::endl;

  // Start server.
  auto server = http_serve(api, factories, 12345, s::non_blocking);

  // Test.
  auto c = libcurl_json_client(api, "127.0.0.1", 12345);

  // Insert.
  auto to_insert = make_metamap(s::name = "matt", s::age = 12, s::address = "USA");
  auto insert_r = c(GET, "/user/create", s::post_url_encoded = to_insert);

  std::cout << json_encode(insert_r) << std::endl;
  assert(insert_r.status == 200);
  int id = decode_json(insert_r.body, s::id = int()).id;

  // Get by id.
  auto get_r = c(GET, "/user/get_by_id", s::get = make_metamap(s::id = id));
  std::cout << get_r.body << std::endl;
  ;
  assert(get_r.status == 200);
  auto matt = json_decode<User>(get_r.body);
  assert(matt.name == "matt");
  assert(matt.age == 12);
  assert(matt.city == "Paris");
  assert(matt.address == "USA");

  auto get_r2 = c(GET, "/user/get_by_id", s::get = make_metamap(s::id = 42));
  std::cout << get_r2.body << std::endl;
  ;
  assert(get_r2.status == 404);

  // Update
  auto update_r =
      c(POST, "/user/update",
        s::post_parameters = make_metamap(s::id = id, s::name = "john", s::age = 42, s::address = "Canad a"));
  auto get_r3 = c(GET, "/user/get_by_id", s::get_parameters = make_metamap(s::id = id));
  assert(get_r3.status == 200);
  auto john = json_decode<User>(get_r.body);
  assert(john.id == id);
  assert(john.name == "john");
  assert(john.age == 42);
  assert(john.address == "Canad a");

  std::cout << json_encode(get_r3.body) << std::endl;
  ;

  // Destroy.
  auto destroy_r = c(POST, "/user/destroy", s::post = make_metamap(s::id = id));
  assert(destroy_r.status == 200);

  auto get_r4 = c(GET, "/user/get_by_id", s::get = make_metamap(s::id = id));
  assert(get_r4.status == 404);

  exit(0);
}
