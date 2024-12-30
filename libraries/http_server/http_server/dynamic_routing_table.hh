#pragma once

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace li {

namespace internal {

template <typename V> struct drt_node {

  drt_node() : v_{0, nullptr} {}

  struct iterator {
    const drt_node<V>* ptr;
    std::string_view first;
    V second;

    auto operator->() { return this; }
    bool operator==(const iterator& b) const { return this->ptr == b.ptr; }
    bool operator!=(const iterator& b) const { return this->ptr != b.ptr; }
  };

  auto end() const { return iterator{nullptr, std::string_view(), V()}; }

  auto& find_or_create(std::string_view r, unsigned int c) {
    if (c == r.size())
      return v_;

    if (r[c] == '/')
      c++; // skip the /
    int s = c;
    while (c < r.size() and r[c] != '/')
      c++;
    std::string_view k = r.substr(s, c - s);

    auto it = children_.find(k);
    if (it != children_.end())
      return children_[k]->find_or_create(r, c);
    else {
      auto new_node = std::make_shared<drt_node>();
      children_shared_pointers_.push_back(new_node);
      children_.insert({k, new_node.get()});
      return new_node->find_or_create(r, c);
    }

    return v_;
  }

  template <typename F> void for_all_routes(F f, std::string prefix = "") const {
    if (children_.size() == 0)
      f(prefix, v_);
    else {
      if (prefix.size() && prefix.back() != '/')
        prefix += '/';
      for (auto pair : children_)
        pair.second->for_all_routes(f, prefix + std::string(pair.first));
    }
  }

  // Find a route.
  iterator find(const std::string_view& r, unsigned int c) const {
    // We found the route r.
    if ((c == r.size() and v_.handler != nullptr) or (children_.size() == 0))
      return iterator{this, r, v_};

    // r does not match any route.
    if (c == r.size() and v_.handler == nullptr)
      return iterator{nullptr, r, v_};

    if (r[c] == '/')
      c++; // skip the first /

    // Find the next /.
    int url_part_start = c;
    while (c < r.size() and r[c] != '/')
      c++;

    // k is the string between the 2 /.
    std::string_view k;
    if (url_part_start < r.size() && url_part_start != c)
      k = std::string_view(&r[url_part_start], c - url_part_start);

    // look for k in the children.
    auto it = children_.find(k);
    if (it != children_.end()) {
      auto it2 = it->second->find(r, c); // search in the corresponding child.
      if (it2 != it->second->end())
        return it2;
    }

    {
      // if one child is a url param {{param_name}}, choose it
      for (auto& kv : children_) {
        auto name = kv.first;
        if (name.size() > 4 and name[0] == '{' and name[1] == '{' and
            name[name.size() - 2] == '}' and name[name.size() - 1] == '}')
          return kv.second->find(r, c);
      }
      return end();
    }
  }

  V v_;
  std::unordered_map<std::string_view, drt_node*> children_;
  std::vector<std::shared_ptr<drt_node>> children_shared_pointers_;
};
} // namespace internal

template <typename V> struct dynamic_routing_table {

  // Find a route and return reference to a procedure.
  auto& operator[](const std::string_view& r) {
    auto [itr, is_inserted] = strings.emplace(std::string(r));
    std::string_view r2(*itr);
    return root.find_or_create(r2, 0);
  }
  auto& operator[](const std::string& s) {
    auto [itr, is_inserted] = strings.emplace(s);
    std::string_view r(*itr);
    return root.find_or_create(r, 0);
  }

  // Find a route and return an iterator.
  auto find(const std::string_view& r) const { return root.find(r, 0); }

  template <typename F> void for_all_routes(F f) const { root.for_all_routes(f); }
  auto end() const { return root.end(); }

  std::unordered_set<std::string> strings;
  internal::drt_node<V> root;
};

} // namespace li
