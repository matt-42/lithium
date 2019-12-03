#pragma once

#include <string>
#include <memory>

namespace li {


  enum json_error_code
  {
    JSON_OK = 0,
    JSON_KO = 1
  };
    
  struct json_error
  {
    json_error& operator=(const json_error&) = default;
    operator bool() { return code != 0; }
    bool good() { return code == 0; }
    bool bad() { return code != 0; }
    int code;
    std::string what;
  };

  int make_json_error(const char* what) { return 1; }
  int json_no_error() { return 0; }

  static int json_ok = json_no_error();

}
