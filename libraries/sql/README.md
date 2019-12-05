li::sql
================================

This library aims to ease the communication with SQL databases within C++ code.

It features:
  - A SQLite C++ connector
  - A MySQL C++ connector
  - An ORM-like class that allow to send requests without typing raw SQL code

# Tutorial

## Connectors

```c++
#include <li/sql/mysql.hh>
#include <li/sql/sqlite.hh>

// Declare a mysql database.
auto db = li::mysql_database(s::host = "127.0.0.1",
                             s::database = "testdb",
                             s::user = "user",
                             s::password = "pass",
                             s::port = 12345,
                             s::charset = "utf8");

// Declare a sqlite database.
auto db = li::sqlite_database("my_sqlitedb.db");

// Connect to the database.
auto con = db.connect();

// Run simple requests.
con("DROP table if exists users;");

// Retrieve data from simple requests.
int count = con("select count(*) from users;").template read<int>();

// Use placeholder to format your request according to some variables.
auto login = con("select login from users where id = ? and name = ?;")(42, "John")
              .template read_optional<std::string>();
// Note: use read_optional when the request may not return data.
//       it returns a std::optional object that allows you to check it:
if (login)
  std::cout << *login << std::endl;
else  
  std::cout << "user not found" << std::endl;

// Process multiple result rows.
con("select name, age from users;").map([] (std::string name, int age) {
  std::cout << name << ":" << age << std::endl;
});
```

## Object Relational Mapping

```c++
// Let's declare our orm.
auto schema = li::sql_orm_schema(db, "users_orm_test" /* the table name in the SQL db*/)

// The fields of our user object:

              .fields(s::id(s::auto_increment, s::primary_key) = int(),
                      s::age(s::read_only) = int(), // Read only a not included in the update requests.
                      s::name = std::string(),
                      s::login = std::string())

// Callbacks can optionally be set to add logic.
// They take as argument the user object being processed.

              .callbacks(
                // the validate callback is called before insert and update.
                s::validate = [] (auto p) { if (p.age < 0) throw std::runtime_error("invalid age"); },
                // the (before|after)(insert|remove|update) are self explanatory.
                s::before_insert = [] (auto p) { std::cout << "going to insert " << json_encode(p) << std::endl; },
                s::after_insert = [] (auto p) { std::cout << "inserted " << json_encode(p) << std::endl; },
                s::after_remove = [] (auto p) { std::cout << "removed " << json_encode(p) << std::endl;})
                ;

// Connect the orm to a database.
auto users = schema.connect(); // db can be built with li::sqlite_database or li::mysql_database

// Drop the table.
users.drop_table_if_exists();

// Create it.
users.create_table_if_not_exists();

// Count users.
int count = users.count();

// Find one user.
// it returns a std::optional object.
auto u = users.find_one(s::id = 42);
if (u) std::cout << u->name << std::endl;
else  std::cout << "user not foudn" << std::endl;
// Note: you can use any combination of user field:
auto u = users.find_one(s::name = "John", s::age = 42); // look for name == John and age == 42;

// Insert a new user.
// Returns the id of the new object.
long long int john_id = users.insert(s::name = "John", s::age = 42, s::login = "lol");

// Update.
auto u = users.find_one(s::id = john_id);
u->age = 43;
users.update(*u);

// Remove.
users.remove(*u);
```

### Callbacks additional arguments

Callbacks can also take additional arguments, it is used for example in the http_backend library to
access the HTTP session.

```c++
auto users = li::sql_orm_schema(database, "user_table")
              .fields([...])
              .callbacks(
                s::before_insert = [] (auto& user, http_request& request) { ... });

// Additional arguments are passed to the ORM methods:
api.post("/orm_test") = [&] (http_request& request, http_response& response) {
  users.connect().insert(s::name = "john", s::age = 42, s::login = "doe", request);
}
```

# Installation / Supported compilers

Everything explained here: https://github.com/matt-42/lithium#installation

# Authors

Matthieu Garrigues https://github.com/matt-42

# Donate

If you find this project helpful, please consider donating:
https://www.paypal.me/matthieugarrigues
