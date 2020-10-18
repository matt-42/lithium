/*
li::http_backend: Coroutine based, Asynchronous, Low Latency and High Throughput HTTP Server
=================================

This library goal is to ease the development of HTTP APIs without
compromising on performance. Coroutines (based on epoll and boost::context) 
are used to provide a asynchronous execution without impacting the ease
of use : Only few threads can serve thousands of clients.

All the communication asynchronous : client <-> server communication and
also server <-> database (except SQLite which does not use the network).


## Main features
  - High performance aync input/output engine based on epoll
  - PostgreSQL (async), Mysql (async), Sqlite (sync) connectors
  - Authentication
  - Object relational mapping for SQL databases
  - Static file serving
  - HTTP Pipelining
  - HTTPS

## Dependencies
  - OpenSSL
  - Boost

Write an api:
*/
// hello_lithium.cc
#include "lithium_http_backend.hh"

using namespace li;

int main() {
  // Build an api.
  http_api api;

  // Define a HTTP GET endpoint.
  api.get("/hello_world") = [&](http_request& request, http_response& response) {
    response.write("hello world.");
  };

  // Start a http server.
  http_serve(api, 12345);

  // Or start a https server:
  http_serve(api, 443, s::ssl_key = "./server.key", s::ssl_certificate = "./server.crt");

  // Optionally start a https with your ciphersuite
  http_serve(api, 443, s::ssl_key = "./server.key", s::ssl_certificate = "./server.crt", s::ssl_ciphers = "ALL:!NULL");
}
/*

## Metamaps

If you are new to lithium, please read this to make sure you understand the metamap paradigm:
https://github.com/matt-42/lithium/tree/master/libraries/metamap

## Request parameters
*/
http_api api;
api.get("/get_params") = [&](http_request& request, http_response& response) {
  // This will throw a BAD REQUEST http error if one field is missing or ill-formated.
  // Note: all s::XXXX are symbols, they must be defined, check here for more info:
  // https://github.com/matt-42/lithium/tree/master/libraries/metamap
  auto params = request.get_parameters(s::my_param = int(), s::my_param2 = std::string());
  response.write("hello " + params.my_param2);
};

api.post("/post_params") = [&](http_request& request, http_response& response) {
  // This will throw a BAD REQUEST http error if one field is missing or ill-formated.
  auto params = request.post_parameters(s::my_param = int(), s::my_param2 = std::string());
  response.write("hello " + params.my_param2);
};

api.post("/url_params/{{name}}") = [&](http_request& request, http_response& response) {
  auto params = request.url_parameters(s::name = std::string());
  response.write("hello " + params.name);
};

// You can also pass optional parameters:
auto param = request.get_parameters(s::my_param = std::optional<int>());
// here param.id has type std::optional<int>()
if (param.id)
  std::cout << "optional parameter set." << std::endl;
else
  std::cout << "optional set: " << param.id.value() >> << std::endl;
/*

## Error handling

`li::http_backend` uses exceptions to report HTTP errors.

*/
http_api api;
api.get("/unauthorized") = [&](http_request& request, http_response& response) {
  throw http_error::unauthorized("You cannot access this route.");
};
/*

## Reading and writing headers and cookies

*/

api.get("/unauthorized") = [&](http_request& request, http_response& response) {
  const char* value = request.header("_header_name_");
  const char* value = request.cookie("_cookie_name_");
  // Values are null if the header/cookie does not exists.

  response.set_header("header_name", "header value");
  response.set_cookie("cookie_name", "cookie_value");
};

/*

## SQL Databases

The sqlite, mysql, postgresql single headers library are located here:
https://github.com/matt-42/lithium/tree/master/single_headers

Download the one suited for your project and /*#include/* it in your code.

*/
// Declare a sqlite database.
auto database = sqlite_database("iod_sqlite_test_orm.db");

// Or a mysql database.
auto database = mysql_database(s::host = "127.0.0.1",
                              s::database = "silicon_test",
                              s::user = "root",
                              s::password = "sl_test_password",
                              s::port = 14550,
                              s::charset = "utf8");

api.post("/db_test") = [&] (http_request& request, http_response& response) {
  auto connection = database.connect();
  int count = connection("Select count(*) from users").read<int>();
  std::optional<std::string> name = 
    connection("Select name from users LIMIT 1").read_optional<std::string>();
  if (name) response.write(*name);
  else response.write("empty table");
};

/*

Check https://github.com/matt-42/lithium/tree/master/libraries/sql for more information
on how to use these connections.

## Object Relational Mapping

*/
auto users = li::sql_orm_schema(database, "user_table" /* the table name in the SQL db*/)

              .fields(s::id(s::auto_increment, s::primary_key) = int(),
                      s::age = int(),
                      s::name = std::string(),
                      s::login = std::string());

