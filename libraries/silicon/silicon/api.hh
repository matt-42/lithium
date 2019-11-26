#include <string_view>
#include <iostream>
#include <functional>
#include <iod/silicon/dynamic_routing_table.hh>
#include <iod/silicon/symbols.hh>
#include <iod/silicon/error.hh>

namespace iod {

enum { ANY, GET, POST, PUT, DELETE };

template <typename T, typename F> struct delayed_assignator {
  delayed_assignator(T& t, F f = [](T& t, auto u) { t = u; })
      : t(t), f(f) {}

  template <typename U> auto operator=(U&& u) { return f(t, u); }

  T& t;
  F& f;
};

template <typename Req, typename Resp> struct api {

  using H = std::function<void(Req&, Resp&)>;
  struct VH {
    int verb = ANY;
    H handler;
  };

  H& operator()(const std::string_view& r) { return routes_map_[r]; }

  H& operator()(int verb, const std::string_view& r) {
    auto& vh = routes_map_[r];
    vh.verb = verb;
    return vh.handler;
  }

  int parse_verb(std::string_view method)
  {
    if (method == "GET") return GET;
    if (method == "PUT") return PUT;
    if (method == "POST") return POST;
    if (method == "DELETE") return DELETE;
    return ANY;
  }

  auto call(std::string_view& method, std::string route, Req* request,
            Resp* response) {
    // skip the last / of the url.
    std::string_view route2(route);
    if (route2.size() != 0 and route2[route2.size() - 1] == '/')
      route2 = route2.substr(0, route2.size() - 1);

    auto it = routes_map_.find(route2);
    if (it != routes_map_.end()) {
      if (it->second->verb == ANY or parse_verb(method) == it->second->verb)
        it->second->handler(request, response);
      else
        throw http_error::not_found("Method ", method, " not implemented on route ",
                               route2);
    } else
      throw http_error::not_found("Route ", route2, " does not exist.");
  }

  dynamic_routing_table<H> routes_map_;
};

} // namespace iod
