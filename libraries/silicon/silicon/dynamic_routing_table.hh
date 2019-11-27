#pragma once

#include <map>
#include <string_view>

namespace iod {

  namespace internal
  {

    template <typename V>
    struct drt_node
    {

      drt_node() : v{0, nullptr} {}
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
      
      auto& find_or_create(std::string_view r, unsigned int c)
      {
        if (c == r.size())
          return v;

        if (r[c] == '/')
          c++; // skip the /
        int s = c;
        while (c < r.size() and r[c] != '/') c++;
        std::string_view k = r.substr(s, c - s);

        auto& v = childs[k].find_or_create(r, c);

        return v;
      }

      template <typename F>
      void for_all_routes(F f, std::string prefix = "") const
      {
        if (childs.size() == 0)
          f(prefix, v);
        else
        {
          if (prefix.back() != '/')
            prefix += '/';
          for (auto it : childs)
            it.second.for_all_routes(f, prefix + std::string(it.first));
        }
      } 
      iterator find(const std::string_view& r, unsigned int c)
      {
        // We found the route r.
        if ((c == r.size() and v.handler != nullptr) or
            (childs.size() == 0))
          return iterator{this, r, v};

        // r does not match any route.
        if (c == r.size() and v.handler == nullptr)
          return iterator{nullptr, r, v};

        if (r[c] == '/')
          c++; // skip the first /

        // Find the next /.
        int s = c;
        while (c < r.size() and r[c] != '/') c++;

        // k is the string between the 2 /.
        std::string_view k(&r[s], c - s);
        
        // look for k in the childs.
        auto it = childs.find(k);
        if (it != childs.end())
        {
          auto it2 = it->second.find(r, c); // search in the corresponding child.
          if (it2 != it->second.end()) return it2;
        }


        {
          auto it2 = childs.find("*"); // wildcard.
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
      strings.push_back(std::shared_ptr<std::string>(new std::string(r)));
      std::string_view r2(*strings.back());
      return root.find_or_create(r2, 0);
    }
    auto& operator[](const std::string& r)
    {
      strings.push_back(std::shared_ptr<std::string>(new std::string(r)));
      std::string_view r2(*strings.back());
      return root.find_or_create(r2, 0);
    }

    // Find a route and return an iterator.
    auto find(const std::string_view& r)
    {
      return root.find(r, 0);
    }


    template <typename F>
    void for_all_routes(F f) const
    {
      root.for_all_routes(f);
    } 
    auto end() { return root.end(); }

    std::vector<std::shared_ptr<std::string>> strings;
    internal::drt_node<V> root;
  };

}
