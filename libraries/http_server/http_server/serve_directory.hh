#pragma once

#include <stdlib.h>
#include <li/http_server/response.hh>
#include <string>

namespace li {

namespace impl {
inline int is_regular_file(const std::string& path) {
  struct stat path_stat;
  if (-1 == stat(path.c_str(), &path_stat))
    return false;
  return S_ISREG(path_stat.st_mode);
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
  size_t len = path.size();
  if (!path.empty() && path[0] == slash) {
    path = std::string_view(path.data() + 1, path.size() - 1); // erase(0, 1);
  }

  // Directory listing not supported.
  std::string full_path(root + std::string(path));
  // Check that path is within the root directory.
  std::cout << root << " - " <<  full_path << std::endl;

  if (path.empty() || !impl::is_regular_file(full_path)) {
    throw http_error::not_found("file not found.");
  }

  
  char realpath_out[PATH_MAX];
  if (nullptr == realpath(full_path.c_str(), realpath_out))
    throw http_error::not_found("file not found.");

  std::cout << root << " - " <<  realpath_out << std::endl;
  if (!impl::starts_with(root.c_str(), realpath_out))
    throw http_error::not_found("Access denied.");

  response.write_static_file(full_path);
};

inline auto serve_directory(std::string root) {
  http_api api;

  // Ensure the root ends with a /

  // extract root realpath. 
  char realpath_out[PATH_MAX];
  if (nullptr == realpath(root.c_str(), realpath_out))
    throw std::runtime_error(std::string("serve_directory error: Directory ") + root + " does not exists.");

  root = std::string(realpath_out);

  if (!root.empty() && root[root.size() - 1] != '/') {
    root.push_back('/');
  }


  api.get("/{{path...}}") = [root](http_request& request, http_response& response) {
    auto path = request.url_parameters(s::path = std::string_view()).path;
    return serve_file(root, path, response);
  };
  return api;
}

} // namespace li
