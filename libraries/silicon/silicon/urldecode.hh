#include <set>
#include <map>
#include <string>
#include <vector>
#include <string_view>
#include <microhttpd.h>
#include <iod/silicon/error.hh>
#include <iod/silicon/optional.hh>
#include <iod/silicon/symbols.hh>

namespace iod
{
  using namespace iod;

  // Decode a plain value.
  template <typename O>
  std::string_view urldecode2(std::set<void*>& found, std::string_view str, O& obj)
  {
    if (str.size() == 0)
      throw std::runtime_error(format_error("Urldecode error: expected key end", str[0]));

    if (str[0] != '=')
      throw std::runtime_error(format_error("Urldecode error: expected =, got ", str[0]));

    int start = 1;
    int end = 1;
    
    while (str.size() != end and str[end] != '&') end++;

    if (start != end)
    {
      std::string content(str.substr(start, end - start));
      // Url escape.
      content.resize(MHD_http_unescape(&content[0]));
      obj = boost::lexical_cast<O>(content);
      found.insert(&obj);
    }
    if (end == str.size())
      return str.substr(end, 0);
    else
      return str.substr(end + 1, str.size() - end - 1);
  }

  template <typename... O>
  std::string_view urldecode2(std::set<void*>& found, std::string_view str, metamap<O...>& obj, bool root = false);
  
  // Decode an array element.
  template <typename O>
  std::string_view urldecode2(std::set<void*>& found, std::string_view str, std::vector<O>& obj)
  {
    if (str.size() == 0)
      throw std::runtime_error(format_error("Urldecode error: expected key end", str[0]));

    if (str[0] != '[')
      throw std::runtime_error(format_error("Urldecode error: expected [, got ", str[0]));

    // Get the index substring.
    int index_start = 1;
    int index_end = 1;
    
    while (str.size() != index_end and str[index_end] != ']') index_end++;

    if (str.size() == 0)
      throw std::runtime_error(format_error("Urldecode error: expected key end", str[0]));
    
    auto next_str = str.substr(index_end + 1, str.size() - index_end - 1);

    if (index_end == index_start) // [] syntax, push back a value.
    {
      O x;
      auto ret = urldecode2(found, next_str, x);
      obj.push_back(x);
      return ret;
    }
    else // [idx] set index idx.
    {
      int idx = std::strtol(str.data() + index_start, nullptr, 10);
      if (idx >= 0 and idx <= 9999)
      {
        if (obj.size() <= idx)
          obj.resize(idx + 1);
        return urldecode2(found, next_str, obj[idx]);
      }
      else
        throw std::runtime_error(format_error("Urldecode error: out of bound array subscript."));
    }
    
  }

  // Decode an object member.
  template <typename... O>
  std::string_view urldecode2(std::set<void*>& found, std::string_view str, metamap<O...>& obj, bool root)
  {
    if (str.size() == 0)
      throw http_error::bad_request("Urldecode error: expected key end", str[0]);
      
    int key_start = 0;
    int key_end = key_start + 1;

    int next = 0;

    if (not root)
    {
      if (str[0] != '[')
        throw std::runtime_error(format_error("Urldecode error: expected [, got ", str[0]));
        
      key_start = 1;
      while (key_end != str.size() and str[key_end] != ']' and str[key_end] != '=') key_end++;

      if (key_end == str.size())
        throw std::runtime_error("Urldecode error: unexpected end.");
      next = key_end + 1; // skip the ]
    }
    else
    {
      while (key_end != str.size() and str[key_end] != '[' and str[key_end] != '=') key_end++;
      next = key_end;
    }

    auto key = str.substr(key_start, key_end - key_start);
    auto next_str = str.substr(next, str.size() - next);

    std::string_view ret;
    map(obj, [&] (auto k, auto& v)
    {
      if (iod::symbol_string(k) == key)
      {
        ret = urldecode2(found, next_str, v);
      }
    });
    return ret;
  }

  template <typename O>
  std::string urldecode_check_missing_fields(const std::set<void*>& found, optional<O>& obj)
  {
    return std::string();
  }

  template <typename O>
  std::string urldecode_check_missing_fields(const std::set<void*>& found, O& obj)
  {
    if (found.find(&obj) == found.end())
      return " ";
    else
      return std::string();
  }

  template <typename O>
  std::string urldecode_check_missing_fields(const std::set<void*>& found, std::vector<O>& obj)
  {
    // Fixme : implement missing fields checking in std::vector<metamap<O...>>
    // for (int i = 0; i < obj.size(); i++)
    // {
    //   std::string missing = urldecode_check_missing_fields(found, obj[i]);
    //   if (missing.size())
    //   {
    //     std::stringstream ss;
    //     ss << '[' << i << "]" << missing;
    //     return ss.str();
    //   }
    // }
    return std::string();
  }
  
  template <typename... O>
  std::string urldecode_check_missing_fields(const std::set<void*>& found, metamap<O...>& obj, bool root = false)
  {
    std::string missing;
    map(obj, [&] (auto k, auto& v)
    {
      missing = urldecode_check_missing_fields(found, v);
      if (missing.size())
        missing = (root ? "" : ".") + std::string(iod::symbol_string(k)) + missing;
    });
    return missing;
  }

  template <typename S, typename... O>
  std::string urldecode_check_missing_fields_on_subset(const std::set<void*>& found, metamap<O...>& obj, bool root = false)
  {
    std::string missing;
    map(S(), [&] (auto k, auto)
    {
      auto& v = obj[k];
      missing = urldecode_check_missing_fields(found, v);
      if (missing.size())
        missing = (root ? "" : ".") + std::string(iod::symbol_string(k)) + missing;
    });
    return missing;
  }
  
  // Frontend.
  template <typename O>
  void urldecode(std::string_view str, O& obj)
  {
    std::set<void*> found;

    // Parse the urlencoded string
    while (str.size() > 0)
      str = urldecode2(found, str, obj, true);

    // Check for missing fields.
    std::string missing = urldecode_check_missing_fields(found, obj, true);
    if (missing.size())
      throw std::runtime_error(format_error("Missing argument ", missing));
  }

}
