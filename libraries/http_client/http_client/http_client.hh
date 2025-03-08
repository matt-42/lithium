#pragma once
#ifndef CURL_STATICLIB
#define CURL_STATICLIB
#endif  // CURL_STATICLIB
#pragma comment(lib, "crypt32")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Wldap32.lib")
#pragma comment(lib, "Normaliz.lib")

#include <curl/curl.h>

#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <unordered_map>

#include <li/http_client/symbols.hh>
#include <li/json/json.hh>

namespace li {

inline size_t curl_write_callback(char* ptr, size_t size, size_t nmemb, void* userdata);

inline std::streamsize curl_read_callback(void* ptr, size_t size, size_t nmemb, void* stream);

inline size_t curl_header_callback(char* buffer, size_t size, size_t nitems, void* userdata) {
  auto& headers_map = *(std::unordered_map<std::string, std::string>*)userdata;

  size_t split = 0;
  size_t total_size = size * nitems;
  while (split < total_size && buffer[split] != ':')
    split++;

  if (split == total_size)
    return total_size;
  // throw std::runtime_error("Header line does not contains a colon (:)");

  int skip_nl = (buffer[total_size - 1] == '\n');
  int skip_space = (buffer[split + 1] == ' ');
  headers_map[std::string(buffer, split)] =
      std::string(buffer + split + 1 + skip_space, total_size - split - 1 - skip_nl - skip_space);
  return total_size;
}

struct http_client {

  enum http_method { HTTP_GET, HTTP_POST, HTTP_PUT, HTTP_DELETE };

  inline http_client(const std::string& prefix = "") : url_prefix_(prefix) {
    curl_global_init(CURL_GLOBAL_ALL);
    curl_ = curl_easy_init();
  }

  inline ~http_client() { curl_easy_cleanup(curl_); }

  inline http_client& operator=(const http_client&) = delete;

  template <typename... A>
  inline auto operator()(http_method http_method, const std::string_view& url, const A&... args) {

    struct curl_slist* headers_list = NULL;

    auto arguments = mmm(args...);
    constexpr bool fetch_headers = has_key<decltype(arguments)>(s::fetch_headers);

    // Generate url.
    std::ostringstream url_ss;
    url_ss << url_prefix_ << url;

    // Get params
    auto get_params = li::get_or(arguments, s::get_parameters, mmm());
    bool first = true;
    li::map(get_params, [&](auto k, auto v) {
      if (first)
        url_ss << '?';
      else
        url_ss << "&";
      std::ostringstream value_ss;
      value_ss << v;
      char* escaped = curl_easy_escape(curl_, value_ss.str().c_str(), value_ss.str().size());
      url_ss << li::symbol_string(k) << '=' << escaped;
      first = false;
      curl_free(escaped);
    });

    // Additional request headers
    auto request_headers = li::get_or(arguments, s::request_headers, mmm());
    li::map(request_headers, [&headers_list](auto k, auto v) {
      std::ostringstream header_ss;
      header_ss << li::symbol_string(k) << ": " << v;
      headers_list = curl_slist_append(headers_list, header_ss.str().c_str());
    });

    // std::cout << url_ss.str() << std::endl;
    // Pass the url to libcurl.
    curl_easy_setopt(curl_, CURLOPT_URL, url_ss.str().c_str());

    // HTTP_POST parameters.
    bool is_urlencoded = not li::has_key(arguments, s::json_encoded);
    std::ostringstream post_stream;
    std::string rq_body;
    if (is_urlencoded) { // urlencoded
      req_body_buffer_.str("");

      auto post_params = li::get_or(arguments, s::post_parameters, mmm());
      first = true;
      li::map(post_params, [&](auto k, auto v) {
        if (!first)
          post_stream << "&";
        post_stream << li::symbol_string(k) << "=";
        std::ostringstream value_str;
        value_str << v;
        char* escaped = curl_easy_escape(curl_, value_str.str().c_str(), value_str.str().size());
        first = false;
        post_stream << escaped;
        curl_free(escaped);
      });
      rq_body = post_stream.str();
      req_body_buffer_.str(rq_body);

    } else // Json encoded
      rq_body = li::json_encode(li::get_or(arguments, s::post_parameters, mmm()));

    // HTTP HTTP_POST
    if (http_method == HTTP_POST) {
      curl_easy_setopt(curl_, CURLOPT_POST, 1);
      curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, rq_body.c_str());
    }

