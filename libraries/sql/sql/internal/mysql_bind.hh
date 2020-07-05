#pragma once

#include <mysql.h>

#include <li/callable_traits/callable_traits.hh>
#include <li/metamap/metamap.hh>
#include <li/sql/sql_common.hh>
#include <li/sql/symbols.hh>

#include <li/sql/internal/mysql_statement_data.hh>
#include <li/sql/mysql_async_wrapper.hh>

namespace li {

// Convert C++ types to mysql types.
auto type_to_mysql_statement_buffer_type(const char&) { return MYSQL_TYPE_TINY; }
auto type_to_mysql_statement_buffer_type(const short int&) { return MYSQL_TYPE_SHORT; }
auto type_to_mysql_statement_buffer_type(const int&) { return MYSQL_TYPE_LONG; }
auto type_to_mysql_statement_buffer_type(const long long int&) { return MYSQL_TYPE_LONGLONG; }
auto type_to_mysql_statement_buffer_type(const float&) { return MYSQL_TYPE_FLOAT; }
auto type_to_mysql_statement_buffer_type(const double&) { return MYSQL_TYPE_DOUBLE; }
auto type_to_mysql_statement_buffer_type(const sql_blob&) { return MYSQL_TYPE_BLOB; }
auto type_to_mysql_statement_buffer_type(const char*) { return MYSQL_TYPE_STRING; }
template <unsigned S> auto type_to_mysql_statement_buffer_type(const sql_varchar<S>) {
  return MYSQL_TYPE_STRING;
}

auto type_to_mysql_statement_buffer_type(const unsigned char&) { return MYSQL_TYPE_TINY; }
auto type_to_mysql_statement_buffer_type(const unsigned short int&) { return MYSQL_TYPE_SHORT; }
auto type_to_mysql_statement_buffer_type(const unsigned int&) { return MYSQL_TYPE_LONG; }
auto type_to_mysql_statement_buffer_type(const unsigned long long int&) {
  return MYSQL_TYPE_LONGLONG;
}

// Convert c++ type to mysql types.
template <typename T>
typename std::enable_if_t<std::is_integral<T>::value, std::string> cpptype_to_mysql_type(const T&) {
  return "INT";
}
template <typename T>
typename std::enable_if_t<std::is_floating_point<T>::value, std::string> cpptype_to_mysql_type(const T&) {
  return "DOUBLE";
}
inline std::string cpptype_to_mysql_type(const std::string&) { return "MEDIUMTEXT"; }
inline std::string cpptype_to_mysql_type(const sql_blob&) { return "BLOB"; }
template <unsigned S> inline std::string cpptype_to_mysql_type(const sql_varchar<S>) {
  std::ostringstream ss;
  ss << "VARCHAR(" << S << ')';
  return ss.str();
}

// Bind parameter functions
// Used to bind input parameters of prepared statement.
template <unsigned N> struct mysql_bind_data {
  mysql_bind_data() { memset(bind.data(), 0, N * sizeof(MYSQL_BIND)); }
  std::array<unsigned long, N> real_lengths;
  std::array<MYSQL_BIND, N> bind;
};

template <typename V> void mysql_bind_param(MYSQL_BIND& b, V& v) {
  b.buffer = const_cast<std::remove_const_t<V>*>(&v);
  b.buffer_type = type_to_mysql_statement_buffer_type(v);
  b.is_unsigned = std::is_unsigned<V>::value;
}

void mysql_bind_param(MYSQL_BIND& b, std::string& s) {
  b.buffer = &s[0];
  b.buffer_type = MYSQL_TYPE_STRING;
  b.buffer_length = s.size();
}
void mysql_bind_param(MYSQL_BIND& b, const std::string& s) {
  mysql_bind_param(b, *const_cast<std::string*>(&s));
}

template <unsigned SIZE> void mysql_bind_param(MYSQL_BIND& b, const sql_varchar<SIZE>& s) {
  mysql_bind_param(b, *const_cast<std::string*>(static_cast<const std::string*>(&s)));
}

void mysql_bind_param(MYSQL_BIND& b, char* s) {
  b.buffer = s;
  b.buffer_type = MYSQL_TYPE_STRING;
  b.buffer_length = strlen(s);
}
void mysql_bind_param(MYSQL_BIND& b, const char* s) { mysql_bind_param(b, const_cast<char*>(s)); }

void mysql_bind_param(MYSQL_BIND& b, sql_blob& s) {
  b.buffer = &s[0];
  b.buffer_type = MYSQL_TYPE_BLOB;
  b.buffer_length = s.size();
}
void mysql_bind_param(MYSQL_BIND& b, const sql_blob& s) {
  mysql_bind_param(b, *const_cast<sql_blob*>(&s));
}

void mysql_bind_param(MYSQL_BIND& b, sql_null_t n) { b.buffer_type = MYSQL_TYPE_NULL; }

//
// Bind output function.
// Used to bind output values to result sets.
//
template <typename T> void mysql_bind_output(MYSQL_BIND& b, unsigned long* real_length, T& v) {
  // Default to mysql_bind_param.
  mysql_bind_param(b, v);
}

void mysql_bind_output(MYSQL_BIND& b, unsigned long* real_length, std::string& v) {
  v.resize(100);
  b.buffer_type = MYSQL_TYPE_STRING;
  b.buffer_length = v.size();
  b.buffer = &v[0];
  b.length = real_length;
}

template <unsigned SIZE>
void mysql_bind_output(MYSQL_BIND& b, unsigned long* real_length, sql_varchar<SIZE>& s) {
  s.resize(SIZE);
  b.buffer = &s[0];
  b.buffer_type = MYSQL_TYPE_STRING;
  b.buffer_length = s.size();
  b.length = real_length;
}

template <typename A> mysql_bind_data<1> mysql_bind_output(mysql_statement_data& data, A& o) {
  if (data.num_fields_ != 1)
    throw std::runtime_error("mysql_statement error: The number of column in the result set "
                             "shoud be 1. Use std::tuple or li::sio to fetch several columns or "
                             "modify the request so that it returns a set of 1 column.");

  mysql_bind_data<1> bind_data;
  mysql_bind_output(bind_data.bind[0], &bind_data.real_lengths[0], o);
  return bind_data;
}

template <typename... A>
mysql_bind_data<sizeof...(A)> mysql_bind_output(mysql_statement_data& data, metamap<A...>& o) {
  if (data.num_fields_ != sizeof...(A)) {
    throw std::runtime_error(
        "mysql_statement error: Not enough columns in the result set to fill the object.");
  }

  mysql_bind_data<sizeof...(A)> bind_data;
  MYSQL_BIND* bind = bind_data.bind.data();
  unsigned long* real_lengths = bind_data.real_lengths.data();

  li::map(o, [&](auto k, auto& v) {
    // Find li::symbol_string(k) position.
    for (int i = 0; i < data.num_fields_; i++)
      if (!strcmp(data.fields_[i].name, li::symbol_string(k)))
      // bind the column.
      {
        mysql_bind_output(bind[i], real_lengths + i, v);
      }
  });

  for (int i = 0; i < data.num_fields_; i++) {
    if (!bind[i].buffer_type) {
      std::ostringstream ss;
      ss << "Error while binding the mysql request to a metamap object: " << std::endl
         << "   Field " << data.fields_[i].name << " could not be bound." << std::endl;
      throw std::runtime_error(ss.str());
    }
  }

  return bind_data;
}

template <typename... A>
mysql_bind_data<sizeof...(A)> mysql_bind_output(mysql_statement_data& data, std::tuple<A...>& o) {
  if (data.num_fields_ != sizeof...(A))
    throw std::runtime_error("mysql_statement error: The number of column in the result set does "
                             "not match the number of attributes of the tuple to bind.");

  mysql_bind_data<sizeof...(A)> bind_data;
  MYSQL_BIND* bind = bind_data.bind.data();
  unsigned long* real_lengths = bind_data.real_lengths.data();

  int i = 0;
  tuple_map(o, [&](auto& m) {
    mysql_bind_output(bind[i], real_lengths + i, m);
    i++;
  });

  return bind_data;
}

// Forward reference tuple impl.
template <typename... A>
mysql_bind_data<sizeof...(A)> mysql_bind_output(mysql_statement_data& data, std::tuple<A...>&& o) {
  if (data.num_fields_ != sizeof...(A))
    throw std::runtime_error("mysql_statement error: The number of column in the result set does "
                             "not match the number of attributes of the tuple to bind.");

  mysql_bind_data<sizeof...(A)> bind_data;
  MYSQL_BIND* bind = bind_data.bind.data();
  unsigned long* real_lengths = bind_data.real_lengths.data();

  int i = 0;
  tuple_map(std::forward<std::tuple<A...>>(o), [&](auto& m) {
    mysql_bind_output(bind[i], real_lengths + i, m);
    i++;
  });

  return bind_data;
}

} // namespace li
