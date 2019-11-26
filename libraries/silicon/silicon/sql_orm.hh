#pragma once

#include <iod/metamap/metamap.hh>
#include <iod/silicon/symbols.hh>
#include <iostream>
#include <sstream>

namespace iod {

using s::auto_increment;
using s::primary_key;
using s::read_only;

template <typename ORM> struct sql_orm_connection {
  typedef ORM orm_type;
  typedef decltype(std::declval<ORM>().metadata_.all_fields()) O;

  sql_orm_connection(ORM& orm) : orm_(orm), con_(orm.database_.get_connection()) {}

  int find_by_id(int id, O& o) {
    std::stringstream field_ss;
    bool first = true;
    iod::map(o, [&](auto k, auto v) {
      if (!first)
        field_ss << ",";
      first = false;
      field_ss << iod::symbol_string(k);
    });

    return con_("SELECT " + field_ss.str() + " from " + orm_.table_name_ + " where id = ?")(id) >> o;
  }

  // save all fields except auto increment.
  // The db will automatically fill auto increment keys.
  template <typename N> int insert(const N& o) {
    std::stringstream ss;
    std::stringstream vs;
    ss << "INSERT into " << orm_.table_name_ << "(";

    bool first = true;
    iod::map(orm_.metadata_.without_auto_increment(),
                                  [&](auto k, auto v) {
                                    if (!first) {
                                      ss << ",";
                                      vs << ",";
                                    }
                                    first = false;
                                    ss << iod::symbol_string(k);
                                    vs << "?";
                                  });

    auto values = intersection(orm_.metadata_.without_auto_increment(), o);
    map(values, [&] (auto k, auto& v) { v = o[k]; });
    //auto values = intersection(o, orm_.metadata_.without_auto_increment());

    ss << ") VALUES (" << vs.str() << ")";
    auto req = con_(ss.str());
    iod::reduce(values, req);
    return req.last_insert_id();
  };
  template <typename S, typename V, typename... A> int insert(const assign_exp<S, V>& a, A&&... tail) {
    return insert(make_metamap(a, tail...));
  }

  // Iterate on all the rows of the table.
  template <typename F> void forall(F f) {
    std::stringstream ss;
    ss << "SELECT * from " << orm_.table_name_;
    con_(ss.str())() | [&] (decltype(orm_.metadata_.all_fields()) o) { f(o); };
  }

  // Update N's members except auto increment members.
  // N must have at least one primary key.
  template <typename N> int update(const N& o) {
    // check if N has at least one member of PKS.
    auto pk = intersect(o, orm_.metadata.primary_keys);
    static_assert(decltype(pk)::size() > 0, "You must provide at least one primary key to update an object.");
    std::stringstream ss;
    ss << "UPDATE " << orm_.table_name_ << " SET ";

    bool first = true;
    auto values = map(o, [&](auto k, auto v) {
      if (!first)
        ss << ",";
      first = false;
      ss << iod::symbol_string(k) << " = ?";
      return k = v;
    });

    ss << " WHERE ";

    first = true;
    auto pks_values = map(pk, [&](auto k, auto v) {
      if (!first)
        ss << " and ";
      first = false;
      ss << iod::symbol_string(k) << " = ? ";
      return k = v;
    });

    auto stmt = con_(ss.str());
    iod::reduce(cat(values, pks_values), stmt);
    //apply(values, pks_values, stmt);
    return stmt.last_insert_id();
  }

  template <typename T> void destroy(const T& o) {
    std::stringstream ss;
    ss << "DELETE from " << orm_.table_name() << " WHERE ";

    bool first = true;
    auto values = map(orm_.metadata.primary_keys, [&](auto k, auto v) {
      if (!first)
        ss << " and ";
      first = false;
      ss << iod::symbol_string(k) << " = ? ";
      return k = o[k];
    });

    iod::reduce(values, con_(ss.str()));
  }

  ORM orm_;
  typename ORM::database_type::connection_type con_;
};

struct sqlite_connection;
struct mysql_connection;


template <typename... F>
struct orm_fields {

  orm_fields(F... fields) : fields_(fields...)
  {
    //static_assert(std::tuple_size<decltype(this->primary_key())>::value != 0, 
    //"You must give at least one primary key to the ORM. Use s::your_field_name(s::primary_key) to add a primary_key");
  }

  // Field extractor.
  template <typename M> struct get_field { typedef M ret; };
  template <typename M, typename T> struct get_field<assign_exp<M, T>> {
    typedef assign_exp<M, T> ret;
    static auto ctor() { return assign_exp<M, T>{M{}, T()}; }
  };
  template <typename M, typename T, typename... A> 
  struct get_field<assign_exp<function_call_exp<M, A...>, T>> : public get_field<assign_exp<M, T>> {};

