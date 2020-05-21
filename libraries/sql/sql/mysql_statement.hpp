#pragma once

namespace li {

template <typename B> long long int mysql_statement_result<B>::affected_rows() {
  return mysql_stmt_affected_rows(data_.stmt_);
}

template <typename B> 
template <typename T>
void mysql_statement_result<B>::fetch_column(MYSQL_BIND*, unsigned long, T&, int) {}

template <typename B> 
void mysql_statement_result<B>::fetch_column(MYSQL_BIND* b, unsigned long real_length, std::string& v, int i) {
  if (real_length <= v.size()) {
    v.resize(real_length);
    return;
  }

  v.resize(real_length);
  b[i].buffer_length = real_length;
  b[i].length = nullptr;
  b[i].buffer = &v[0];
  if (mysql_stmt_fetch_column(data_.stmt_, b + i, i, 0) != 0) {
    *connection_status_ = 1;
    throw std::runtime_error(std::string("mysql_stmt_fetch_column error: ") +
                             mysql_stmt_error(data_.stmt_));
  }
}
template <typename B> template <unsigned SIZE>
void mysql_statement_result<B>::fetch_column(MYSQL_BIND& b, unsigned long real_length,
                                             sql_varchar<SIZE>& v, int i) {
  v.resize(real_length);
}

template <typename B> template <typename T> int mysql_statement_result<B>::fetch(T&& o) {

  auto bind_data = mysql_bind_output(data_, o);
  unsigned long* real_lengths = bind_data.real_lengths.data();
  MYSQL_BIND* bind = bind_data.bind.data();

  mysql_stmt_bind_result(data_.stmt_, bind);

  int f = mysql_wrapper_.mysql_stmt_fetch(connection_status_, data_.stmt_);
  int res = 1;
  if (f == MYSQL_NO_DATA)
    res = 0;
  else
    finalize_fetch(bind, real_lengths, o);

  mysql_wrapper_.mysql_stmt_free_result(connection_status_, data_.stmt_);
  return res;
}

template <typename B> template <typename T> void mysql_statement_result<B>::read(std::optional<T>& o) {
  T t;
  if (fetch(t))
    o = std::optional<T>(t);
  else
    o = std::optional<T>();
}

template <typename B> template <typename T> T mysql_statement_result<B>::read() {
  T t;
  if (!fetch(t))
    throw std::runtime_error("Request did not return any data.");
  return t;
}

template <typename B> template <typename T> std::optional<T> mysql_statement_result<B>::read_optional() {
  T t;
  if (fetch(t))
    return std::optional<T>(t);
  else
    return std::optional<T>();
}

template <typename T> struct unconstref_tuple_elements {};
template <typename... T> struct unconstref_tuple_elements<std::tuple<T...>> {
  typedef std::tuple<std::remove_const_t<std::remove_reference_t<T>>...> ret;
};

template <typename B> template <typename F> void mysql_statement_result<B>::map(F f) {

  typedef typename unconstref_tuple_elements<callable_arguments_tuple_t<F>>::ret tp;
  typedef std::remove_const_t<std::remove_reference_t<std::tuple_element_t<0, tp>>> T;

  auto o = []() {
    if constexpr (li::is_metamap<T>::value)
      return T{};
    else
      return tp{};
  }();

  auto bind_data = mysql_bind_output(data_, o);
  mysql_stmt_bind_result(data_.stmt_, bind_data.bind.data());

  while (mysql_wrapper_.mysql_stmt_fetch(connection_status_, data_.stmt_) != MYSQL_NO_DATA) {
    this->finalize_fetch(bind_data.bind.data(), bind_data.real_lengths.data(), o);
    if constexpr (li::is_metamap<T>::value)
      f(o);
    else
      std::apply(f, o);
  }

  mysql_wrapper_.mysql_stmt_free_result(connection_status_, data_.stmt_);
}

template <typename B> template <typename... A>
void mysql_statement_result<B>::finalize_fetch(MYSQL_BIND* bind, unsigned long* real_lengths,
                                               metamap<A...>& o) {
  li::map(o, [&](auto k, auto& v) {
    for (int i = 0; i < data_.num_fields_; i++)
      if (!strcmp(data_.fields_[i].name, li::symbol_string(k)))
        this->fetch_column(bind, real_lengths[i], v, i);
  });
}

template <typename B> template <typename... A>
void mysql_statement_result<B>::finalize_fetch(MYSQL_BIND* bind, unsigned long* real_lengths,
                                               std::tuple<A...>& o) {
  int i = 0;
  tuple_map(o, [&](auto& m) {
    this->fetch_column(bind, real_lengths[i], m, i);
    i++;
  });
}

template <typename B> template <typename A>
void mysql_statement_result<B>::finalize_fetch(MYSQL_BIND* bind, unsigned long* real_lengths,
                                               A& o) {
  this->fetch_column(bind, real_lengths[0], o, 0);
}

template <typename B> void mysql_statement_result<B>::wait() {}

template <typename B> long long int mysql_statement_result<B>::last_insert_id() {
  return mysql_stmt_insert_id(data_.stmt_);
}

template <typename B> bool mysql_statement_result<B>::empty() {
  return mysql_wrapper_.mysql_stmt_fetch(connection_status_, data_.stmt_) == MYSQL_NO_DATA;
}

template <typename B> mysql_statement_result<B> mysql_statement<B>::operator()() {
  mysql_wrapper_.mysql_stmt_execute(connection_status_, data_.stmt_);
  return mysql_statement_result<B>{mysql_wrapper_, data_, connection_status_};
}

template <typename B> template <typename... T>
mysql_statement_result<B> mysql_statement<B>::operator()(T&&... args) {
  MYSQL_BIND bind[sizeof...(T)];
  memset(bind, 0, sizeof(bind));

  int i = 0;
  tuple_map(std::forward_as_tuple(args...), [&](auto& m) {
    mysql_bind_param(bind[i], m);
    i++;
  });

  if (mysql_stmt_bind_param(data_.stmt_, bind) != 0) {
    *connection_status_ = 1;
    throw std::runtime_error(std::string("mysql_stmt_bind_param error: ") +
                             mysql_stmt_error(data_.stmt_));
  }
  mysql_wrapper_.mysql_stmt_execute(connection_status_, data_.stmt_);
  return mysql_statement_result<B>{mysql_wrapper_, data_, connection_status_};
}

} // namespace li
