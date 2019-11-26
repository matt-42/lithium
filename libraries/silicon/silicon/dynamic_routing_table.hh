#pragma once

#include <map>
#include <string_view>

namespace iod {

  namespace internal
  {

    template <typename V>
    struct drt_node
    {

      drt_node() : v(nullptr) {}
      struct iterator
      {
        drt_node<V>* ptr;
        std::string_view first;
        V second;

        auto operator->(){ return this; }
        bool operator==(const iterator& b) { return this->ptr == b.ptr; }
        bool operator!=(const iterator& b) { return this->ptr != b.ptr; }
      };

      auto end() { return iterator{nullptr, std::string_view(), V()}; }
      
      auto& find_or_create(const std::string_view& r, unsigned int c)
      {
        if (c == r.size())
          return v;

        c++; // skip the /
        int s = c;
        while (c < r.size() and r[c] != '/') c++;
        std::string_view k = r.substr(s, c - s);

        auto& v = childs[k].find_or_create(r, c);

        return v;
      }

      iterator find(const std::string_view& r, unsigned int c)
      {
        if ((c == r.size() and v != nullptr) or
            (childs.size() == 0))
          return iterator{this, r, v};
        if (c == r.size() and v == nullptr)
          return iterator{nullptr, r, v};

        c++; // skip the /
        int s = c;
        while (c < r.size() and r[c] != '/') c++;
        std::string_view k(&r[s], c - s);
        
        auto it = childs.find(k);
        if (it != childs.end())
        {
          auto it2 = it->second.find(r, c);
          if (it2 != it->second.end()) return it2;
        }

        {
          auto it2 = childs.find("*");
          if (it2 != childs.end())
            return it2->second.find(r, c);
          else
            return end();
        }
      }

      V v;
      std::map<std::string_view, drt_node> childs;
    };
  }
  
  template <typename V>
  struct dynamic_routing_table
  {

    // Find a route and return reference to a procedure.
    auto& operator[](const std::string_view& r)
    {
      return root.find_or_create(r, 0);
    }

    // Find a route and return an iterator.
    auto find(const std::string_view& r)
    {
      return root.find(r, 0);
    }

    auto end() { return root.end(); }

    internal::drt_node<V> root;
  };

}
