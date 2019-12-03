#pragma once

#include <string_view>
#include <microhttpd.h>

//#include <stdlib.h>
#include <fcntl.h>
//#include <sys/stat.h>

namespace li {
using namespace li;

struct http_response {
  inline http_response() : status(200), file_descriptor(-1) {}
  inline void set_header(std::string k, std::string v) { headers[k] = v; }
  inline void set_cookie(std::string k, std::string v) { cookies[k] = v; }

  void write(const std::string res) { body = res; }
  void write_file(const std::string path) {

    int fd = open(path.c_str(), O_RDONLY);

    if (fd == -1)
      throw http_error::not_found("File not found.");
    file_descriptor = fd;
  }
  // void write_file(http_response* r, const std::string& path) const {
  //   int fd = open(path.c_str(), O_RDONLY);

  //   if (fd == -1)
  //     throw http_error::not_found("File not found.");

  //   // Read extension.
  //   int c = path.size();
  //   while (c >= 1 and path[c - 1] != '.')
  //     c--;
  //   if (c > 1 and c < path.size()) {
  //     const char* ext = path.c_str() + c;
  //     if (!strcmp(ext, "js"))
  //       r->set_header("Content-Type", "text/javascript");
  //     if (!strcmp(ext, "css"))
  //       r->set_header("Content-Type", "text/css");
  //     if (!strcmp(ext, "html"))
  //       r->set_header("Content-Type", "text/html");
  //   }
  //   r->file_descriptor = fd;
  // }

  void write(const std::string_view res) {
    body = std::string(res.data(), res.size());
  }

  void write(const char* res) { body = res; }

  int status;
  std::string body;
  int file_descriptor;
  std::unordered_map<std::string, std::string> cookies;
  std::unordered_map<std::string, std::string> headers;
};

} // namespace li
