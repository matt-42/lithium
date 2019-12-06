#pragma once

#include <random>
#include <sstream>
#include <li/http_backend/response.hh>
#include <li/http_backend/request.hh>

namespace li {

std::string generate_secret_tracking_id() {
  std::ostringstream os;
  std::random_device rd;
  os << std::hex << rd() << rd() << rd() << rd();
  return os.str();
}

inline std::string random_cookie(http_request& request, http_response& response,
                                 const char* key = "silicon_token") {
  std::string token;
  const char* token_ = request.cookie(key);
  if (!token_) {
    token = generate_secret_tracking_id();
    response.set_cookie(key, token);
  } else {
    token = token_;
  }

  return token;
}

} // namespace li
