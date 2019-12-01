#include <iod/http_client/http_client.hh>
#include <iod/silicon/silicon.hh>
#include <iod/silicon/sql_http_session.hh>

#include "symbols.hh"

using namespace iod;

int main() {

  api<http_request, http_response> my_api;

  // Database
  sqlite_database db("./test_sqlite_authentication.sqlite");
  // Users table
  auto user_orm = sql_orm_schema("users").fields(s::id(s::autoset, s::primary_key) = int(), s::login = std::string(),
                            s::password = std::string());
  // Session.
  sql_http_session session("user_sessions", s::user_id = -1);

  my_api.get("/who_am_i") = [&](http_request& request, http_response& response) {
    auto sess = session.connect(db, request, response);
    if (sess.values().user_id != -1)
    {
      bool found;
      auto u = user_orm.connect(db).find_one(found, s::id = sess.values().user_id);
      response.write(json_encode(u));
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
      return true;
    }
    else throw http_error::unauthorized("Bad login or password.");
  };

  my_api.post("/signup") = [&](http_request& request, http_response& response) {
    auto new_user = request.post_parameters(s::login = std::string(), s::password = std::string());
    bool exists = false;
    auto user_db = user_orm.connect(db);
    auto user = user_db.find_one(exists, s::login = new_user.login);
    if (not exists)
      user_db.insert(new_user);
    else throw http_error::bad_request("User already exists.");
  };

  my_api.get("/logout") = [&](http_request& request, http_response& response) {
      session.connect(db, request, response).logout();
  };


 
  auto ctx = http_serve(my_api, 12350, s::non_blocking);
  auto c = http_client("http://localhost:12350");

  // bad user -> unauthorized.
  auto r1 = c.post("/login", s::post_parameters = make_metamap(s::login = "x", s::password = "x"));
  assert(r1.status == 401);
  assert(r1.body == "Bad login or password.");

  // bad request.
  auto r2 = c.post("/signup", s::post_parameters = make_metamap(s::login = "x"));
  assert(r1.status == 401);
  assert(r1.body == "Bad login or password.");

  //assert(http_get("http://localhost:12345/hello_world").body == "hello world.");
}
