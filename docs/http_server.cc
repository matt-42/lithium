#include "lithium_http_server.hh"

int main() {

// __documentation_starts_here__
/*
http_server
=================================

## Introduction

### Motivations

The goal of this library is to take advangage of C++17 new features
to simplify the writing of high performance web server. It provides consise
and simple construcs that allows non C++ experts to quickly implement simple HTTP
servers.

### Dependencies
  - OpenSSL
  - Boost

### Hello World

Write your first hello world server:
```c++
// hello_world.cc
#include "lithium_http_server.hh"

int main() {

  // Define an api with one HTTP GET route.
  li::http_api api;
  api.get("/hello_world") = [&](li::http_request& request, li::http_response& response) {
    response.write("hello world.");
  };

  // Start a http server.
  li::http_serve(api, 12345);

}
```

Run it with the cli:
```bash
$ li run ./hello_world.cc
```
/*

## Error handling

`li::http_server` uses exceptions to report HTTP errors.

*/
http_api api;
api.get("/unauthorized") = [&](http_request& request, http_response& response) {
  throw http_error::unauthorized("You cannot access this route.");
};
/*
## GET / POST / URL request parameters

`request.get_parameters`, `request.post_parameters` and `request.url_parameters` allows to read request parameters.
*/
http_api api;
api.get("/get_params") = [&](http_request& request, http_response& response) {
  // This will throw a BAD REQUEST http error if one field is missing or ill-formated.
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
/*
## Optional request parameters

You can also take optional parameters:
*/
auto param = request.get_parameters(s::my_param = std::optional<int>());
// here param.id has type std::optional<int>()
if (param.id)
  std::cout << "optional parameter set." << std::endl;
else
  std::cout << "optional set: " << param.id.value() >> << std::endl;
/*

## Headers and cookies

Headers and cookies are accessed using `request.header` and `request.cookie`:
*/

api.get("/unauthorized") = [&](http_request& request, http_response& response) {
  const char* value = request.header("_header_name_");
  const char* value = request.cookie("_cookie_name_");
  // Values are nullptr if the header/cookie does not exists.

  response.set_header("header_name", "header value");
  response.set_cookie("cookie_name", "cookie_value");
};

/*
## Nested APIs

APIs can be nested so its trivial to plug one API into a larger one.

*/

http_api root_api;
http_api nested_api;
// Let's assume there is something in these two APIs.
root_api.add_subapi("/prefix_of_the_nested_api", nested_api);

/*

## Sessions

Let's use a session to remember the name of a user.

To store the session in the server memory, use `hashmap_http_session`. In this example,
we'll store the name of the current user. When the session is new, the name will be initialized
to `'unknown'`.
*/
auto session = hashmap_http_session("session_cookie_name", s::name = "unknown");
/*
To store it in a database, use `sql_http_session`:
*/
auto db = sqlite_database("session_database.sqlite");
auto sql_session = sql_http_session(db, "user_sessions_table", "session_cookie_name", s::name = "unknown");
/*

Session can then be accessed in a handler using the `connect` method of the global session object.
It returns an object with the session fields:
*/
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

On top of SQL sessions is built `http_authentication` that allows
to authenticate users. It requires to instantiate several object fisrt
  - a SQL database so we know where is the user table (including login and password), and where to store the sessions
  - a user ORM so we know the structure of user table
  - a session object so we can remember a logged user.
*/
auto db = sqlite_database("blog_database.sqlite");
auto users =
    li::sql_orm_schema(db, "auth_users")
        .fields(s::id(s::auto_increment, s::primary_key) = int(), 
                s::email = std::string(),
                s::password = std::string());

// Store user session in sql:
auto sessions = sql_http_session(db, "user_sessions_table", "test_cookie", s::name = "unknown");

// Authenticator 
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

/*
Once the authenticator build we can use it in the handlers.

3 methods are available:
  - auth.current_user(request, response) -> std::optional<user_type>: 
      Return the current user object as it is stored in the database. If no user is logged, 
      return an empty std::optional.
*/
li::api<li::http_request, li::http_response> my_api;

my_api.post("/auth_test") = [&] (http_request& request, http_response& response) {
  // auth.login: 
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
https://github.com/matt-42/lithium/blob/master/libraries/http_server/http_server/http_authentication.hh


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
https://github.com/matt-42/lithium/blob/master/libraries/http_server/examples/blog.cc
*/

}
