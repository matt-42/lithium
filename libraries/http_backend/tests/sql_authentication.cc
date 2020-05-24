#include <li/http_backend/http_backend.hh>
#include <li/http_client/http_client.hh>
#include <li/sql/mysql.hh>
#include <li/sql/sqlite.hh>
#include <li/sql/pgsql.hh>

#include "symbols.hh"
#include "test.hh"

using namespace li;

int main() {

  auto test_with_db = [](auto& db, int port) {
    http_api my_api;

    // Users table
    auto user_orm = sql_orm_schema<decltype(db)>(db, "users_to_test_session")
                        .fields(s::id(s::auto_increment, s::primary_key) = int(),
                                s::login = std::string(), s::password = std::string());
    user_orm.connect().drop_table_if_exists().create_table_if_not_exists();

    // Session.
    auto session = sql_http_session(db, "user_sessions", "test_cookie", s::user_id = -1);
    session.orm().connect().drop_table_if_exists().create_table_if_not_exists();

    my_api.get("/who_am_i") = [&](http_request& request, http_response& response) {
      auto sess = session.connect(request, response);
      std::cout << sess.values().user_id << std::endl;
      if (sess.values().user_id != -1) {
        if (auto u = user_orm.connect().find_one(s::id = sess.values().user_id))
          response.write(u->login);
        else
          throw http_error::internal_server_error("Id in session does not match database...");
      } else
        throw http_error::unauthorized("Please login.");
    };

    my_api.post("/login") = [&](http_request& request, http_response& response) {
      auto lp = request.post_parameters(s::login = std::string(), s::password = std::string());
      if (auto user = user_orm.connect().find_one(lp)) {
        session.connect(request, response).store(s::user_id = user->id);
        response.write("login success");
      } else
        throw http_error::unauthorized("Bad login or password.");
    };

    my_api.post("/signup") = [&](http_request& request, http_response& response) {
      auto new_user =
          request.post_parameters(s::login = std::string(), s::password = std::string());
      auto user_db = user_orm.connect();
      if (user_db.exists(s::login = new_user.login))
        throw http_error::bad_request("User already exists.");
      else
        user_db.insert(new_user);
    };

    my_api.get("/logout") = [&](http_request& request, http_response& response) {
      session.connect(request, response).logout();
    };

    http_serve(my_api, port, s::non_blocking);
    auto c = http_client("http://localhost:" + boost::lexical_cast<std::string>(port));

    // bad user -> unauthorized.
    auto r1 = c.post("/login", s::post_parameters = mmm(s::login = "x", s::password = "x"));
    std::cout << json_encode(r1) << std::endl;
    CHECK_EQUAL("unknown user", r1.status, 401);
    assert(r1.body == "Bad login or password.");

    // valid signup.
    auto r2 = c.post("/signup", s::post_parameters = mmm(s::login = "john", s::password = "abcd"));
    std::cout << json_encode(r2) << std::endl;
    CHECK_EQUAL("signup", r2.status, 200);

    // Bad password
    auto r3 = c.post("/login", s::post_parameters = mmm(s::login = "john", s::password = "abcd2"));
    std::cout << json_encode(r3) << std::endl;
    CHECK_EQUAL("bad password", r3.status, 401);
    assert(r1.body == "Bad login or password.");

    // Valid login
    auto r4 = c.post("/login", s::post_parameters = mmm(s::login = "john", s::password = "abcd"));
    std::cout << json_encode(r4) << std::endl;
    CHECK_EQUAL("valid login", r4.status, 200);
    CHECK_EQUAL("valid login", r4.body, "login success");

    // Remove this check, postgresql need time to update the table. Don't know why.
    CHECK_EQUAL("valid login", session.orm().connect().count(), 1);

    // Check session.
    auto r5 = c.get("/who_am_i");
    std::cout << json_encode(r5) << std::endl;
    CHECK_EQUAL("read session", r5.body, "john");
    CHECK_EQUAL("read session", r5.status, 200);


    // Logout
    c.get("/logout");
    auto r6 = c.get("/who_am_i");
    std::cout << json_encode(r6) << std::endl;
    CHECK_EQUAL("read session", r6.status, 401);
    CHECK_EQUAL("read session", r6.body, "Please login.");
    // assert(http_get("http://localhost:12345/hello_world").body == "hello world.");
  };

  auto sqlite_db = sqlite_database("iod_sqlite_orm_test.db");
  test_with_db(sqlite_db, 12258);

  auto mysql_db =
      mysql_database(s::host = "127.0.0.1", s::database = "mysql_test", s::user = "root",
                     s::password = "lithium_test", s::port = 14550, s::charset = "utf8");
  test_with_db(mysql_db, 12262);

  auto pgsql_db = pgsql_database(s::host = "localhost", s::database = "postgres", s::user = "postgres",
                            s::password = "lithium_test", s::port = 32768, s::charset = "utf8");
  test_with_db(pgsql_db, 12261);
}
