#pragma once

#include <cstring>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <optional>

#include <mysql.h>

#include <li/callable_traits/callable_traits.hh>
#include <li/metamap/metamap.hh>
#include <li/sql/sql_common.hh>
#include <li/sql/symbols.hh>

namespace li {

struct mysql_scoped_use_result {
  inline mysql_scoped_use_result(MYSQL* con) : res_(mysql_use_result(con)) {
    if (!res_)
      throw std::runtime_error(std::string("Mysql error: ") + mysql_error(con));
  }

  inline ~mysql_scoped_use_result() { mysql_free_result(res_); }
  inline operator MYSQL_RES*() { return res_; }

private:
  MYSQL_RES* res_;
};

auto type_to_mysql_statement_buffer_type(const char&) { return MYSQL_TYPE_TINY; }
auto type_to_mysql_statement_buffer_type(const short int&) { return MYSQL_TYPE_SHORT; }
auto type_to_mysql_statement_buffer_type(const int&) { return MYSQL_TYPE_LONG; }
auto type_to_mysql_statement_buffer_type(const long long int&) { return MYSQL_TYPE_LONGLONG; }
auto type_to_mysql_statement_buffer_type(const float&) { return MYSQL_TYPE_FLOAT; }
auto type_to_mysql_statement_buffer_type(const double&) { return MYSQL_TYPE_DOUBLE; }
auto type_to_mysql_statement_buffer_type(const sql_blob&) { return MYSQL_TYPE_BLOB; }
auto type_to_mysql_statement_buffer_type(const char*) { return MYSQL_TYPE_STRING; }
template <unsigned S>
auto type_to_mysql_statement_buffer_type(const sql_varchar<S>) { return MYSQL_TYPE_STRING; }

auto type_to_mysql_statement_buffer_type(const unsigned char&) { return MYSQL_TYPE_TINY; }
auto type_to_mysql_statement_buffer_type(const unsigned short int&) { return MYSQL_TYPE_SHORT; }
auto type_to_mysql_statement_buffer_type(const unsigned int&) { return MYSQL_TYPE_LONG; }
auto type_to_mysql_statement_buffer_type(const unsigned long long int&) {
  return MYSQL_TYPE_LONGLONG;
}

struct mysql_statement {
  mysql_statement()
      : stmt_(nullptr), stmt_sptr_(nullptr), num_fields_(-1), metadata_(nullptr),
        metadata_sptr_(nullptr), fields_(nullptr) {}

  mysql_statement(MYSQL_STMT* s) : stmt_(s) {
    metadata_ = mysql_stmt_result_metadata(stmt_);
    stmt_sptr_ = std::shared_ptr<MYSQL_STMT>(stmt_, mysql_stmt_close);

    if (metadata_) {
      metadata_sptr_ = std::shared_ptr<MYSQL_RES>(metadata_, mysql_free_result);
      fields_ = mysql_fetch_fields(metadata_);
      num_fields_ = mysql_num_fields(metadata_);
    } else {
      num_fields_ = -1;
      metadata_ = nullptr;
      metadata_sptr_ = nullptr;
      fields_ = nullptr;
    }
  }

  auto& operator()() {
  if (mysql_stmt_execute(stmt_) != 0)
      throw std::runtime_error(std::string("mysql_stmt_execute error: ") + mysql_stmt_error(stmt_));
    return *this;
  }

  template <typename... T> auto& operator()(T&&... args) {
    MYSQL_BIND bind[sizeof...(T)];
    memset(bind, 0, sizeof(bind));

    int i = 0;
    tuple_map(std::forward_as_tuple(args...), [&](auto& m) {
      this->bind(bind[i], m);
      i++;
    });

    if (mysql_stmt_bind_param(stmt_, bind) != 0)
      throw std::runtime_error(std::string("mysql_stmt_bind_param error: ") +
                               mysql_stmt_error(stmt_));

    if (mysql_stmt_execute(stmt_) != 0)
      throw std::runtime_error(std::string("mysql_stmt_execute error: ") + mysql_stmt_error(stmt_));

    return *this;
  }

  long long int affected_rows() { return mysql_stmt_affected_rows(stmt_); }

  template <typename V> void bind(MYSQL_BIND& b, V& v) {
    b.buffer = const_cast<std::remove_const_t<V>*>(&v);
    b.buffer_type = type_to_mysql_statement_buffer_type(v);
    b.is_unsigned = std::is_unsigned<V>::value;
  }

