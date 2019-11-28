#include <iod/http_client/http_client.hh>
#include <iod/silicon/silicon.hh>

using namespace iod;

int main() {

  api<http_request, http_response> my_api;

  // Database
  sqlite_database db("./test_sqlite_authentication.sqlite");
  // Users table
  sql_orm_schema user_orm("users").fields(s::id(s::autoset) = int(), s::login = std::string(),
                            s::password = std::string());
  // Session.
  sql_session session(s::user_id = int());

  my_api(GET, "/who_am_i") = [&](http_request& request, http_response& response) {
    auto sess = session.get(db, request);
    if (sess.logged_in())
      response.write(json_encode(user_orm.connect(db).find_one(s::id = sess.id)));
    else
      throw http_error::unauthorized("Please login.")    
  };

  my_api(POST, "/login") = [&](http_request& request, http_response& response) {
    auto lp = r.post_parameters(s::login = std::string(), s::password = std::string());
    bool exists = false;
    auto user = user_orm.connect(db).find_one(exists, lp);
    if (exists)
    {
      session.connect(db, request).login(s::user_id = user.id);
      return true;
    }
    else throw http_error::unauthorized("Bad login or password.");
  };

  my_api(POST, "/register") = [&](http_request& request, http_response& response) {
    auto new_user = r.post_parameters(s::login = std::string(), s::password = std::string());
    bool exists = false;
    auto user_db = user_orm.connect(db)
    auto user = user_db.find_one(exists, s::login == new_user.login);
    if (not exists)
      user_db.insert(new_user);
    else throw http_error::bad_request("User already exists.");
  };

  my_api(GET, "/logout") = [&](http_request& request, http_response& response) {
      session.connect(db, request).logout();
  };


  auto ctx = http_serve(my_api, 12345, s::non_blocking);

  assert(http_get("http://localhost:12345/hello_world").body == "hello world.");
}
