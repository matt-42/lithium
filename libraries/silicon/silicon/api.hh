#include <string_view>
#include <iostream>
#include <functional>
#include <iod/silicon/dynamic_routing_table.hh>
#include <iod/silicon/symbols.hh>
#include <iod/silicon/error.hh>

namespace iod {

enum { ANY, GET, POST, PUT, HTTP_DELETE };

template <typename T, typename F> struct delayed_assignator {
  delayed_assignator(T& t, F f = [](T& t, auto u) { t = u; })
      : t(t), f(f) {}

  template <typename U> auto operator=(U&& u) { return f(t, u); }

  T& t;
  F& f;
};

template <typename Req, typename Resp> struct api {

  typedef api<Req, Resp> self;

  using H = std::function<void(Req&, Resp&)>;
  struct VH {
    int verb = ANY;
    H handler;
    std::string url_spec;
  };

  H& operator()(const std::string_view& r) 
  { 
    auto& vh = routes_map_[r];
    vh.verb = ANY;
    vh.url_spec = r;
    return vh.handler;
  }

  H& operator()(int verb, std::string_view r) {
    auto& vh = routes_map_[r];
    vh.verb = verb;
    vh.url_spec = r;
    return vh.handler;
  }
  H& get(std::string_view r) { return this->operator()(GET, r); }
  H& post(std::string_view r) { return this->operator()(POST, r); }
  H& put(std::string_view r) { return this->operator()(PUT, r); }
  H& delete_(std::string_view r) { return this->operator()(HTTP_DELETE , r); }

  int parse_verb(std::string_view method)
  {
    if (method == "GET") return GET;
    if (method == "PUT") return PUT;
    if (method == "POST") return POST;
    if (method == "HTTP_DELETE") return HTTP_DELETE;
    return ANY;
  }

  void add_subapi(std::string prefix, const self& subapi)
  {
    subapi.routes_map_.for_all_routes([this, prefix] (auto r, auto h) {
      if (r.back() == '/')
        this->routes_map_[prefix + r] = h;
      else
        this->routes_map_[prefix + "/" + r] = h;
    });

  }

  void print_routes()
  {
    std::cout << "=====================" << std::endl;
    routes_map_.for_all_routes([this] (auto r, auto h) {
      std::cout << r << std::endl;
    });
    std::cout << "=====================" << std::endl;
  }
  auto call(const char* method, std::string route, Req& request,
            Resp& response) {
    // skip the last / of the url.
    std::string_view route2(route);
    if (route2.size() != 0 and route2[route2.size() - 1] == '/')
      route2 = route2.substr(0, route2.size() - 1);

    auto it = routes_map_.find(route2);
    if (it != routes_map_.end()) {
      if (it->second.verb == ANY or parse_verb(method) == it->second.verb)
      {
        request.url_spec = it->second.url_spec;
        it->second.handler(request, response);
      }
      else
        throw http_error::not_found("Method ", method, " not implemented on route ",
                               route2);
    } else
      throw http_error::not_found("Route ", route2, " does not exist.");
  }

  dynamic_routing_table<VH> routes_map_;
};

} // namespace iod
