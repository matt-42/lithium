#pragma once

#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

namespace li {

  namespace internal {

    /**
     * A contiguous arena allocator for drt_node objects.
     * Allocates nodes in chunks of kChunkSize for cache-friendly traversal
     * and reduced per-node allocation overhead.
     */
    template <typename T> struct drt_node_pool {
      static constexpr std::size_t kChunkSize = 64;

      drt_node_pool() noexcept = default;
      drt_node_pool(const drt_node_pool&) = delete;
      drt_node_pool& operator=(const drt_node_pool&) = delete;

      ~drt_node_pool() {
        for (auto& chunk : chunks_) {
          // Destroy only the constructed nodes in each chunk
          std::size_t count = (&chunk == &chunks_.back()) ? pos_ : kChunkSize;
          for (std::size_t i = 0; i < count; i++)
            chunk[i].~T();
          ::operator delete(static_cast<void*>(chunk.get()));
        }
      }

      template <typename... Args> T* allocate(Args&&... args) noexcept {
        if (chunks_.empty() || pos_ == kChunkSize) {
          // Allocate a new chunk of raw memory
          void* mem = ::operator new(sizeof(T) * kChunkSize, std::nothrow);
          chunks_.emplace_back(static_cast<T*>(mem));
          pos_ = 0;
        }
        T* slot = chunks_.back().get() + pos_++;
        return ::new (static_cast<void*>(slot)) T(std::forward<Args>(args)...);
      }

    private:
      struct chunk_deleter {
        void operator()(T*) const noexcept {} // destructor handles cleanup
        T* get() const noexcept { return ptr_; }
        T* ptr_ = nullptr;
      };

      // Lightweight owning wrapper that doesn't auto-delete (arena manages lifetime)
      struct chunk_ptr {
        chunk_ptr() noexcept : ptr_(nullptr) {}
        explicit chunk_ptr(T* p) noexcept : ptr_(p) {}
        chunk_ptr(chunk_ptr&& o) noexcept : ptr_(o.ptr_) { o.ptr_ = nullptr; }
        chunk_ptr& operator=(chunk_ptr&& o) noexcept { ptr_ = o.ptr_; o.ptr_ = nullptr; return *this; }
        T* get() const noexcept { return ptr_; }
        T& operator[](std::size_t i) const noexcept { return ptr_[i]; }
        bool operator==(const chunk_ptr& o) const noexcept { return ptr_ == o.ptr_; }
        T* ptr_;
      };

      std::vector<chunk_ptr> chunks_;
      std::size_t pos_ = 0; // next free slot in current chunk
    };

    /**
     * Hybrid children container: uses a flat sorted vector for small child sets
     * (cache-friendly linear search) and upgrades to unordered_map when the
     * number of children exceeds kUpgradeThreshold.
     */
    template <typename V> struct hybrid_children_map {
      static constexpr std::size_t kUpgradeThreshold = 8;

      using pair_type = std::pair<std::string_view, V*>;
      using small_type = std::vector<pair_type>;
      using large_type = std::unordered_map<std::string_view, V*>;

      hybrid_children_map() noexcept : storage_(small_type{}) {}

      V* find(std::string_view key) const noexcept {
        if (auto* vec = std::get_if<small_type>(&storage_)) {
          for (auto& [k, v] : *vec)
            if (k == key) return v;
          return nullptr;
        }
        auto& map = std::get<large_type>(storage_);
        auto it = map.find(key);
        return it != map.end() ? it->second : nullptr;
      }

      void insert(std::string_view key, V* value) {
        if (auto* vec = std::get_if<small_type>(&storage_)) {
          vec->emplace_back(key, value);
          if (vec->size() >= kUpgradeThreshold) {
            // Upgrade to unordered_map
            large_type map;
            map.reserve(vec->size() * 2);
            for (auto& [k, v] : *vec)
              map.emplace(k, v);
            storage_ = std::move(map);
          }
          return;
        }
        std::get<large_type>(storage_).emplace(key, value);
      }

      std::size_t size() const noexcept {
        if (auto* vec = std::get_if<small_type>(&storage_))
          return vec->size();
        return std::get<large_type>(storage_).size();
      }

      // Iterate over all children (for for_all_routes)
      template <typename F> void for_each(F&& f) const {
        if (auto* vec = std::get_if<small_type>(&storage_)) {
          for (auto& [k, v] : *vec)
            f(k, v);
        }
        else {
          for (auto& [k, v] : std::get<large_type>(storage_))
            f(k, v);
        }
      }

    private:
      std::variant<small_type, large_type> storage_;
    };

    template <typename V> struct drt_node {
      drt_node() noexcept : pool_(nullptr), v_{ 0, nullptr } {}
      drt_node(drt_node_pool<drt_node>& pool) noexcept : pool_(pool), v_{ 0, nullptr } {}

      struct iterator {
        const drt_node<V>* ptr;
        std::string_view first;
        V second;

        auto operator->() noexcept { return this; }
        bool operator==(const iterator& b) const noexcept { return this->ptr == b.ptr; }
        bool operator!=(const iterator& b) const noexcept { return this->ptr != b.ptr; }
      };

      auto end() const noexcept { return iterator{ nullptr, std::string_view(), V() }; }

      auto& find_or_create(std::string_view r, unsigned int c) {
        if (c == r.size())
          return v_;

        if (r[c] == '/')
          c++; // skip the /
        int s = c;
        while (c < r.size() and r[c] != '/')
          c++;
        std::string_view k = r.substr(s, c - s);

        auto* existing = children_.find(k);
        if (!existing) {
          auto* child = pool_.allocate(pool_);
          children_.insert(k, child);
          // Cache the parameter child pointer for O(1) lookup in find()
          if (k.size() > 4 and k[0] == '{' and k[1] == '{' and
            k[k.size() - 2] == '}' and k[k.size() - 1] == '}')
            param_child_ = child;
          return child->find_or_create(r, c);
        }
        return existing->find_or_create(r, c);
      }

      template <typename F> void for_all_routes(F f, std::string prefix = "") const {
        if (children_.size() == 0) {
          f(prefix, v_);
        }
        else {
          if (prefix.size() && prefix.back() != '/') {
            prefix += '/';
          }
          children_.for_each([&](std::string_view key, const drt_node* child) {
            child->for_all_routes(f, prefix + std::string(key));
            });
        }
      }

      // Find a route.
      iterator find(const std::string_view& r, unsigned int c) const noexcept {
        // We found the route r.
        if ((c == r.size() and v_.handler != nullptr) or (children_.size() == 0))
          return iterator{ this, r, v_ };

        // r does not match any route.
        if (c == r.size() and v_.handler == nullptr)
          return iterator{ nullptr, r, v_ };

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
        auto* child = children_.find(k);
        if (child) {
          auto it2 = child->find(r, c); // search in the corresponding child.
          if (it2 != child->end())
            return it2;
        }

        // O(1) fallback to cached parameter child instead of O(n) linear scan
        if (param_child_)
          return param_child_->find(r, c);
        return end();
      }

      V v_;
      hybrid_children_map<drt_node> children_;
      drt_node* param_child_{ nullptr }; // cached {{param}} child for O(1) lookup
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
      : data_(std::make_shared<internal::dynamic_routing_table_data<V>>()) {
    }
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