api.post("/orm_test") = [&] (http_request& request, http_response& response) {
  auto U = users.connect();
  long long int id = U.insert(s::name = "john", s::age = 42, s::login = "doe");
  response.write("new inserted id: ", id, ", new user count is ", U.count());
  
  U.update(s::id = id, s::age = 43);
  assert(U.find_one(s::id = id)->age == 43);
  U.remove(s::id = id);

}
/*
### ORM Callbacks

If you need to insert custom logic in the ORM, you can pass callbacks to the orm:
*/
auto users = li::sql_orm_schema(database, "user_table")
              .fields([...])
              .callbacks(s::before_insert = [] (auto& user) { ... });
/*

Callbacks can also take additional arguments:
*/
auto users = li::sql_orm_schema(database, "user_table")
              .fields([...])
              .callbacks(
                s::before_insert = [] (auto& user, http_request& request) { ... });

// Additional arguments are passed to the ORM methods:
api.post("/orm_test") = [&] (http_request& request, http_response& response) {
  users.connect().insert(s::name = "john", s::age = 42, s::login = "doe", request);
}
/*


More info on the ORM here: https://github.com/matt-42/lithium/tree/master/libraries/sql

## Insert/Update/Remove API

If you are lazy to write update/remove/remove/find_by_id routes for all you objects, you can
use `sql_crud_api` to define them automatically 
*/
api.add_subapi("/user", sql_crud_api(my_orm));
/*
Note that when using it, ORM callbacks must take `http_request& request, http_response& response`
as additional arguments.

Check the code for more info:
https://github.com/matt-42/lithium/blob/master/libraries/http_backend/http_backend/sql_crud_api.hh


## Sessions

*/
// Let's use a session to remember the name of a user.
// By default, we set the username to "unknown"
// Store sessions in a sql db:
auto session = sql_http_session(db, "user_sessions_table", "test_cookie", s::name = "unknown");
// Or in-memory (does not persist after a server shutdown).
auto session = hashmap_http_session("test_cookie", s::name = "unknown");

api.get("/sessions") = [&] (http_request& request, http_response& response) {
  auto sess = session.connect(request, response);

  // Access session values.
  response.write("hello ", sess.values().name);

  // Store values in the session.
  auto get_params = request.get_parameter(s::name = std::string());
  sess.store(s::name = params.name);

  // Remove an existing session.
  sess.logout();
}
/*

## User authentication

*/
auto db = sqlite_database("blog_database.sqlite");
auto users =
    li::sql_orm_schema(db, "auth_users")
        .fields(s::id(s::auto_increment, s::primary_key) = int(), 
                s::email = std::string(),
                s::password = std::string());

// Store user session in sql:
auto sessions = sql_http_session(db, "user_sessions_table", "test_cookie", s::name = "unknown");

// Or in memory:
auto sessions = li::hashmap_http_session("auth_cookie", s::user_id = -1);

//
auto auth = li::http_authentication(// the sessions to store the user id of the logged user.
                                    // It need to have a user_id field.
                                    sessions,
                                    users, // The user ORM
                                    s::email, // The user field used as login.
                                    s::password, // The user field used as password.
                                    // Optional but really recommended: password hashing.
                                    s::hash_password = [&] (auto login, auto password) { 
                                      return your_secure_hash_function(login, password);
                                      },
                                    // Optional: generate secret key to salt your hash function.
                                    s::update_secret_key = [&] (auto login, auto password) {

                                      },
                                    );

li::api<li::http_request, li::http_response> my_api;

my_api.post("/auth_test") = [&] (http_request& request, http_response& response) {
  // Login: 
  //   Read the email/password field in the POST parameters
  //   Check if this match with a user in DB.
  //   Store the user_id in the session.
  //   Return true if the login is successful, false otherwise.
  if (!auth.login(request, response))
      throw http_error::unauthorized("Bad login.");

  // Access the current user.
  auto u = auth.current_user(request, response);
  // u is a std::optional.
  if (!u) throw http_error::unauthorized("Please login.");
  // Logout: destroy the session.
  auth.logout(request, response);
};
/*

Check the code for more info:
https://github.com/matt-42/lithium/blob/master/libraries/http_backend/http_backend/http_authentication.hh


## Testing

Lithium provide a http client and a server non blocking mode so you can easily tests your server responses:

*/
// Use the s::non_blocking parameters so the http_serve will run in a separate thread. 
http_serve(my_api, 12344, s::non_blocking);

// Use li::http_client to test your API.
auto r = http_get("http://localhost:12344/hello_world", s::get_parameters = mmm(s::name = "John")));
assert(r.status == 200);
assert(r.body == "expected response body");
/*

## A complete blog API

Checkout the code here:
https://github.com/matt-42/lithium/blob/master/libraries/http_backend/examples/blog.cc
*/