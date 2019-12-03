#pragma once

#include <string>

namespace li
{
  struct sql_blob : public std::string {
    using std::string::string;
    using std::string::operator=;

    sql_blob() : std::string() {}
  };

  struct sql_null_t {};
  static sql_null_t null;

  template <unsigned SIZE>
  struct sql_varchar : public std::string {
    using std::string::string;
    using std::string::operator=;

    sql_varchar() : std::string() {}
  };
}