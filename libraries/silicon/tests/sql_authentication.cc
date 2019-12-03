#include <iod/http_client/http_client.hh>
#include <iod/silicon/silicon.hh>
#include <iod/silicon/sql_http_session.hh>

#include "symbols.hh"
#include "test.hh"

using namespace iod;

int main() {

  auto test_with_db = [] (auto& db) {
    api<http_request, http_response> my_api;

    // Users table
    auto user_orm = sql_orm_schema("users_to_test_session").fields(s::id(s::auto_increment, s::primary_key) = int(), s::login = std::string(),
                              s::password = std::string());
    user_orm.connect(db).drop_table_if_exists().create_table_if_not_exists();

    // Session.
    sql_http_session session("user_sessions", s::user_id = -1);
    session.orm().connect(db).drop_table_if_exists().create_table_if_not_exists();

    my_api.get("/who_am_i") = [&](http_request& request, http_response& response) {
      auto sess = session.connect(db, request, response);
      std::cout << sess.values().user_id << std::endl;
      if (sess.values().user_id != -1)
      {
        bool found;
        auto u = user_orm.connect(db).find_one(found, s::id = sess.values().user_id);
        response.write(u.login);
      }
      else
        throw http_error::unauthorized("Please login.");   
    };

    my_api.post("/login") = [&](http_request& request, http_response& response) {
      auto lp = request.post_parameters(s::login = std::string(), s::password = std::string());
      bool exists = false;
      auto user = user_orm.connect(db).find_one(exists, lp);
      if (exists)
      {
        session.connect(db, request, response).store(s::user_id = user.id);
        response.write("login success");
      }
      else throw http_error::unauthorized("Bad login or password.");
    };

    my_api.post("/signup") = [&](http_request& request, http_response& response) {
      auto new_user = request.post_parameters(s::login = std::string(), s::password = std::string());
      bool exists = false;
      auto user_db = user_orm.connect(db);
      auto user = user_db.find_one(exists, s::login = new_user.login);
      std::cout << user_db.count() << std::endl;
      if (! exists)
        user_db.insert(new_user);
      else throw http_error::bad_request("User already exists.");
    };

    my_api.get("/logout") = [&](http_request& request, http_response& response) {
        session.connect(db, request, response).logout();
    };


  
    auto ctx = http_serve(my_api, 12350, s::non_blocking);
    auto c = http_client("http://localhost:12350");

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

    CHECK_EQUAL("valid login", session.orm().connect(db).count(), 1);

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
    //assert(http_get("http://localhost:12345/hello_world").body == "hello world.");
  };


  auto sqlite_db = sqlite_database("iod_sqlite_orm_test.db");
  test_with_db(sqlite_db);

  auto mysql_db = mysql_connection_pool(s::host = "127.0.0.1",
                                s::database = "silicon_test",
                                s::user = "root",
                                s::password = "sl_test_password",
                                s::port = 14550,
                                s::charset = "utf8");
  test_with_db(mysql_db);

}
