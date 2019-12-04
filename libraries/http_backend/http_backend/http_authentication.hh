#pragma once

#include <optional>
#include <li/http_backend/sql_http_session.hh>
#include <li/http_backend/api.hh>

namespace li {

template <typename S, typename U, typename... F>
struct http_authentication {
  http_authentication(S& session, U& users, F... login_fields)
      : sessions_(session),
        users_(users),
        login_fields_(std::make_tuple(login_fields...))
        {

        }

 bool login(http_request& req, http_response& resp) {
    auto fields_mm = tuple_reduce(login_fields_, mmm);
    auto lp = req.post_parameters(intersection(users_.all_fields(), fields_mm));
    if (auto user = users_.connect().find_one(lp)) {
      sessions_.connect(req, resp).store(s::user_id = user->id);
      return true;
    } else
      return false;
  }

  auto current_user(http_request& req, http_response& resp) {
    auto sess = sessions_.connect(req, resp);
    if (sess.values().user_id != -1)
      return users_.connect().find_one(s::id = sess.values().user_id);
    else
      return decltype(users_.connect().find_one(s::id = sess.values().user_id)){};
  }

  void logout(http_request& req, http_response& resp) {
    sessions_.connect(req, resp).logout();
  }

  bool signup(http_request& req, http_response& resp) {
      auto new_user = req.post_parameters(users_.without_auto_increment());
      auto users = users_.connect();
      auto fields_mm = tuple_reduce(login_fields_, mmm);
      if (users.exists(intersection(new_user, fields_mm)))
        return false;
      else 
      {
        users.insert(new_user);
        return true;
      }
  }
  S& sessions_;
  U& users_;
  std::tuple<F...> login_fields_;
};

template <typename... A>
api<http_request, http_response> http_authentication_api(http_authentication<A...>& auth) {

  api<http_request, http_response> api;

  api.post("/login") = [&] (http_request& request, http_response& response) {
    if (!auth.login(request, response))
      throw http_error::unauthorized("Bad login.");
  };

  api.get("/logout") = [&] (http_request& request, http_response& response) {
    auth.logout(request, response);
  };

  api.get("/signup") = [&] (http_request& request, http_response& response) {
    if (!auth.signup(request, response))
      throw http_error::bad_request("User already exists.");
  };


  return api;
}

} // namespace li
