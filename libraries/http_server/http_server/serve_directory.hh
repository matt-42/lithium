#pragma once

#include <stdlib.h>

#include <memory>
#include <string>

#include <li/http_server/response.hh>

namespace li {

namespace impl {
inline bool is_regular_file(const std::string& path) {
  struct stat path_stat;
  if (-1 == stat(path.c_str(), &path_stat))
    return false;
  return path_stat.st_mode & S_IFREG;
}

inline bool is_directory(const std::string& path) {
  struct stat path_stat;
  if (-1 == stat(path.c_str(), &path_stat))
    return false;
  return path_stat.st_mode & S_IFDIR;
}

inline bool starts_with(const char* pre, const char* str) {
  size_t lenpre = strlen(pre), lenstr = strlen(str);
  return lenstr < lenpre ? false : memcmp(pre, str, lenpre) == 0;
}
} // namespace impl

#if defined _WIN32
#define CROSSPLATFORM_MAX_PATH MAX_PATH
#else
#define CROSSPLATFORM_MAX_PATH PATH_MAX
#endif

template <unsigned S> bool crossplatform_realpath(const std::string& path, char out_buffer[S]) {
  // Check if file exists by real file path.
#if defined _WIN32
  return 0 != GetFullPathNameA(path.c_str(), S, out_buffer, nullptr);
#else
  return nullptr != realpath(path.c_str(), out_buffer);
#endif
}

inline auto serve_file(const std::string& root, std::string_view path, http_response& response) {
  static char dot = '.', slash = '/';
  // remove first slash if needed.
  if (!path.empty() && path[0] == slash) {
    path = std::string_view(path.data() + 1, path.size() - 1); // erase(0, 1);
  }
  if (path[0] == ' ') path = "index.html";
  // Directory listing not supported.
  std::string full_path(root + std::string(path));
  //std::cout << path << " + " << full_path << std::endl;
  if (path.empty() || !impl::is_regular_file(full_path)) {
    throw http_error::not_found("file not found.");
  }
  // Check if file exists by real file path.
  char realpath_out[CROSSPLATFORM_MAX_PATH]{0};
  if (!crossplatform_realpath<CROSSPLATFORM_MAX_PATH>(full_path, realpath_out))
    throw http_error::not_found("file not found.");
  
  //std::cout << root.c_str() << " + " << realpath_out << std::endl;
  // Check that path is within the root directory.
  if (!impl::starts_with(root.c_str(), realpath_out))
    throw http_error::not_found("Access denied.");

  response.write_file(full_path);
};

inline auto serve_directory(const std::string& root) {
  // extract root realpath.

  char realpath_out[CROSSPLATFORM_MAX_PATH]{0};
  if (!crossplatform_realpath<CROSSPLATFORM_MAX_PATH>(root, realpath_out))
    throw std::runtime_error(std::string("serve_directory error: Directory ") + root +
                             " does not exists.");

  // Check if it is a directory.
  if (!impl::is_directory(realpath_out)) {
    throw std::runtime_error(std::string("serve_directory error: ") + root +
                             " is not a directory.");
  }

  // Ensure the root ends with a /
  std::string real_root(realpath_out);
#if _WIN32
  if (real_root.back() != '\\') {
    real_root.push_back('\\');
  }
#else
  if (real_root.back() != '/') {
    real_root.push_back('/');
  }
#endif

  http_api api;
  api.get("/{{path...}}") = [real_root](http_request& request, http_response& response) {
    auto path = request.url_parameters(s::path = std::string_view()).path;
    return serve_file(real_root, path, response);
  };
  return api;
}

} // namespace li
