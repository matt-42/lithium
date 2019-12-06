#pragma once

#include <string_view>
#include <microhttpd.h>

//#include <stdlib.h>
#include <fcntl.h>

#if defined(_MSC_VER)
#include <io.h>
#endif

//#include <sys/stat.h>

namespace li {
using namespace li;

struct http_response {
  inline http_response() : status(200), file_descriptor(-1) {}
  inline void set_header(std::string k, std::string v) { headers[k] = v; }
  inline void set_cookie(std::string k, std::string v) { cookies[k] = v; }

  inline void write() {}
  template <typename A1, typename... A>
  inline void write(A1 a1, A&&... a) { body += boost::lexical_cast<std::string>(a1); write(a...); }
  inline void write_file(const std::string path) {

#if defined(_MSC_VER)
    int fd; 
    _sopen_s(&fd, path.c_str(), O_RDONLY, _SH_DENYRW, _S_IREAD);
#else
    int fd = open(path.c_str(), O_RDONLY);
#endif

    if (fd == -1)
      throw http_error::not_found("File not found.");
    file_descriptor = fd;
  }

  int status;
  std::string body;
  int file_descriptor;
  std::unordered_map<std::string, std::string> cookies;
  std::unordered_map<std::string, std::string> headers;
};

} // namespace li
