#include <tuple>

namespace iod {


  template <typename... E, typename F>
  void apply_each(F&& f, E&&... e)
  {
    (void)std::initializer_list<int>{
      ((void)f(std::forward<E>(e)), 0)...};
  }

  template <typename... E, typename F, typename R>
  auto tuple_map_reduce_impl(F&& f, R&& reduce, E&&... e)
  {
    return reduce(f(std::forward<E>(e))...);
  }

  template <typename T, typename F>
  void tuple_map(T&& t, F&& f)
  {
    return std::apply([&] (auto&&... e) { apply_each(f, std::forward<decltype(e)>(e)...); },
                                    std::forward<T>(t));
  }

  template <typename T, typename F>
  auto tuple_reduce(T&& t, F&& f)
  {
    return std::apply(std::forward<F>(f), std::forward<T>(t));
  }

  template <typename T, typename F, typename R>
  decltype(auto) tuple_map_reduce(T&& m, F map, R reduce)
  {
    auto fun = [&] (auto... e) {
      return tuple_map_reduce_impl(map, reduce, e...);
    };
    return std::apply(fun, m);
  }

  template <typename F>
  inline std::tuple<> tuple_filter_impl() { return std::make_tuple(); }

  template <typename F, typename... M, typename M1>
  auto tuple_filter_impl(M1 m1, M... m) {
    if constexpr (std::is_same<M1, F>::value)
      return tuple_filter_impl<F>(m...);
    else
      return std::tuple_cat(std::make_tuple(m1), tuple_filter_impl<F>(m...));
  }

  template <typename F, typename... M>
  auto tuple_filter(const std::tuple<M...>& m) {

    auto fun = [] (auto... e) { return tuple_filter_impl<F>(e...); };
    return std::apply(fun, m);
  
  }
  
}