  // field attributes checks.
  #define CHECK_FIELD_ATTR(ATTR)                                                                                         \
    template <typename M> struct is_##ATTR : std::false_type{};                                                                 \
    template <typename M, typename T, typename... A>                                                                     \
    struct is_##ATTR<assign_exp<function_call_exp<M, A...>, T>> : std::disjunction<std::is_same<std::decay_t<A>, s::ATTR##_t>...> {};\
    \
    auto ATTR() {\
      return tuple_map_reduce(fields_, [] (auto e) {\
        typedef std::remove_reference_t<decltype(e)> E;\
        if constexpr (is_##ATTR<E>::value)\
          return get_field<E>::ctor();\
        else\
          return skip{};\
      }, make_metamap_skip);\
    }

  CHECK_FIELD_ATTR(primary_key);
  CHECK_FIELD_ATTR(read_only);
  CHECK_FIELD_ATTR(auto_increment);

  auto all_info() { return fields_; }

  auto all_fields() {

    auto fs = tuple_map_reduce(fields_, [] (auto e) {
        typedef std::remove_reference_t<decltype(e)> E;\
        return get_field<E>::ctor();
    }, [] (auto... e) { return make_metamap(e...); });

    //void* x = fs;
    //void* x = auto_increment();
    return fs;
  }
  auto without_auto_increment() {
    //return all_fields();

    return substract(all_fields(), auto_increment());
  }

  std::tuple<F...> fields_;
};

template <typename... P> auto make_orm_callbacks(P... params_list) {

  auto params = make_metamap(params_list...);
  return make_metamap(s::read_access = get_or(params, s::read_access, [](auto) {}),
                          s::write_access = get_or(params, s::write_access, [](auto) {}),
                          s::validate = get_or(params, s::validate, [](auto) {}),
                          s::on_create_success = get_or(params, s::on_create_success, [](auto) {}),
                          s::on_destroy_success = get_or(params, s::on_destroy_success, [](auto) {}),
                          s::on_update_success = get_or(params, s::on_update_success, [](auto) {}),
                          s::before_update = get_or(params, s::before_update, [](auto) {}),
                          s::before_create = get_or(params, s::before_create, [](auto) {}));
}

template <typename D, typename MD = orm_fields<>, typename CB = decltype(make_metamap())> struct sql_orm {

  typedef sql_orm<D, MD, CB> self;

  typedef D database_type;
  typedef decltype(std::declval<MD>().all_fields()) object_type;
  typedef sql_orm_connection<self> connection_type;
  friend connection_type;  

  sql_orm(D& database, const std::string& table_name, MD md = MD(), CB cb = CB())
      : database_(database), table_name_(table_name), metadata_(md) {}

  void create_table_if_not_exists() {
    std::stringstream ss;
    ss << "CREATE TABLE if not exists " << table_name_ << " (";

    auto c = database_.get_connection();
    typedef std::decay_t<decltype(c)> C;

    bool first = true;
    iod::tuple_map(metadata_.all_info(), [&](auto f) {

      typedef decltype(f) F;
      typedef typename MD::template get_field<F>::ret A;
      typedef typename A::left_t K;
      typedef typename A::right_t V;

      
      bool auto_increment =  MD::template is_auto_increment<F>::value;
      bool primary_key = MD::template is_primary_key<F>::value;
      K k;
      V v;


      if (!first)
        ss << ", ";
      ss << iod::symbol_string(k) << " " << c.type_to_string(v);

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
      c(ss.str())();
    } catch (std::exception e) {
      std::cerr << "Warning: Silicon could not create the " << table_name_ << " sql table." << std::endl
                << "You can ignore this message if the table already exists."
                << "The sql error is: " << e.what() << std::endl;
    }
  }

  auto get_connection() { 
    return connection_type(*this); };

  template <typename... P> auto callbacks(P... params_list) {

    auto params = make_metamap(params_list...);
    auto cbs = make_metamap(s::read_access = get_or(params, s::read_access, [](auto) {}),
                            s::write_access = get_or(params, s::write_access, [](auto) {}),
                            s::validate = get_or(params, s::validate, [](auto) {}),
                            s::on_create_success = get_or(params, s::on_create_success, [](auto) {}),
                            s::on_destroy_success = get_or(params, s::on_destroy_success, [](auto) {}),
                            s::on_update_success = get_or(params, s::on_update_success, [](auto) {}),
                            s::before_update = get_or(params, s::before_update, [](auto) {}),
                            s::before_create = get_or(params, s::before_create, [](auto) {}));

    return sql_orm(database_, table_name_, metadata_, cbs);
  }

  template <typename... P> auto fields(P... p) {

    return sql_orm<D, orm_fields<P...>, CB>(database_, table_name_, orm_fields<P...>(p...), callbacks_);
  }

  D& database_;
  std::string table_name_;

  MD metadata_;
  CB callbacks_;
};

}; // namespace iod
