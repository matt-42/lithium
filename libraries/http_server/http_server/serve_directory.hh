#pragma once

#include <stdlib.h>

#include <string>
#include <memory>

#include <li/http_server/response.hh>

namespace li {

namespace impl {
  inline bool is_regular_file(const std::string& path) {
    struct stat path_stat;
    if (-1 == stat(path.c_str(), &path_stat))
      return false;
    return S_ISREG(path_stat.st_mode);
  }

  inline bool is_directory(const std::string& path) {
    struct stat path_stat;
    if (-1 == stat(path.c_str(), &path_stat))
      return false;
    return S_ISDIR(path_stat.st_mode);
  }

  inline bool starts_with(const char *pre, const char *str)
  {
      size_t lenpre = strlen(pre),
            lenstr = strlen(str);
      return lenstr < lenpre ? false : memcmp(pre, str, lenpre) == 0;
  }
} // namespace impl

inline auto serve_file(const std::string& root, std::string_view path, http_response& response) {
  static char dot = '.', slash = '/';

  // remove first slash if needed.
  if (!path.empty() && path[0] == slash) {
    path = std::string_view(path.data() + 1, path.size() - 1); // erase(0, 1);
  }

  // Directory listing not supported.
  std::string full_path(root + std::string(path));
  if (path.empty() || !impl::is_regular_file(full_path)) {
    throw http_error::not_found("file not found.");
  }
  
  // Check if file exists by real file path.
  char realpath_out[PATH_MAX]{0};
  if (nullptr == realpath(full_path.c_str(), realpath_out))
    throw http_error::not_found("file not found.");

  // Check that path is within the root directory.
  if (!impl::starts_with(root.c_str(), realpath_out))
    throw http_error::not_found("Access denied.");

  response.write_static_file(full_path);
};

inline auto serve_directory(const std::string& root) {
  // extract root realpath. 
  char realpath_out[PATH_MAX]{0};
  if (nullptr == realpath(root.c_str(), realpath_out))
    throw std::runtime_error(std::string("serve_directory error: Directory ") + root + " does not exists.");

  // Check if it is a directory.
  if (!impl::is_directory(realpath_out))
  {
    throw std::runtime_error(std::string("serve_directory error: ") + root + " is not a directory.");
  }

  // Ensure the root ends with a /
  std::string real_root(realpath_out);
  if (real_root.back() != '/')
  {
    real_root.push_back('/');
  }

  http_api api;
  api.get("/{{path...}}") = [real_root](http_request& request, http_response& response) {
    auto path = request.url_parameters(s::path = std::string_view()).path;
    return serve_file(real_root, path, response);
  };
  return api;
}

} // namespace li