  void bind(MYSQL_BIND& b, std::string& s) {
    b.buffer = &s[0];
    b.buffer_type = MYSQL_TYPE_STRING;
    b.buffer_length = s.size();
  }
  void bind(MYSQL_BIND& b, const std::string& s) { bind(b, *const_cast<std::string*>(&s)); }
  template <unsigned SIZE>
  void bind(MYSQL_BIND& b, const sql_varchar<SIZE>& s) { bind(b, *const_cast<std::string*>(static_cast<const std::string*>(&s))); }

  void bind(MYSQL_BIND& b, char* s) {
    b.buffer = s;
    b.buffer_type = MYSQL_TYPE_STRING;
    b.buffer_length = strlen(s);
  }
  void bind(MYSQL_BIND& b, const char* s) { bind(b, const_cast<char*>(s)); }

  void bind(MYSQL_BIND& b, sql_blob& s) {
    b.buffer = &s[0];
    b.buffer_type = MYSQL_TYPE_BLOB;
    b.buffer_length = s.size();
  }
  void bind(MYSQL_BIND& b, const sql_blob& s) { bind(b, *const_cast<sql_blob*>(&s)); }

  void bind(MYSQL_BIND& b, sql_null_t n) { b.buffer_type = MYSQL_TYPE_NULL; }

  template <typename T> void fetch_column(MYSQL_BIND*, unsigned long, T&, int) {}
  void fetch_column(MYSQL_BIND* b, unsigned long real_length, std::string& v, int i) {
    v.resize(real_length);
    b[i].buffer_length = real_length;
    b[i].length = nullptr;
    b[i].buffer = &v[0];
    if (mysql_stmt_fetch_column(stmt_, b + i, i, 0) != 0)
      throw std::runtime_error(std::string("mysql_stmt_fetch_column error: ") +
                               mysql_stmt_error(stmt_));
  }

  template <typename T> void bind_output(MYSQL_BIND& b, unsigned long* real_length, T& v) {
    bind(b, v);
  }
  void bind_output(MYSQL_BIND& b, unsigned long* real_length, std::string& v) {
    b.buffer_type = MYSQL_TYPE_STRING;
    b.buffer_length = 0;
    b.buffer = 0;
    b.length = real_length;
  }

  template <typename A> static constexpr auto number_of_fields(const A&) {
    return std::integral_constant<int, 1>();
  }
  template <typename... A> static constexpr auto number_of_fields(const metamap<A...>&) {
    return std::integral_constant<int, sizeof...(A)>();
  }
  template <typename... A> static constexpr auto number_of_fields(const std::tuple<A...>&) {
    return std::integral_constant<int, sizeof...(A)>();
  }

  template <typename T> int fetch(T&& o) {

    constexpr int size = decltype(number_of_fields(o))::value;
    unsigned long real_lengths[size];
    MYSQL_BIND bind[size];
    memset(bind, 0, sizeof(bind));

    prepare_fetch(bind, real_lengths, o);
    int f = fetch();
    int res = 1;
    if (f == MYSQL_NO_DATA)
      res = 0;
    else
      finalize_fetch(bind, real_lengths, o);

    mysql_stmt_free_result(stmt_);
    return res;
  }
  template <typename T> void read(std::optional<T>& o) {
    T t;
    if (fetch(t))
      o = std::optional<T>(t);
    else
      o = std::optional<T>();
  }

  template <typename T> T read() {
    T t;
    if (!fetch(t))
      throw std::runtime_error("Request did not return any data.");
    return t;
  }

  template <typename T> std::optional<T> read_optional() {
    T t;
    if (fetch(t))
      return std::optional<T>(t);
    else
      return std::optional<T>();
  }

