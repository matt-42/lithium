#include "lithium_http_server.hh"

int main() {

// __documentation_starts_here__
/*
http_server
=================================

## Header

```c++
#include <lithium_http_server.hh>
```
## Introduction

### Motivations

The goal of this library is to take advangage of C++17 new features
to simplify the writing of high performance web server. It provides consise
and simple construcs that allows non C++ experts to quickly implement simple HTTP(s)
servers.

The design of this library is focused on **simplicity**:
- Simple things should be simple to write for the end user.
- Inheritance is rarely used
- The end user should not have to instantiate complex templates
- Function taking many parameters should use named parameters

### Dependencies
  - Boost context and lexical_cast
  - OpenSSL

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

## Serving HTTP / HTTPS

```c++
void http_serve(http_api api, int port, options...)
```

Available options are:
- `s::non_blocking`: do not block until the server stoped. default: blocking.
- `s::threads`: number of threads, default to `std::thread::hardware_concurrency()`.

For HTTPS, you must provide:
- `s::ssl_key`: path of the SSL key.
- `s::ssl_certificate`: path of the SSL certificate.
- `s::ssl_cifers`: OpenSSL cifers.


## Error handling

`li::http_server` uses exceptions to report HTTP errors. 

*/
http_api api;
api.get("/unauthorized") = [&](http_request& request, http_response& response) {
  throw http_error::unauthorized("You cannot access this route.");
};
/*

Available errors are:
- `http_error::bad_request`
- `http_error::unauthorized`
- `http_error::forbidden`
- `http_error::not_found`
- `http_error::internal_server_error`
- `http_error::not_implemented`

All the error builders above take any number of argument and use
`std::ostringstream::operator<<` to convert them into one error string that is
sent as the HTTP response body with the appropriate HTTP error code. 

## GET POST and URL request parameters

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
auto sql_session = sql_http_session(db, "user_sessions_table",
                                    "session_cookie_name", s::name = "unknown");
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
to authenticate users. It requires to instantiate several object first:

  - a SQL database so we know where is the user table (including login and password), and where to store the sessions
  - a user ORM so we know the structure of user table
  - a session object so we can remember a logged user

Here is an code snippet that implement user authentication in a sqlite database:

*/
auto db = sqlite_database("blog_database.sqlite");
auto users =
    li::sql_orm_schema(db, "auth_users")
        .fields(s::id(s::auto_increment, s::primary_key) = int(), 
                s::email = std::string(),
                s::password = std::string());

// Store user session in sql:
auto sessions = sql_http_session(db, "user_sessions_table", 
                                 "test_cookie", s::name = "unknown");

// Authenticator 
auto auth = li::http_authentication(
  // the sessions to store the user id of the logged user.
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
  }
);
/*
Once the authenticator build we can use it in the handlers.

3 methods are available:

### current_user

```c++
auth.current_user(request, response) -> std::optional<user_type>
```

Return the current user object as it is declared by the ORM. In the example above,
we would get an object with 3 members: `id`, `email` and `password`.  

If no user is logged, return an empty std::optional.

### login

```c++
auth.login(request, response) -> bool
```

- Reads the login and password (field names that you have to pass to `http_authentication`) in the request POST parameters.
- Check in the database if the login/password is valid.
- If yes, fill the session `user_id` field with the id of the logged user in the database.
- Return true if we could authenticate the user, false otherwise.

### logout

```c++
auth.logout(request, response) -> void
```

- Destroy the session.
- Logout the logged user if any.

### signup

```c++
auth.signup(request, response) -> void
```

- Fetch the user fields (as defined by the ORM, email and login in the example above) in the request POST parameters.
- Check if no user with the same login already exists.
- If not, insert the new user in the db, call the update_secret_key and hash_password callback and return true.
- Otherwise return false.  

## Serving static files

`serve_directory` serves a directory containing static files:
*/
http_api my_static_file_server_api;
my_static_file_server_api.add_subapi("/root", serve_directory("/path/to/the/directory/to/serve"));

/*
You can also send static files directly from a handler with:
*/
my_static_file_server_api.get("/get_file") = [&] (http_request& request, http_response& response) {
  response.write_static_file("path/to/the/file");
}
/*

Note that `response.write_static_file` and `serve_directory` automatically set the Content-Type header to the mime type.
All mime types of this list are supported: http://svn.apache.org/repos/asf/httpd/httpd/trunk/docs/conf/mime.types

.
## Testing

Using `http_client` and the `s::non_blocking` flag of http_serve
you can easily tests your server responses.
[Read more about http_client here](#http-client)

Here is a quick example:

*/
// Use the s::non_blocking parameters so the http_serve will run in a separate thread. 
http_serve(my_api, 12344, s::non_blocking);

// Use li::http_client to test your API.
auto r = http_get("http://localhost:12344/hello_world", s::get_parameters = mmm(s::name = "John")));
assert(r.status == 200);
assert(r.body == "expected response body");

}
