li::sql
================================

``li::sql`` is a set of high performance and easy to use C++ SQL drivers built on top of the C drivers provided by the databases. It allows you to target MySQL, PostgreSQL and SQLite under the same C++ API which
is significantly smaller and easier to learn than the C APIs provided by the databases.

It features:
  - MySQL sync & async requests
  - A PostgreSQL sync & async requests
  - A SQLite sync requests
  - An ORM-like class that allow to send requests without typing raw SQL code.
  - Thread-safe connection pooling for MySQL and PostgreSQL.

All the three connectors are following the same API so you can use the same way
a SQLite, MySQL or PostgreSQL database.

# Runtime Performances

All ``li::sql`` abstractions are unwrapped at compile time so it adds almost zero overhead over the raw C drivers of the databases.
According to the [Techempower benchmark](http://tfb-status.techempower.com/), Lithium MySQL and PostgreSQL drivers are part of the fastest available.

# Ease of use

While building a fast DB driver, we did not do any compromise on the ease of use of the library.
The learning curve of this library is inferior to 5 minutes since it's API has only 5 functions (excluding the ORM):
  - `database::connect` to get a connection
  - `connection::operator()` to execute queries
  - `connection::prepare` to build prepared statements
  - `prepared_statement::operator()` to execute a prepared statement
  - `result::read` to read result
  - `result::read_optional` to read result that may be null
  - `result::map` to map function on result rows

# Sync and Async Database Connections with Pooling

```c++
// Declare a mysql database.
auto db = li::mysql_database(s::host = "127.0.0.1",
                             s::database = "testdb",
                             s::user = "user",
                             s::password = "pass",
                             s::port = 12345,
                             s::charset = "utf8",
                             // Only for async connection, specify the maximum number of SQL connections per thread.
                             s::max_async_connections_per_thread = 200,
                             // Only for synchronous connection, specify the maximum number of SQL connections
                             s::max_sync_connections = 2000
                             );

// OR a sqlite database.
auto db = li::sqlite_database("my_sqlitedb.db");

// OR a postgresql database.
auto db = li::pgsql_database(s::host = "127.0.0.1",
                             s::database = "testdb",
                             s::user = "user",
                             s::password = "pass",
                             s::port = 12345,
                             s::charset = "utf8",
                             // Only for async connection, specify the maximum number of SQL connections per thread.
                             s::max_async_connections_per_thread = 200,
                             // Only for synchronous connection, specify the maximum number of SQL connections
                             s::max_sync_connections = 2000
                             );

// Connect to the database: SYNCHRONOUS MODE.
// All function call are blocking.
auto connection = db.connect();
// Throws if an error occured during the connection.

// For MySQL and Postgresql, the db object manages a thread-safe pool of connections. connect() will
// reuse any previously open connection if it is not currently used.
// For SQLite, one thread-safe connection open with the flag SQLITE_OPEN_FULLMUTEX is shared.

// Every connection will be automatically put back in the pool at the end of the scope.

// Connect to the database: ASYNCHRONOUS MODE.
// You must provide as argument an object that follow this API:
// struct active_yield {
//   typedef std::runtime_error exception_type;
//   void epoll_add(int, int) {} wrapper to your epoll_add. For more info: man epoll
//   void epoll_mod(int, int) {} wrapper to your epoll_mod. For more info: man epoll
//   void yield() {}  Yield the current fiber
// };
auto con = db.connect(your_async_fiber_wrapper);

```

# Running Raw Requests and Prepared Statements


```c++
// This was the only difference between using the async and the synchronous
// connector. All the rest of the API is identical.

// Run simple requests.
con("DROP table if exists users;");

// Map a function on all result rows.
con("select name, age from users;").map([] (std::string name, int age) {
  std::cout << name << ":" << age << std::endl;
});

// Retrieve data from simple requests.
int count = con("select count(*) from users;").read<int>();
auto [name, age] = con("select name, age from users where id = 42;").read<std::string, int>();
// following calls to read on the same result will read the following result rows.
// or throw if no more result row are available.

// Retrive data that may be null (do not throw is no data available).
auto optional = con("select name, age from users where id = 42;").read_optional<std::string, int>();
// typeof(optional) == std::optional<std::tuple<std::string, int>>
if (optional)
{
  auto [name, age] = optional.value();
  [...]
}


// Read can also take arguments by reference to avoid copies.
std::string name;
int age;
con("select name, age from users where id = 42;").read(name, age);

std::optional<std::tuple<std::string, int>> optional;
auto optional = con("select name, age from users where id = 42;").read(optional);


// To build prepared statement: 
auto prepared_stmt = con.prepare("select login from users where id = ? and name = ?;");

// Prepared statement follow the same read/read_optional/map API:
auto login = prepared_stmt(32, "john").read<std::string>();
```

## Object Relational Mapping

On top of the raw query API, `li::sql` provides an ORM implementation that ease the insertion/update/removal
of objects in a database.

This heavily relies on the metamap paradigm, please first check https://github.com/matt-42/lithium/tree/master/libraries/metamap for more information about the symbols (`s::XXXX`) and how to define them.

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
else  std::cout << "user not found" << std::endl;
// Note: you can use any combination of user field:
auto u = users.find_one(s::name = "John", s::age = 42); // look for name == John and age == 42;

// Insert a new user.
// Returns the id of the new object.
long long int john_id = users.insert(s::name = "John", s::age = 42, s::login = "john_d");

// Update.
auto u = users.find_one(s::id = john_id);
u->age = 43;
users.update(*u);

// Remove.
users.remove(*u);
```

### ORM Callbacks additional arguments

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