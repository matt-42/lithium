#include <lithium.hh>


#include "symbols.hh"

using namespace li;

// The sqlite database that we use to store the users.
auto db = sqlite_database("blog_database.sqlite");

// We declare the users ORM.
//   Per user, we need a unique id, login and a password.
auto users = sql_orm_schema(db, "blog_users")
                  .fields(s::id(s::auto_increment, s::primary_key) = int(),
                          s::login(s::primary_key) = std::string(), s::password = std::string(),
                          s::email = std::string());

// The session will be place in an in memory hashmap.
// They store the user_id of the logged used.
auto sessions = hashmap_http_session("test_cookie", s::user_id = -1);

// This authentication module implements the login, logout, signup and current_user logic.
// We specify here that we use the login and password field of the user table
// to authenticate the users.
// This means that the client will have to pass a login and password POST parameters
// to the login HTTP route.
// For the signup, the module will ask for every fields (login, password) except the auto
// increment field (i.e. the id field).
auto auth = http_authentication(sessions, users, s::login, s::password);

// The posts ORM will handle the blog_posts table in the sqlite database.
// We use the ORM callbacks to check if needed the user priviledges or the validity of post.
//
auto posts = sql_orm_schema(db, "blog_posts")
                  .fields(s::id(s::auto_increment, s::primary_key) = int(),
                          // We mark the user_id as computed so the CRUD api does not
                          // require it in the create and update methods.
                          s::user_id(s::computed) = int(), s::title = std::string(),
                          s::body = std::string())

                  .callbacks(
                      // This callback is called before insertion of a new post
                      // and before the update of an existing post. We use it to reject a request
                      // whenever the client submit invalid data.
                      s::validate =
                          [](auto post, http_request&, http_response&) {
                            if (post.title.size() > 0 and post.body.size() > 0)
                              throw http_error::bad_request("Cannot post an empty post.");
                          },

                      // Only logged users are allowed to post. We check it everytime someone tries
                      // to create a post.
                      // We also use this callback to set the user_id of the new post before the
                      // ORM saves it in the database.
                      s::before_insert =
                          [](auto& post, http_request& req, http_response& resp) {
                            auto u = auth.current_user(req, resp);
                            if (!u)
                              throw http_error::unauthorized("Please login to post.");
                            post.user_id = u->id;
                          },

                      // This checks that a given use has the right to edit the post.
                      s::before_update =
                          [](auto& post, http_request& req, http_response& resp) {
                            auto u = auth.current_user(req, resp);
                            if (!u || u->id != post.user_id)
                              throw http_error::unauthorized("This post does not belong to you.");
                          });

http_api api1() {

  http_api my_api;

  // Generate /auth/[login|logout|signup]
  my_api.add_subapi("/auth", http_authentication_api(auth));
  // Generate /post/[create|update|remove]
  my_api.add_subapi("/post", sql_crud_api(posts));

  return my_api;
}


li::http_api api2();

int main() {

  li::http_serve(api1(), 12345);
  li::http_serve(api2(), 12345);
}
