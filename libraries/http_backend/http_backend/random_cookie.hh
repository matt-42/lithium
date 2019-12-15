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
  std::string_view token_ = request.cookie(key);
  if (!token_.data()) {
    token = generate_secret_tracking_id();
    response.set_cookie(key, token);
  } else {
    std::cout << "got token " << token_ << std::endl;
    token = token_;
  }

  return token;
}

} // namespace li
