#pragma once

#include <string>
#include <li/http_backend/response.hh>

namespace li {

auto serve_file(const std::string& root, std::string path, http_response& response) {
  std::string base_path = root;
  if (!base_path.empty() && base_path[base_path.size() - 1] != '/') {
    base_path.push_back('/');
  }

  static char dot = '.', slash = '/';

  // remove first slahs if needed.
  //std::string path(r->url.substr(prefix.string().size(), r->url.size() - prefix.string().size()));
  size_t len = path.size();
  if (!path.empty() && path[0] == slash) {
    path.erase(0, 1);
  }

  if (path.empty()) {
    throw http_error::bad_request("No file path given, directory listing not supported.");
  }

  if (len == 1 && path[0] == dot) {
    throw http_error::bad_request("Invalid URI ", path);
  } else if (len == 2 && path[0] == dot && path[1] == dot) {
    throw http_error::bad_request("Invalid URI ", path);
  } else {
    char prev0 = slash, prev1 = slash;
    for (size_t i = 0; i < len; ++i) {
      if (prev0 == dot && prev1 == dot && path[i] == slash) {
        throw http_error::bad_request("Unsupported URI, ../ is not allowed in the URI");
      }
      prev0 = prev1;
      prev1 = path[i];
    }
  }

  response.write_file(base_path + path);
};

inline auto serve_directory(std::string root) {
  api<http_request, http_response> api;

  api.get("/{{path...}}") = [root](http_request& request, http_response& response) {
    auto path = request.url_parameters(s::path = std::string()).path;
    return serve_file(root, path, response);
  };
  return api;
}

} // namespace li
