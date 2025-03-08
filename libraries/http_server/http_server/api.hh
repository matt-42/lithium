#pragma once

#include <functional>
#include <iostream>
#include <li/http_server/dynamic_routing_table.hh>
#include <li/http_server/error.hh>
#include <li/http_server/symbols.hh>
#include <string_view>

namespace li {

template <typename T, typename F> struct delayed_assignator {
  delayed_assignator(T& t, F f = [](T& t, auto u) { t = u; }) : t(t), f(f) {}

  template <typename U> auto operator=(U&& u) { return f(t, u); }

  T& t;
  F& f;
};

template <typename Req, typename Resp> struct api {

  enum { ANY, GET, POST, PUT, HTTP_DELETE };

  typedef api<Req, Resp> self;

  api() : is_global_handler(false), global_handler_(nullptr) {}

  using H = std::function<void(Req&, Resp&)>;
  struct VH {
    int verb = ANY;
    H handler;
    std::string url_spec;
  };

  H& operator()(std::string_view& route) {
    auto& vh = routes_map_[route];
    vh.verb = ANY;
    vh.url_spec = route;
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
  H& delete_(std::string_view r) { return this->operator()(HTTP_DELETE, r); }

  int parse_verb(std::string_view method) const {
    if (method == "GET")
      return GET;
    if (method == "PUT")
      return PUT;
    if (method == "POST")
      return POST;
    if (method == "HTTP_DELETE")
      return HTTP_DELETE;
    return ANY;
  }

  void add_subapi(std::string prefix, const self& subapi) {
    subapi.routes_map_.for_all_routes([this, prefix](auto r, VH h) {
      if (r.empty() || r == "/")
        h.url_spec = prefix;
      else if (r.back() == '/')
        h.url_spec = prefix + r;
      else
        h.url_spec = prefix + '/' + r;

      this->routes_map_[h.url_spec] = h;
    });
  }

  H& global_handler() {
    is_global_handler = true;
    return global_handler_;
  }
  void print_routes() {
    routes_map_.for_all_routes([this](auto r, auto h) { std::cout << r << '\n'; });
    std::cout << std::endl;
  }
  auto call(std::string_view method, std::string_view route, Req& request, Resp& response) const {
    if (is_global_handler) {
      global_handler_(request, response);
      return;
    }
    if (route == last_called_route_) {
      if (last_handler_.verb == ANY or parse_verb(method) == last_handler_.verb) {
        request.url_spec = last_handler_.url_spec;
        last_handler_.handler(request, response);
        return;
      } else
        throw http_error::not_found("Method ", method, " not implemented on route ", route);
    }

    // skip the last / of the url and trim spaces.
    std::string_view route2(route);
    while (route2.size() > 1 and
           (route2[route2.size() - 1] == '/' || route2[route2.size() - 1] == ' '))
      route2 = route2.substr(0, route2.size() - 1);

    auto it = routes_map_.find(route2);
    if (it != routes_map_.end()) {
      if (it->second.verb == ANY or parse_verb(method) == it->second.verb) {
        const_cast<self*>(this)->last_called_route_ = route;
        const_cast<self*>(this)->last_handler_ = it->second;
        request.url_spec = it->second.url_spec;
        it->second.handler(request, response);
      } else
        throw http_error::not_found("Method ", method, " not implemented on route ", route2);
    } else
      throw http_error::not_found("Route ", route2, " does not exist.");
  }

  dynamic_routing_table<VH> routes_map_;
  std::string last_called_route_;
  VH last_handler_;
  H global_handler_;
  bool is_global_handler;
};

} // namespace li
