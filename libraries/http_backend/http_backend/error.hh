#pragma once

#include <sstream>
#include <string>

namespace li {

template <typename E> inline void format_error_(E&) {}

template <typename E, typename T1, typename... T>
inline void format_error_(E& err, T1 a, T... args) {
  err << a;
  format_error_(err, std::forward<T>(args)...);
}

template <typename... T> inline std::string format_error(T&&... args) {
  std::stringstream ss;
  format_error_(ss, std::forward<T>(args)...);
  return ss.str();
}

struct http_error {
public:
  http_error(int status, const std::string& what) : status_(status), what_(what) {}
  http_error(int status, const char* what) : status_(status), what_(what) {}
  auto status() const { return status_; }
  auto what() const { return what_; }


#define LI_HTTP_ERROR(CODE, ERR)                                          \
  template <typename... A> static auto ERR(A&&... m) {                                \
    return http_error(CODE, format_error(m...));                                    \
  }                                                                            \
  static auto ERR(const char* w) { return http_error(CODE, w); }

LI_HTTP_ERROR(400, bad_request)
LI_HTTP_ERROR(401, unauthorized)
LI_HTTP_ERROR(403, forbidden)
LI_HTTP_ERROR(404, not_found)

LI_HTTP_ERROR(500, internal_server_error)
LI_HTTP_ERROR(501, not_implemented)

#undef LI_HTTP_ERROR


private:
  int status_;
  std::string what_;
};

} // namespace li
