// Visit https://matt-42.github.io/lithium to better visualize this documentation.

#include <lithium_http_server.hh>
#include <lithium_mysql.hh>
#include <lithium_sqlite.hh>
#include <lithium_pgsql.hh>
#include "symbols.hh"

using namespace li;
int main() {


// __documentation_starts_here__

/*
sql
================================

## Headers

```c++
// MySQL header.
#include <lithium_mysql.hh>
// PostgreSQL header.
#include <lithium_pgsql.hh>
// SQLite3 header.
#include <lithium_sqlite.hh>
```

## Introduction
``li::sql`` is a set of high performance and easy to use C++ SQL drivers built on top of the C
drivers provided by the databases. It allows you to target MySQL, PostgreSQL and SQLite under the
same C++ API which is significantly smaller and easier to learn than the C APIs provided by the
databases.

### Features
  - MySQL sync & async requests
  - A PostgreSQL sync & async requests
  - A SQLite sync requests
  - An ORM-like class that allow to send requests without typing raw SQL code.
  - Thread-safe connection pooling for MySQL and PostgreSQL.

### Dependencies
  - boost::lexical_cast
  - MariaDB mysqlClient. Note: MariaDB mysqlclient can target MySQL (non-mariadb) server.
  - PostgreSQL libpq
  - SQLite3


### Runtime Performances

All the implemented abstractions are unwrapped at compile time so it adds almost zero overhead over the
raw C drivers of the databases. According to the [Techempower
benchmark](http://tfb-status.techempower.com/), Lithium MySQL and PostgreSQL drivers are part of the
fastest available.

### Overview

The API contains only 7 operators (excluding the ORM):
  - Build a database: `mysql_database`, `sqlite_database`, `pgsql_database`, 
  - Get a connection from the pool: `database::connect -> connection`
  - Execute queries: `connection::operator() -> result`
  - Build prepared statements: `connection::prepare`
  - Execute prepared statements: `prepared_statement::operator() -> result`
  - Read one query result row: `result::read`
  - Read one result row that may not exist: `result::read_optional`
  - Map function to each result row: `result::map` 

## Database creation

### MySQL

*/
// Declare a mysql database.
mysql_database mysql_db = li::mysql_database(
    s::host = "127.0.0.1", // Hostname or ip of the database server
    s::database = "testdb",  // Database name
    s::user = "user", // Username
    s::password = "pass", // Password
    s::port = 12345, // Port
    s::charset = "utf8", // Charset
    // Only for async connection, specify the maximum number of SQL connections per thread.
    s::max_async_connections_per_thread = 200,
    // Only for synchronous connection, specify the maximum number of SQL connections
    s::max_sync_connections = 2000);

/*
### PostgreSQL
*/
pgsql_database pgsql_db = li::pgsql_database(
    s::host = "127.0.0.1", // Hostname or ip of the database server
    s::database = "testdb",  // Database name
    s::user = "user", // Username
    s::password = "pass", // Password
    s::port = 12345, // Port
    s::charset = "utf8", // Charset
    // Only for async connection, specify the maximum number of SQL connections per thread.
    s::max_async_connections_per_thread = 200,
    // Only for synchronous connection, specify the maximum number of SQL connections
    s::max_sync_connections = 2000);
/*
### SQLite

*/

sqlite_database sqlite_db = li::sqlite_database(
  "my_sqlitedb.db" // Path to the sqlite database file.
);

/*
## Connect

```c++
database::connect() -> connection
```

Requested a database connection. One connection of the internally
managed connection pool is returned, otherwise a new connection is created. 

Connections automatically returns to the pool at the end of their scope (thanks to RAII).

## Connect (asynchronous mode)

Support: MySQL and PostgreSQL

Asynchronous application based on epoll (Linux) or kqueue (MacOS) can take advantage
of asynchronous connections. The only requirement is that fibers must be identified by an integer and
that you wrap you fibers (or coroutine, light thread, green thread, ...) with the following interface.

*/

struct fiber_wrapper {
  typedef std::runtime_error exception_type;
  int fiber_id = 0;
  // A sleeping fiber need to be woken up.
  // Plan to wake it up.
  inline void defer_fiber_resume(int fiber_id) {}
  // Make this fiber wakeup on IO event on the file descriptor \fd.
  // fd was assigned to another file descriptor.
  inline void reassign_fd_to_this_fiber(int fd) {}

  // epoll/kqueue wrapper for the current fiber.
  // Make this fiber wakeup on IO event filtered by the filtering flags \flags
  // of the file descriptor \fd.
  inline void epoll_add(int fd, int flags) {}
  // epoll/kqueue wrapper
  // modify the filtering flags of the file descriptor \fd
  inline void epoll_mod(int fd, int flags) {}

  // yield this fiber.
  inline void yield() {}
};

/*
Once the wrapper defined, pass an instance of your wrapper to `database::connect` to get an asynchronous SQL connection:
*/
  fiber_wrapper w{1};
  auto connection = pgsql_db.connect(w);
/*

## SQL Requests

`connection::operator(std::string request)` send SQL requests to the database;
*/
auto result = connection("SELECT 1+2");
/*

## Prepared Statements

Build SQL prepared statement:
```c++
connection::prepare(std::string request) -> prepared_statement
```
Execute a prepared statement:
```c++
prepared_statement::operator(T... arguments) -> result
```

Example:
*/
auto prepared_statement = connection.prepare("SELECT id from users where id=?");
auto result_ps = prepared_statement(42);
/*

## Reading results

The 3 methods of the result object allow to read result of a SQL query: `read`, `read_optional` and `map`


### Read

Read one row of the current query result.

For rows containing only one column:
```c++
result::read<T>() -> T
result::read(T& out_values...) -> void
```

For rows containing more than one columns:
```c++
result::read<T1, T...>() -> std::tuple<T1, T...>
```

The `read` method reads the current row and advances to the next row.
It throws an exception:
  - if the result set is empty or totally consumed.
  - if there is a mismatch between the types `T...` and the columns of the SQL query.

Example:

*/
int count = connection("select count(*) from users;").read<int>();
auto [name, age] = connection("select name, age from users where id = 42;").read<std::string, int>();
/*
### Optional read

`read_optional` behave excatly like `read` except that:
- It does not throw an exception when the result set is empty or totally consumed.
- It wraps the result in `std::optional` 

```c++
result::read_optional<T>() -> std::optional<T>
result::read_optional<T1, T...>() -> std::optional<std::tuple<T1, T...>>
```

*/
auto optional =
    connection("select name, age from users where id = 42;").read_optional<std::string, int>();
if (optional) {
  auto [name, age] = optional.value();
  // [...]
}
/*

### Map

```c++
result::map(F fun) -> void
``` 

Maps a function \fun to each row of a result set. 
Template functions (or lambda with auto arguments) are not allowed.

Throw if there is a mismatch between the argument types and the columns of the SQL query.

Example:
*/

connection("select name, age from users;").map([](std::string name, int age) {
  std::cout << name << ":" << age << std::endl;
});
/*

### Writting results to Metamaps

All the read methods above can also take metamaps as argument:

*/
connection("select name, age from users;").map(
  [](const metamap_t<s::name_t, int, s::age_t, std::string>& row) {
    std::cout << row.name << ":" << row.age << std::endl;
  }
);

/*

## Object Relational Mapping

On top of the raw query API, an ORM allows to automatically generate 
insertion/update/removal requests. It does not generate `JOIN` requets so automatically fetching 
object foreign key is not possible.


*/
// Let's declare our orm.
auto schema =
    sql_orm_schema(pgsql_db, "users_orm_test" /* the table name in the SQL db*/)

        // The fields of our user object:

        .fields(s::id(s::auto_increment, s::primary_key) = int(),
                s::age(s::read_only) = int(), // Read only a not included in the update requests.
                s::name = std::string(), s::login = std::string())

        // Callbacks can optionally be set to add logic.
        // They take as argument the user object being processed.

        .callbacks(
            // the validate callback is called before insert and update.
            s::validate =
                [](auto p) {
                  if (p.age < 0)
                    throw std::runtime_error("invalid age");
                },
            // the (before|after)(insert|remove|update) are self explanatory.
            s::before_insert =
                [](auto p) { std::cout << "going to insert " << json_encode(p) << std::endl; },
            s::after_insert =
                [](auto p) { std::cout << "inserted " << json_encode(p) << std::endl; },
            s::after_remove =
                [](auto p) { std::cout << "removed " << json_encode(p) << std::endl; });

// Connect the orm to a database.
auto users = schema.connect(); // db can be built with li::sqlite_database or li::mysql_database

// Drop the table.
users.drop_table_if_exists();

// Create it.
users.create_table_if_not_exists();

// Count users.
count = users.count();

// Find one user.
// it returns a std::optional object.
auto u42 = users.find_one(s::id = 42);
if (u42)
  std::cout << u42->name << std::endl;
else
  std::cout << "user not found" << std::endl;
// Note: you can use any combination of user field:
auto john = users.find_one(s::name = "John", s::age = 42); // look for name == John and age == 42;

// Insert a new user.
// Returns the id of the new object.
long long int new_john_id = users.insert(s::name = "John", s::age = 42, s::login = "john_d");

// Update.
auto to_update = users.find_one(s::id = new_john_id);
to_update->age = 43;
users.update(*to_update);

// Remove.
users.remove(*to_update);
/*

### ORM Callbacks additional arguments

Callbacks can also take additional arguments, it is used for example in the http_server library
to access the HTTP session.

*/
auto users2 =
    sql_orm_schema(pgsql_db, "user_table")
        .fields(s::id(s::auto_increment, s::primary_key) = int(),
                s::age(s::read_only) = int(), // Read only a not included in the update requests.
                s::name = std::string(), s::login = std::string())
        .callbacks(s::before_insert = [](auto& user, li::http_request& request) {});

// Additional arguments are passed to the ORM methods:
li::http_api api;
api.post("/orm_test") = [&](li::http_request& request, li::http_response& response) {
  users2.connect().insert(s::name = "john", s::age = 42, s::login = "doe", request);
};
}
