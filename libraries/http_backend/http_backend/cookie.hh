#pragma once

#include <sstream>
#include <random>

namespace li {

  std::string generate_secret_tracking_id()
  {
    std::ostringstream os;
    std::random_device rd;
    os << std::hex << rd() << rd() << rd() << rd();
    return os.str();
  }
  
  inline std::string cookie(http_request& request,
				    http_response& response,
				    const char* key = "silicon_token")
  {
    std::string token;
    const char* token_ = request.cookie(key);
    if (!token_)
      {
        token = generate_secret_tracking_id();
        response.set_cookie(key, token);
      }
    else
      {
        token = token_;
      }

    return token;
  }

}
