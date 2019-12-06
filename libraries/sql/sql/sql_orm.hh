#pragma once

#include <li/metamap/metamap.hh>
#include <li/http_backend/symbols.hh>
#include <iostream>
#include <sstream>

#include <li/sql/symbols.hh>

namespace li {

struct sqlite_connection;
struct mysql_connection;

using s::auto_increment;
using s::primary_key;
using s::read_only;

template <typename SCHEMA, typename C> struct sql_orm {

  typedef decltype(std::declval<SCHEMA>().all_fields()) O;
  typedef O object_type;

  sql_orm(SCHEMA& schema, C con) : schema_(schema), con_(con) {}

  template <typename S, typename... A> void call_callback(S s, A&&... args) {
    if constexpr(has_key<decltype(schema_.get_callbacks())>(S{}))
      return schema_.get_callbacks()[s](args...);
  }

  inline auto drop_table_if_exists() {
    con_(std::string("DROP TABLE IF EXISTS ") + schema_.table_name());
    return *this;
  }

  inline auto create_table_if_not_exists() {
    std::ostringstream ss;
    ss << "CREATE TABLE if not exists " << schema_.table_name() << " (";

    bool first = true;
    li::tuple_map(schema_.all_info(), [&](auto f) {
      auto f2 = schema_.get_field(f);
      typedef decltype(f) F;
      typedef decltype(f2) F2;
      typedef typename F2::left_t K;
      typedef typename F2::right_t V;

      bool auto_increment = SCHEMA::template is_auto_increment<F>::value;
      bool primary_key = SCHEMA::template is_primary_key<F>::value;
      K k{};
      V v{};

      if (!first)
        ss << ", ";
      ss << li::symbol_string(k) << " " << con_.type_to_string(v);

      if (std::is_same<C, sqlite_connection>::value) {
        if (auto_increment || primary_key)
          ss << " PRIMARY KEY ";
      }

      if (std::is_same<C, mysql_connection>::value) {
        if (auto_increment)
          ss << " AUTO_INCREMENT NOT NULL";
        if (primary_key)
          ss << " PRIMARY KEY ";
      }

      // To activate when pgsql_connection is implemented.
      // if (std::is_same<C, pgsql_connection>::value and
      //     m.attributes().has(s::auto_increment))
      //   ss << " SERIAL ";

      first = false;
    });
    ss << ");";
    try {
      con_(ss.str())();
    } catch (std::exception e) {
      std::cerr << "Warning: Silicon could not create the " << schema_.table_name() << " sql table."
                << std::endl
                << "You can ignore this message if the table already exists."
                << "The sql error is: " << e.what() << std::endl;
    }
    return *this;
  }

  template <typename W>
  void where_clause(W&& cond, std::ostringstream& ss)
  {
    ss << " WHERE ";
    bool first = true;
    map(cond, [&](auto k, auto v) {
      if (!first)
        ss << " and ";
      first = false;
      ss << li::symbol_string(k) << " = ? ";
    });
    ss << " ";
  }

  template <typename... W, typename... A> auto find_one(metamap<W...> where, A&&... cb_args) {
    std::ostringstream ss;
    O o;
    ss << "SELECT ";
    bool first = true;
    li::map(o, [&](auto k, auto v) {
      if (!first)
        ss << ",";
      first = false;
      ss << li::symbol_string(k);
    });

    ss << " FROM " << schema_.table_name();
    where_clause(where, ss);
    ss << "LIMIT 1";
    auto stmt = con_.prepare(ss.str());

    auto res = li::tuple_reduce(metamap_values(where), stmt).template read_optional<O>();
    if (res)
      call_callback(s::read_access, o, cb_args...);
    return res;
  }

  template <typename A, typename B, typename... O, typename... W>
  auto find_one(metamap<O...>&& o, assign_exp<A, B> w1, W... ws) {
    return find_one(cat(o, mmm(w1)), ws...);
  }
  template <typename A, typename B, typename... W>
  auto find_one(assign_exp<A, B> w1, W... ws) {
    return find_one(mmm(w1), ws...);
  }

  template <typename W>
  bool exists(W&& cond)
  {
    std::ostringstream ss;
    O o;
    ss << "SELECT count(*) FROM " << schema_.table_name();
    where_clause(cond, ss);
    ss << "LIMIT 1";

    auto stmt = con_.prepare(ss.str());

    return li::tuple_reduce(metamap_values(cond), stmt).template read<int>();
  }

  template <typename A, typename B, typename... W>
  auto exists(assign_exp<A, B> w1, W... ws) {
    return exists(mmm(w1, ws...));
  }
  // Save a ll fields except auto increment.
  // The db will automatically fill auto increment keys.
  template <typename N, typename... A> long long int insert(N&& o, A&&... cb_args) {
    std::ostringstream ss;
    std::ostringstream vs;

    auto values = schema_.without_auto_increment();
    map(o, [&](auto k, auto& v) { values[k] = o[k]; });
    // auto values = intersection(o, schema_.without_auto_increment());
    
    call_callback(s::validate, values, cb_args...);
    call_callback(s::before_insert, values, cb_args...);
    ss << "INSERT into " << schema_.table_name() << "(";

    bool first = true;
    li::map(values, [&](auto k, auto v) {
      if (!first) {
        ss << ",";
        vs << ",";
      }
      first = false;
      ss << li::symbol_string(k);
      vs << "?";
    });


    ss << ") VALUES (" << vs.str() << ")";
    auto req = con_.prepare(ss.str());
    li::reduce(values, req);

    call_callback(s::after_insert, o, cb_args...);

    return req.last_insert_id();
  };
  template <typename A, typename B, typename... O, typename... W>
  long long int insert(metamap<O...>&& o, assign_exp<A, B> w1, W... ws) {
    return insert(cat(o, mmm(w1)), ws...);
  }
  template <typename A, typename B, typename... W>
  long long int insert(assign_exp<A, B> w1, W... ws) {
    return insert(mmm(w1), ws...);
  }

  // template <typename S, typename V, typename... A>
  // long long int insert(const assign_exp<S, V>& a, A&&... tail) {
  //   auto m = mmm(a, tail...);
  //   return insert(m);
  // }

  // Iterate on all the rows of the table.
  template <typename F> void forall(F f) {
    std::ostringstream ss;
    ss << "SELECT * from " << schema_.table_name();
    con_(ss.str()).map([&](decltype(schema_.all_fields()) o) { f(o); });
  }

  // Update N's members except auto increment members.
  // N must have at least one primary key.
  template <typename N, typename... CB> void update(const N& o, CB&&... args) {
    // check if N has at least one member of PKS.

    call_callback(s::validate, o, args...);
    call_callback(s::write_access, o, args...);
    call_callback(s::before_update, o, args...);

    // static_assert(metamap_size<decltype(intersect(o, schema_.read_only()))>(),
    //"You cannot give read only fields to the orm update method.");

    auto pk = intersection(o, schema_.primary_key());
    static_assert(metamap_size<decltype(pk)>() > 0,
                  "You must provide at least one primary key to update an object.");
    std::ostringstream ss;
    ss << "UPDATE " << schema_.table_name() << " SET ";

    bool first = true;
    auto to_update = substract(o, schema_.read_only());

    map(to_update, [&](auto k, auto v) {
      if (!first)
        ss << ",";
      first = false;
      ss << li::symbol_string(k) << " = ?";
    });

    where_clause(pk, ss);

    auto stmt = con_.prepare(ss.str());
    li::tuple_reduce(std::tuple_cat(metamap_values(to_update), metamap_values(pk)), stmt);

    call_callback(s::after_update, o, args...);
  }

  template <typename A, typename B, typename... O, typename... W>
  void update(metamap<O...>&& o, assign_exp<A, B> w1, W... ws) {
    return update(cat(o, mmm(w1)), ws...);
  }
  template <typename A, typename B, typename... W>
  void update(assign_exp<A, B> w1, W... ws) {
    return update(mmm(w1), ws...);
  }

  inline int count() {
    return con_(std::string("SELECT count(*) from ") + schema_.table_name()).template read<int>();
  }

  template <typename N, typename... CB> void remove(const N& o, CB&&... args) {

    call_callback(s::before_remove, o, args...);

    std::ostringstream ss;
    ss << "DELETE from " << schema_.table_name() << " WHERE ";

    bool first = true;
    map(schema_.primary_key(), [&](auto k, auto v) {
      if (!first)
        ss << " and ";
      first = false;
      ss << li::symbol_string(k) << " = ? ";
    });

    auto pks = intersection(o, schema_.primary_key());
    li::reduce(pks, con_.prepare(ss.str()));

    call_callback(s::after_remove, o, args...);
  }
  template <typename A, typename B, typename... O, typename... W>
  void remove(metamap<O...>&& o, assign_exp<A, B> w1, W... ws) {
    return remove(cat(o, mmm(w1)), ws...);
  }
  template <typename A, typename B, typename... W>
  void remove(assign_exp<A, B> w1, W... ws) {
    return remove(mmm(w1), ws...);
  }

  auto& schema() { return schema_; }

  SCHEMA schema_;
  C con_;
};

template <typename... F> struct orm_fields {

  orm_fields(F... fields) : fields_(fields...) {
    static_assert(sizeof...(F) == 0 || metamap_size<decltype(this->primary_key())>() != 0,
                  "You must give at least one primary key to the ORM. Use "
                  "s::your_field_name(s::primary_key) to add a primary_key");
  }

  // Field extractor.
  template <typename M> auto get_field(M m) { return m; }
  template <typename M, typename T> 
  auto get_field(assign_exp<M, T> e) { return e; }
  template <typename M, typename T, typename... A> 
  auto get_field(assign_exp<function_call_exp<M, A...>, T> e) { return assign_exp<M, T>{M{}, e.right}; }

  // template <typename M> struct get_field { typedef M ret; };
  // template <typename M, typename T> struct get_field<assign_exp<M, T>> {
  //   typedef assign_exp<M, T> ret;
  //   static auto ctor() { return assign_exp<M, T>{M{}, T()}; }
  // };
  // template <typename M, typename T, typename... A>
  // struct get_field<assign_exp<function_call_exp<M, A...>, T>> : public get_field<assign_exp<M, T>> {
  // };

//get_field<E>::ctor();
// field attributes checks.
#define CHECK_FIELD_ATTR(ATTR)                                                                     \
  template <typename M> struct is_##ATTR : std::false_type {};                                     \
  template <typename M, typename T, typename... A>                                                 \
  struct is_##ATTR<assign_exp<function_call_exp<M, A...>, T>>                                      \
      : std::disjunction<std::is_same<std::decay_t<A>, s::ATTR##_t>...> {};                        \
                                                                                                   \
  auto ATTR() {                                                                                    \
    return tuple_map_reduce(fields_,                                                               \
                            [this](auto e) {                                                           \
                              typedef std::remove_reference_t<decltype(e)> E;                      \
                              if constexpr (is_##ATTR<E>::value)                                   \
                                return get_field(e);                                       \
                              else                                                                 \
                                return skip{};                                                     \
                            },                                                                     \
                            make_metamap_skip);                                                    \
  }

  CHECK_FIELD_ATTR(primary_key); 
  CHECK_FIELD_ATTR(read_only);
  CHECK_FIELD_ATTR(auto_increment);
  CHECK_FIELD_ATTR(computed);
#undef CHECK_FIELD_ATTR

  // Do not remove this comment, this is used by the symbol generation.
  // s::primary_key s::read_only s::auto_increment s::computed 

  auto all_info() { return fields_; }

  auto all_fields() {

    return tuple_map_reduce(fields_,
                            [this](auto e) {
                              //typedef std::remove_reference_t<decltype(e)> E;
                              return get_field(e);
                            },
                            [](auto... e) { return mmm(e...); });
  }

  auto without_auto_increment() { return substract(all_fields(), auto_increment()); }
  auto all_fields_except_computed() { return substract(substract(all_fields(), computed()),
                                                       auto_increment()); }

  std::tuple<F...> fields_;
};

template <typename DB, typename MD = orm_fields<>, typename CB = decltype(mmm())>
struct sql_orm_schema : public MD {

  sql_orm_schema(DB& db, const std::string& table_name, CB cb = CB(), MD md = MD())
      : MD(md), database_(db), table_name_(table_name), callbacks_(cb) {}

  inline auto connect() { return sql_orm(*this, database_.connect()); }

  const std::string& table_name() const { return table_name_; }
  auto get_callbacks() const { return callbacks_; }

  template <typename... P> auto callbacks(P... params_list) const {
    auto cbs = mmm(params_list...);
    auto allowed_callbacks = mmm(s::before_insert, s::before_remove, s::before_update,
                                 s::after_insert, s::after_remove, s::after_update, s::validate);

    static_assert(metamap_size<decltype(substract(cbs, allowed_callbacks))>() == 0, 
    "The only supported callbacks are: s::before_insert, s::before_remove, s::before_update,"
    " s::after_insert, s::after_remove, s::after_update, s::validate");
    return sql_orm_schema<DB, MD, decltype(cbs)>(database_, table_name_, cbs, *static_cast<const MD*>(this));
  }

  template <typename... P> auto fields(P... p) const {
    return sql_orm_schema<DB, orm_fields<P...>, CB>(database_, table_name_, callbacks_, orm_fields<P...>(p...));
  }

  DB& database_;
  std::string table_name_;
  CB callbacks_;
};

}; // namespace li