  template <typename F> void map(F f) {
    typedef callable_arguments_tuple_t<F> tp;
    typedef std::remove_reference_t<std::tuple_element_t<0, tp>> T;
    if constexpr (li::is_metamap<T>::ret) {
      T o;

      unsigned long real_lengths[decltype(number_of_fields(T()))::value];
      MYSQL_BIND bind[decltype(number_of_fields(T()))::value];
      memset(bind, 0, sizeof(bind));
      this->prepare_fetch(bind, real_lengths, o);
      while (this->fetch() != MYSQL_NO_DATA) {
        this->finalize_fetch(bind, real_lengths, o);
        f(o);
      }
      mysql_stmt_free_result(stmt_);
    } else {
      tp o;

      unsigned long real_lengths[std::tuple_size<tp>::value];
      MYSQL_BIND bind[std::tuple_size<tp>::value];
      memset(bind, 0, sizeof(bind));
      this->prepare_fetch(bind, real_lengths, o);
      while (this->fetch() != MYSQL_NO_DATA) {
        this->finalize_fetch(bind, real_lengths, o);
        apply(o, f);
      }
      mysql_stmt_free_result(stmt_);
    }
  }

  template <typename A> void prepare_fetch(MYSQL_BIND* bind, unsigned long* real_lengths, A& o) {
    if (num_fields_ != 1)
      throw std::runtime_error("mysql_statement error: The number of column in the result set "
                               "shoud be 1. Use std::tuple or li::sio to fetch several columns or "
                               "modify the request so that it returns a set of 1 column.");

    this->bind_output(bind[0], real_lengths, o);
    mysql_stmt_bind_result(stmt_, bind);
  }

  template <typename... A>
  void prepare_fetch(MYSQL_BIND* bind, unsigned long* real_lengths, metamap<A...>& o) {
    if (num_fields_ != sizeof...(A)) {
      throw std::runtime_error(
          "mysql_statement error: Not enough columns in the result set to fill the object.");
    }

    li::map(o, [&](auto k, auto& v) {
      // Find li::symbol_string(k) position.
      for (int i = 0; i < num_fields_; i++)
        if (!strcmp(fields_[i].name, li::symbol_string(k)))
        // bind the column.
        {
          this->bind_output(bind[i], real_lengths + i, v);
        }
    });

    for (int i = 0; i < num_fields_; i++) {
      if (!bind[i].buffer_type) {
        std::stringstream ss;
        ss << "Error while binding the mysql request to a SIO object: " << std::endl
           << "   Field " << fields_[i].name << " could not be bound." << std::endl;
        throw std::runtime_error(ss.str());
      }
    }

    mysql_stmt_bind_result(stmt_, bind);
  }

  template <typename... A>
  void prepare_fetch(MYSQL_BIND* bind, unsigned long* real_lengths, std::tuple<A...>& o) {
    if (num_fields_ != sizeof...(A))
      throw std::runtime_error("mysql_statement error: The number of column in the result set does "
                               "not match the number of attributes of the tuple to bind.");

    int i = 0;
    tuple_map(o, [&](auto& m) {
      this->bind_output(bind[i], real_lengths + i, m);
      i++;
    });

    mysql_stmt_bind_result(stmt_, bind);
  }

  template <typename... A>
  void finalize_fetch(MYSQL_BIND* bind, unsigned long* real_lengths, metamap<A...>& o) {
    li::map(o, [&](auto k, auto& v) {
      for (int i = 0; i < num_fields_; i++)
        if (!strcmp(fields_[i].name, li::symbol_string(k)))
          this->fetch_column(bind, real_lengths[i], v, i);
    });
  }

  template <typename... A>
  void finalize_fetch(MYSQL_BIND* bind, unsigned long* real_lengths, std::tuple<A...>& o) {
    int i = 0;
    tuple_map(o, [&](auto& m) {
      this->fetch_column(bind, real_lengths[i], m, i);
      i++;
    });
  }

  template <typename A> void finalize_fetch(MYSQL_BIND* bind, unsigned long* real_lengths, A& o) {
    this->fetch_column(bind, real_lengths[0], o, 0);
  }

  int fetch() {
    int f = mysql_stmt_fetch(stmt_);
    if (f == 1)
      throw std::runtime_error(std::string("mysql_stmt_fetch error: ") + mysql_stmt_error(stmt_));
    return f;
  }

  long long int last_insert_id() { return mysql_stmt_insert_id(stmt_); }

  bool empty() { return mysql_stmt_fetch(stmt_) == MYSQL_NO_DATA; }

  MYSQL_STMT* stmt_;
  std::shared_ptr<MYSQL_STMT> stmt_sptr_;
  int num_fields_;
  MYSQL_RES* metadata_;
  std::shared_ptr<MYSQL_RES> metadata_sptr_;
  MYSQL_FIELD* fields_;
}; // namespace li

struct mysql_database;

struct mysql_connection {
  //mysql_connection(MYSQL* con) : con_(con) {}

