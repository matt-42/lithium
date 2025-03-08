#pragma once

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace li {

namespace internal {

/**
 * A memory pool for drt_node objects that manages node allocation and lifetime
 */
template <typename T> struct drt_node_pool {
  template <typename... Args> T* allocate(Args&&... args) noexcept {
    auto new_node = std::make_unique<T>(std::forward<Args>(args)...);
    T* ptr = new_node.get();
    pool_.emplace_back(std::move(new_node));
    return ptr;
  }

  std::vector<std::unique_ptr<T>> pool_;
};

template <typename V> struct drt_node {
  drt_node() noexcept : pool_(nullptr), v_{0, nullptr} {}
  drt_node(drt_node_pool<drt_node>& pool) noexcept : pool_(pool), v_{0, nullptr} {}

  struct iterator {
    const drt_node<V>* ptr;
    std::string_view first;
    V second;

    auto operator->() noexcept { return this; }
    bool operator==(const iterator& b) const noexcept { return this->ptr == b.ptr; }
    bool operator!=(const iterator& b) const noexcept { return this->ptr != b.ptr; }
  };

  auto end() const noexcept { return iterator{nullptr, std::string_view(), V()}; }

  auto& find_or_create(std::string_view r, unsigned int c) {
    if (c == r.size())
      return v_;

    if (r[c] == '/')
      c++; // skip the /
    int s = c;
    while (c < r.size() and r[c] != '/')
      c++;
    std::string_view k = r.substr(s, c - s);

    if (children_.find(k) == children_.end()) {
      children_[k] = pool_.allocate(pool_);
    }
    return children_[k]->find_or_create(r, c);
  }

  template <typename F> void for_all_routes(F f, std::string prefix = "") const {
    if (children_.size() == 0)
      f(prefix, v_);
    else {
      if (prefix.size() && prefix.back() != '/')
        prefix += '/';
      for (const auto& kv : children_)
        kv.second->for_all_routes(f, prefix + std::string(kv.first));
    }
  }

  // Find a route.
  iterator find(const std::string_view& r, unsigned int c) const noexcept {
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
      for (const auto& kv : children_) {
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
  drt_node_pool<drt_node>& pool_;
};

template <typename V> struct dynamic_routing_table_data {
  dynamic_routing_table_data() noexcept : root(pool_) {}

  std::unordered_set<std::string> paths;
  drt_node<V> root;

private:
  drt_node_pool<drt_node<V>> pool_;
};
} // namespace internal

/**
 * A dynamic routing table that supports route registration and lookup.
 */
template <typename V> struct dynamic_routing_table {
  dynamic_routing_table() noexcept
      : data_(std::make_shared<internal::dynamic_routing_table_data<V>>()) {}
  dynamic_routing_table(const dynamic_routing_table& other) noexcept : data_(other.data_) {}

  dynamic_routing_table& operator=(const dynamic_routing_table& other) noexcept {
    if (this != &other) {
      data_ = other.data_;
    }
    return *this;
  }

  auto& operator[](const std::string_view& r) { return this->operator[](std::string(r)); }
  auto& operator[](const std::string& s) {
    auto [itr, is_inserted] = data_->paths.emplace(s);
    return data_->root.find_or_create(*itr, 0);
  }
  auto find(const std::string_view& r) const noexcept { return data_->root.find(r, 0); }
  template <typename F> void for_all_routes(F f) const { data_->root.for_all_routes(f); }
  auto end() const noexcept { return data_->root.end(); }

private:
  std::shared_ptr<internal::dynamic_routing_table_data<V>> data_;
};

} // namespace li
