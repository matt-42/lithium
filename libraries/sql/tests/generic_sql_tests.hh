#pragma once

#include <iostream>

#define EXPECT_THROW(STM)                                                                          \
  try {                                                                                            \
    STM;                                                                                           \
    std::cerr << "This must have thrown an exception: " << #STM << std::endl;                      \
    assert(0);                                                                                     \
  } catch (...) {                                                                                  \
  }

#define EXPECT_EQUAL(A, B)                                                                         \
  if ((A) != (B)) {                                                                                \
    std::cerr << #A << " (== " << (A) << ") "                                                      \
              << " != " << #B << " (== " << (B) << ") " << std::endl;                              \
    assert(0);                                                                                     \
  }

#define EXPECT(A)                                                                                  \
  if (!(A)) {                                                                                      \
    std::cerr << #A << " (== " << (A) << ") must be true" << std::endl;                            \
    assert(0);                                                                                     \
  }

typename<typename C> void init_test_table(C&& query) {
  query("DROP table if exists users_test;");
  query("CREATE TABLE users_test (id int,name TEXT,age int);");
}
template <typename R> void test_result(R&& query) {


  // Invalid queries must throw.
  EXPECT_THROW(query("SELECTT 1+2").read<int>());

  //   long long int affected_rows();
  //   template <typename T> T read();
  EXPECT_EQUAL(3, query("SELECT 1+2").read<int>());
  EXPECT_EQUAL(query("SELECT 'xxx'").read<std::string>(), "xxx");
  // Reading empty sets must throw.
  EXPECT_THROW(query("SELECT * from users_test;").read<int>());
  EXPECT_THROW(query("SELECT 'xxx';").read<int>());

  //   template <typename T> void read(T& out);
  int i = 0;
  query("SELECT 1+2").read(i);
  EXPECT_EQUAL(i, 3);

  std::string str;
  query("SELECT 'xxx'").read(str);
  EXPECT_EQUAL(str, "xxx");

  std::tuple<int, std::string, int> tp;
  query("SELECT 1,'xxx',3 ").read(tp);
  EXPECT_EQUAL(std::get<0>(tp), 1);
  EXPECT_EQUAL(std::get<1>(tp), "xxx");
  EXPECT_EQUAL(std::get<2>(tp), 3);

  int i1 = 0;
  int i2 = 0;
  str = "";
  query("SELECT 1,'xxx',3 ").read(i1, str, i2);
  EXPECT_EQUAL(i1, 1);
  EXPECT_EQUAL(str, "xxx");
  EXPECT_EQUAL(i2, 3);

  //   template <typename T> std::optional<T> read_optional();
  init_test_table(query);
  EXPECT(!query("SELECT * from users_test;").read_optional<int>().has_value());
  EXPECT_EQUAL(3, query("SELECT 1+2").read_optional<int>().value());

  //   template <typename T> void read(std::optional<T>& o);
  std::optional<int> opt;
  query("SELECT 1+2").read(opt);
  EXPECT_EQUAL(opt.value(), 3);

  //   bool empty();
  EXPECT(query("SELECT * from users_test;").empty());

  //   long long int last_insert_id();
  EXPECT_EQUAL(
      1, query("INSERT into users_test_mysql(id, name, age) values (1,'a',41);").last_insert_id());
  EXPECT_EQUAL(0, query("SELECT count(*) from users_test;").read<int>());

  EXPECT_EQUAL(
      2, query("INSERT into users_test_mysql(id, name, age) values (2,'b',42);").last_insert_id());
  EXPECT_EQUAL(
      3, query("INSERT into users_test_mysql(id, name, age) values (3,'c',43);").last_insert_id());

  EXPECT_EQUAL(3, query("SELECT count(*) from users_test").read<int>());

  //   template <typename F> void map(F f);
  index = 1;
  query("SELECT id, name, age from users_test;").map([&](int id, std::string name, int age) {
    EXPECT_EQUAL(id, index);
    EXPECT_EQUAL(age, index + 40);
    EXPECT_EQUAL(name.size(), 1);
    EXPECT_EQUAL(name[0], 'a' + index - 1);
    index++;
  });
}

template <typename Q> void test_long_strings_prepared_statement(Q& query) {
  query("DELETE from users_test_mysql");
  std::string test_str_patern = "lkjlkjdgfad0875g9f658g8w97f32orjw0r89ofuhq07fy0rjgq3478fyqh03g7y0b"
                                "347fyj08yg034f78yj047yh078fy0fyj40";
  std::string test_str = "";
  for (int k = 0; k < 10; k++) {
    test_str += test_str_patern;
    insert_user(k, test_str, 42);
  }

  i = 1;
  query.prepare("Select id,name from users_test_mysql;")().map([&](int id, std::string name) {
    EXPECT_EQUAL(name.size() == 100 * i);
    i++;
  });
}

template <typename D> std::string placeholder(int pos) {
  if constexpr (std::same_v<D::db_tag, pgsql_tag>) {
    std::stringstream ss;
    ss << "$" << pos;
    return ss.str();
  } else
    return "?";
}

template <typename D> void test_sql(D& database) {

  auto con = database.connect();

  // test non prepared statement result.
  test_result(query);
  // test prepared statement result.
  test_result([&](std::string q) { return query.prepare(q)(); });

  test_long_strings_prepared_statement(con);

  // Prepared statement.
  auto insert_user =
      con.prepare(std::string("INSERT into users_test_mysql(id, name, age) values (") +
                  placeholder<D>(1) + "," + placeholder<D>(2) + "," + placeholder<D>(3) + ");");

  init_test_table();

  insert_user(1, "John", 42);
  insert_user(2, "Bob", 24);
  EXPECT_EQUAL("")
}