  // template <typename... OPTS> inline mysql_connection(OPTS&&... opts) :
  // con_(impl::open_mysql_connection(opts...)) {}

  inline mysql_connection(MYSQL* con, mysql_database& pool);

  long long int last_insert_rowid() { return mysql_insert_id(con_); }

  mysql_statement& operator()(std::string rq) {
    return prepare(rq)();
  }

  mysql_statement& prepare(std::string rq) {
    std::cout << rq << std::endl;
    auto it = stm_cache_.find(rq);
    if (it != stm_cache_.end())
      return it->second;

    MYSQL_STMT* stmt = mysql_stmt_init(con_);
    if (!stmt)
      throw std::runtime_error(std::string("mysql_stmt_init error: ") + mysql_error(con_));

    if (mysql_stmt_prepare(stmt, rq.data(), rq.size()))
      throw std::runtime_error(std::string("mysql_stmt_prepare error: ") + mysql_error(con_));

    stm_cache_[rq] = mysql_statement(stmt);
    return stm_cache_[rq];
  }

  template <typename T>
  inline std::string type_to_string(const T&, std::enable_if_t<std::is_integral<T>::value>* = 0) {
    return "INT";
  }
  template <typename T>
  inline std::string type_to_string(const T&,
                                    std::enable_if_t<std::is_floating_point<T>::value>* = 0) {
    return "DOUBLE";
  }
  inline std::string type_to_string(const std::string&) { return "MEDIUMTEXT"; }
  inline std::string type_to_string(const sql_blob&) { return "BLOB"; }
  template <unsigned S>
  inline std::string type_to_string(const sql_varchar<S>) { 
    std::stringstream ss;
    ss << "VARCHAR(" << S << ')'; return ss.str(); }

  std::unordered_map<std::string, mysql_statement> stm_cache_;
  MYSQL* con_;
  std::shared_ptr<int> sptr_;
  mysql_database& pool_;
};

struct mysql_database : std::enable_shared_from_this<mysql_database> {

  template <typename... O> inline mysql_database(O... opts) {

    auto options = mmm(opts...);
    static_assert(has_key(options, s::host), "open_mysql_connection requires the s::host argument");
    static_assert(has_key(options, s::database),
                  "open_mysql_connection requires the s::databaser argument");
    static_assert(has_key(options, s::user), "open_mysql_connection requires the s::user argument");
    static_assert(has_key(options, s::password),
                  "open_mysql_connection requires the s::password argument");

    host_ = options.host;
    database_ = options.database;
    user_ = options.user;
    passwd_ = options.password;
    port_ = get_or(options, s::port, 0);
 
    character_set_ = get_or(options, s::charset, "utf8");

    if (mysql_library_init(0, NULL, NULL))
      throw std::runtime_error("Could not initialize MySQL library.");
    if (!mysql_thread_safe())
      throw std::runtime_error("Mysql is not compiled as thread safe.");
  }

  inline void free_connection(MYSQL* c) {
    std::unique_lock<std::mutex> l(mutex_);
    pool_.push_back(c);
  }

  inline mysql_connection get_connection() {
    MYSQL* con_ = nullptr;
    {
      std::unique_lock<std::mutex> l(mutex_);
      if (!pool_.empty()) {
        con_ = pool_.back();
        pool_.pop_back();
      }
    }

    if (!con_) {
      con_ = mysql_init(con_);
      con_ = mysql_real_connect(con_, host_.c_str(), user_.c_str(), passwd_.c_str(),
                                database_.c_str(), port_, NULL, 0);
      if (!con_)
        throw std::runtime_error("Cannot connect to the database");

      mysql_set_character_set(con_, character_set_.c_str());
    }

    assert(con_);
    return mysql_connection(con_, *this);
  }

  std::mutex mutex_;
  std::string host_, user_, passwd_, database_;
  unsigned int port_;
  std::deque<MYSQL*> pool_;
  std::string character_set_;
};

inline mysql_connection::mysql_connection(MYSQL* con, mysql_database& pool)
    : con_(con), pool_(pool) {
  sptr_ = std::shared_ptr<int>((int*)42, [&pool, con](int* p) { pool.free_connection(con); });
}

} // namespace li
