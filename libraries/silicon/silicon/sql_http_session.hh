#pragma once

#include <iod/silicon/sql_orm.hh>
#include <iod/silicon/cookie.hh>


namespace iod {

template <typename ORM> struct connected_sql_http_session {

  // Construct the session.
  // Retrive the cookie
  // Retrieve it from the database.
  // Insert it if it does not exists.
  connected_sql_http_session(typename ORM::object_type& defaults, ORM orm, const std::string& session_id)
      : loaded_(false), session_id_(session_id), orm_(orm), values_(defaults) {

      }

  // Store fiels into the session
  template <typename... F> auto store(F... fields) {
    map(mmm(fields...), [this](auto k, auto v) { values_[k] = v; });
    if (!orm_.exists(s::session_id = session_id_))
      orm_.insert(s::session_id = session_id_, fields...);
    else
      orm_.update(s::session_id = session_id_, fields...);
  }

  // Access values of the session.
  const auto values() {
    load();
    return values_;
  }

  // Delete the session from the database.
  void logout() { orm_.remove(s::session_id = session_id_); }

private:
  auto load() {
    if (loaded_)
      return;
    if (auto new_values_ = orm_.find_one(s::session_id = session_id_))
      values_ = *new_values_;
    loaded_ = true;
  }

  bool loaded_;
  std::string session_id_;
  ORM orm_;
  typename ORM::object_type values_;
};

template <typename... F>
decltype(auto) create_session_orm(std::string table_name, F... fields)
{
  return sql_orm_schema(table_name)
                           .fields(s::session_id(s::read_only, s::primary_key) = sql_varchar<32>(), fields...);
}

template <typename... F> struct sql_http_session {

  sql_http_session(std::string table_name, F... fields)
      : default_values_(mmm(s::session_id = sql_varchar<32>(), fields...)), session_table_(create_session_orm(table_name, fields...)) 
      {
      }

  template <typename DB>
  auto connect(DB& database, http_request& request, http_response& response) {
    return connected_sql_http_session(default_values_, session_table_.connect(database),
                                      session_cookie(request, response));
  }

  auto orm() { return session_table_; }

  std::decay_t<decltype(mmm(s::session_id = sql_varchar<32>(), std::declval<F>()...))> default_values_;
  std::decay_t<decltype(create_session_orm(std::string(), std::declval<F>()...))> session_table_;
};

} // namespace iod
