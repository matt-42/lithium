#pragma once

#include <li/http_backend/cookie.hh>
#include <mutex>

namespace li {

template <typename O> struct connected_hashmap_http_session {

  connected_hashmap_http_session(std::string session_id_, O* values_,
                                 std::unordered_map<std::string, O>& sessions_, std::mutex& mutex_)
      : session_id_(session_id_), values_(values_), sessions_(sessions_), mutex_(mutex_) {}

  // Store fiels into the session
  template <typename... F> auto store(F... fields) {
    map(mmm(fields...), [this](auto k, auto v) { values()[k] = v; });
  }

  // Access values of the session.
  O& values() {
    if (!values_)
      throw http_error::internal_server_error("Cannot access session values after logout.");
    return *values_;
  }

  // Delete the session from the database.
  void logout() {
    std::lock_guard<std::mutex> guard(mutex_);
    sessions_.erase(session_id_);
  }

private:
  std::string session_id_;
  O* values_;
  std::unordered_map<std::string, O>& sessions_;
  std::mutex& mutex_;
};

template <typename... F> struct hashmap_http_session {

  typedef decltype(mmm(std::declval<F>()...)) session_values_type;
  inline hashmap_http_session(std::string cookie_name, F... fields)
      : cookie_name_(cookie_name), default_values_(mmm(s::session_id = std::string(), fields...)) {}

  inline auto connect(http_request& request, http_response& response) {
    std::string session_id = session_cookie(request, response, cookie_name_.c_str());
    auto it = sessions_.try_emplace(session_id, default_values_).first;
    return connected_hashmap_http_session<session_values_type>{session_id, &it->second, sessions_,
                                                               sessions_mutex_};
  }

  std::string cookie_name_;
  session_values_type default_values_;
  std::unordered_map<std::string, session_values_type> sessions_;
  std::mutex sessions_mutex_;
};

} // namespace li