    // HTTP HTTP_GET
    if (http_method == HTTP_GET)
      curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1);

    // HTTP HTTP_PUT
    if (http_method == HTTP_PUT) {
      curl_easy_setopt(curl_, CURLOPT_UPLOAD, 1L);
      curl_easy_setopt(curl_, CURLOPT_READFUNCTION, curl_read_callback);
      curl_easy_setopt(curl_, CURLOPT_READDATA, this);
    }

    if (http_method == HTTP_PUT or http_method == HTTP_POST) {
      if (is_urlencoded)
        headers_list =
            curl_slist_append(headers_list, "Content-Type: application/x-www-form-urlencoded");
      else
        headers_list = curl_slist_append(headers_list, "Content-Type: application/json");
    }

    // HTTP HTTP_DELETE
    if (http_method == HTTP_DELETE)
      curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "HTTP_DELETE");

    // Cookies
    curl_easy_setopt(curl_, CURLOPT_COOKIEJAR,
                     0); // Enable cookies but do no write a cookiejar.

    body_buffer_.clear();
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, curl_write_callback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, this);

    curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers_list);

    if (li::has_key(arguments, s::disable_check_certificate))
      curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, 0);

    // Setup response header parsing.
    std::unordered_map<std::string, std::string> response_headers_map;
    if (fetch_headers) {
      curl_easy_setopt(curl_, CURLOPT_HEADERDATA, &response_headers_map);
      curl_easy_setopt(curl_, CURLOPT_HEADERFUNCTION, curl_header_callback);
    }

    // Set timeout.
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 10);

    // Send the request.
    char errbuf[CURL_ERROR_SIZE];
    curl_easy_setopt(curl_, CURLOPT_ERRORBUFFER, errbuf);
    if (curl_easy_perform(curl_) != CURLE_OK) {
      std::ostringstream errss;
      errss << "Libcurl error when sending request: " << errbuf;
      std::cerr << errss.str() << std::endl;
      throw std::runtime_error(errss.str());
    }
    curl_slist_free_all(headers_list);
    // Read response code.
    long response_code;
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &response_code);

    // Return response object.
    if constexpr (fetch_headers)
      return mmm(s::status = response_code, s::body = body_buffer_,
                 s::headers = response_headers_map);
    else
      return mmm(s::status = response_code, s::body = body_buffer_);
  }

  template <typename... P> auto get(const std::string& url, P... params) {
    return this->operator()(HTTP_GET, url, params...);
  }
  template <typename... P> auto put(const std::string& url, P... params) {
    return this->operator()(HTTP_PUT, url, params...);
  }
  template <typename... P> auto post(const std::string& url, P... params) {
    return this->operator()(HTTP_POST, url, params...);
  }
  template <typename... P> auto delete_(const std::string& url, P... params) {
    return this->operator()(HTTP_DELETE, url, params...);
  }

  inline void read(char* ptr, int size) { body_buffer_.append(ptr, size); }

  inline std::streamsize write(char* ptr, int size) {
    std::streamsize ret = req_body_buffer_.sgetn(ptr, size);
    return ret;
  }

  CURL* curl_;
  std::map<std::string, std::string> cookies_;
  std::string body_buffer_;
  std::stringbuf req_body_buffer_;
  std::string url_prefix_;
};

inline std::streamsize curl_read_callback(void* ptr, size_t size, size_t nmemb, void* userdata) {
  http_client* client = (http_client*)userdata;
  return client->write((char*)ptr, size * nmemb);
}

size_t curl_write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
  http_client* client = (http_client*)userdata;
  client->read(ptr, size * nmemb);
  return size * nmemb;
}

template <typename... P> auto http_get(const std::string& url, P... params) {
  return http_client{}.get(url, params...);
}
template <typename... P> auto http_post(const std::string& url, P... params) {
  return http_client{}.post(url, params...);
}
template <typename... P> auto http_put(const std::string& url, P... params) {
  return http_client{}.put(url, params...);
}
template <typename... P> auto http_delete(const std::string& url, P... params) {
  return http_client{}.delete_(url, params...);
}

} // namespace li
