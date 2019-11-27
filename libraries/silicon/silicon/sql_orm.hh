#pragma once

#include <iod/metamap/metamap.hh>
#include <iod/silicon/symbols.hh>
#include <iostream>
#include <sstream>

namespace iod {

struct sqlite_connection;
struct mysql_connection;

using s::auto_increment;
using s::primary_key;
using s::read_only;

template <typename SCHEMA, typename C> struct sql_orm {

  typedef decltype(std::declval<SCHEMA>().all_fields()) O;

  sql_orm(SCHEMA& schema, C& con) : schema_(schema), con_(con) {}

  template <typename S, typename O> void call_callback(S s, O& o) {
    get_or(schema_.get_callbacks(), s, [](auto p) {})(o);
  }

  void create_table_if_not_exists() {
    std::stringstream ss;
    ss << "CREATE TABLE if not exists " << schema_.table_name() << " (";

    bool first = true;
    iod::tuple_map(schema_.all_info(), [&](auto f) {
      typedef decltype(f) F;
      typedef typename SCHEMA::template get_field<F>::ret A;
      typedef typename A::left_t K;
      typedef typename A::right_t V;

      bool auto_increment = SCHEMA::template is_auto_increment<F>::value;
      bool primary_key = SCHEMA::template is_primary_key<F>::value;
      K k;
      V v;

      if (!first)
        ss << ", ";
      ss << iod::symbol_string(k) << " " << con_.type_to_string(v);

      if (std::is_same<C, sqlite_connection>::value) {
        if (auto_increment or primary_key)
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
  }

  auto find_by_id(int id) {
    O o;
    std::stringstream field_ss;
    bool first = true;
    iod::map(o, [&](auto k, auto v) {
      if (!first)
        field_ss << ",";
      first = false;
      field_ss << iod::symbol_string(k);
    });

    int res =
        con_("SELECT " + field_ss.str() + " from " + schema_.table_name() + " where id = ?")(id) >>
        o;
    call_callback(s::read_access, o);
    return o;
  }

  template <typename... W>
  auto select_one_where(W... where_conditions) {
    auto where = make_metamap(where_conditions...);
    std::stringstream ss;
    O o;
    ss << "SELECT";
    bool first = true;
    iod::map(o, [&](auto k, auto v) {
      if (!first)
        ss << ",";
      first = false;
      ss << iod::symbol_string(k);
    });

    ss << "WHERE";

    first = true;
    map(where, [&](auto k, auto v) {
      if (!first)
        ss << " and ";
      first = false;
      ss << iod::symbol_string(k) << " = ? ";
    });
    ss << "LIMIT 1";
    auto stmt = con_(ss.str());

    iod::reduce(where, stmt) >> o;
    call_callback(s::read_access, o);
    return o;
  }

  // Save all fields except auto increment.
  // The db will automatically fill auto increment keys.
  template <typename N> int insert(const N& o) {
    std::stringstream ss;
    std::stringstream vs;

    call_callback(s::validate, o);
    call_callback(s::before_insert, o);
    call_callback(s::write_access, o);
    ss << "INSERT into " << schema_.table_name() << "(";

    bool first = true;
    iod::map(schema_.without_auto_increment(), [&](auto k, auto v) {
      if (!first) {
        ss << ",";
        vs << ",";
      }
      first = false;
      ss << iod::symbol_string(k);
      vs << "?";
    });

    auto values = intersection(schema_.without_auto_increment(), o);
    map(values, [&](auto k, auto& v) { v = o[k]; });
    // auto values = intersection(o, schema_.without_auto_increment());

    ss << ") VALUES (" << vs.str() << ")";
    auto req = con_(ss.str());
    iod::reduce(values, req);

    call_callback(s::after_insert, o);

    return req.last_insert_id();
  };
  template <typename S, typename V, typename... A>
  int insert(const assign_exp<S, V>& a, A&&... tail) {
    return insert(make_metamap(a, tail...));
  }

  // Iterate on all the rows of the table.
  template <typename F> void forall(F f) {
    std::stringstream ss;
    ss << "SELECT * from " << schema_.table_name();
    con_(ss.str())() | [&](decltype(schema_.all_fields()) o) { f(o); };
  }

  // Update N's members except auto increment members.
  // N must have at least one primary key.
  template <typename N> int update(const N& o) {
    // check if N has at least one member of PKS.

    call_callback(s::validate, o);
    call_callback(s::write_access, o);
    call_callback(s::before_update, o);

    //static_assert(metamap_size<decltype(intersect(o, schema_.read_only()))>(),
    //"You cannot give read only fields to the orm update method.");

    auto pk = intersection(o, schema_.primary_key());
    static_assert(metamap_size<decltype(pk)>() > 0,
                  "You must provide at least one primary key to update an object.");
    std::stringstream ss;
    ss << "UPDATE " << schema_.table_name() << " SET ";

    bool first = true;
    auto to_update = substract(o, schema_.read_only());

    map(to_update, [&](auto k, auto v) {
      if (!first)
        ss << ",";
      first = false;
      ss << iod::symbol_string(k) << " = ?";
    });

    ss << " WHERE ";

    first = true;
    map(pk, [&](auto k, auto v) {
      if (!first)
        ss << " and ";
      first = false;
      ss << iod::symbol_string(k) << " = ? ";
      return k = v;
    });

    auto stmt = con_(ss.str());
    iod::tuple_reduce(std::tuple_cat(metamap_values(to_update), metamap_values(pk)), stmt);

    call_callback(s::after_update, o);

    return stmt.last_insert_id();
  }

  inline int count () {
    int c;
    con_(std::string("SELECT count(*) from ") + schema_.table_name()) >> c;
    return c;
  }

  template <typename T> void remove(const T& o) {

    call_callback(s::write_access, o);
    call_callback(s::before_destroy, o);

    std::stringstream ss;
    ss << "DELETE from " << schema_.table_name() << " WHERE ";

    bool first = true;
    map(schema_.primary_key(), [&](auto k, auto v) {
      if (!first)
        ss << " and ";
      first = false;
      ss << iod::symbol_string(k) << " = ? ";
    });

    auto pks = intersection(o, schema_.primary_key());
    iod::reduce(pks, con_(ss.str()));

    call_callback(s::after_destroy, o);
  }

  SCHEMA schema_;
  C con_;
};

template <typename... F> struct orm_fields {

  orm_fields(F... fields) : fields_(fields...) {
    static_assert(sizeof...(F) == 0 or
                      metamap_size<decltype(this->primary_key())>() != 0,
                  "You must give at least one primary key to the ORM. Use "
                  "s::your_field_name(s::primary_key) to add a primary_key");
  }

  // Field extractor.
  template <typename M> struct get_field { typedef M ret; };
  template <typename M, typename T> struct get_field<assign_exp<M, T>> {
    typedef assign_exp<M, T> ret;
    static auto ctor() { return assign_exp<M, T>{M{}, T()}; }
  };
  template <typename M, typename T, typename... A>
  struct get_field<assign_exp<function_call_exp<M, A...>, T>> : public get_field<assign_exp<M, T>> {
  };

// field attributes checks.
#define CHECK_FIELD_ATTR(ATTR)                                                                     \
  template <typename M> struct is_##ATTR : std::false_type {};                                     \
  template <typename M, typename T, typename... A>                                                 \
  struct is_##ATTR<assign_exp<function_call_exp<M, A...>, T>>                                      \
      : std::disjunction<std::is_same<std::decay_t<A>, s::ATTR##_t>...> {};                        \
                                                                                                   \
  auto ATTR() {                                                                                    \
    return tuple_map_reduce(fields_,                                                               \
                            [](auto e) {                                                           \
                              typedef std::remove_reference_t<decltype(e)> E;                      \
                              if constexpr (is_##ATTR<E>::value)                                   \
                                return get_field<E>::ctor();                                       \
                              else                                                                 \
                                return skip{};                                                     \
                            },                                                                     \
                            make_metamap_skip);                                                    \
  }

  CHECK_FIELD_ATTR(primary_key);
  CHECK_FIELD_ATTR(read_only);
  CHECK_FIELD_ATTR(auto_increment);

  auto all_info() { return fields_; }

  auto all_fields() {

    return tuple_map_reduce(fields_,
                            [](auto e) {
                              typedef std::remove_reference_t<decltype(e)> E;
                              return get_field<E>::ctor();
                            },
                            [](auto... e) { return make_metamap(e...); });
  }

  auto without_auto_increment() { return substract(all_fields(), auto_increment()); }

  std::tuple<F...> fields_;
};

template <typename MD = orm_fields<>, typename CB = decltype(make_metamap())>
struct sql_orm_shema : public MD {

  sql_orm_shema(const std::string& table_name, CB cb = CB(), MD md = MD())
      : MD(md), table_name_(table_name), callbacks_(cb) {}

  template <typename D> auto connect(D& db) { return sql_orm(*this, db.get_connection()); }

  const std::string& table_name() const { return table_name_; }
  auto get_callbacks() const { return callbacks_; }

  template <typename... P> auto callbacks(P... params_list) {
    auto cbs = make_metamap(params_list...);
    return sql_orm_shema<MD, decltype(cbs)>(table_name_, cbs, *static_cast<MD*>(this));
  }

  template <typename... P> auto fields(P... p) {
    return sql_orm_shema<orm_fields<P...>, CB>(table_name_, callbacks_, orm_fields<P...>(p...));
  }

  std::string table_name_;
  CB callbacks_;
};

}; // namespace iod
