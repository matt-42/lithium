#pragma once

#include <curl/curl.h>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>

#include <iod/http_client/symbols.hh>
#include <iod/metajson/metajson.hh>

namespace iod {

namespace http_client {

using iod::metamap::make_metamap;
using iod::metamap::metamap;

inline size_t curl_write_callback(char* ptr, size_t size, size_t nmemb,
                                  void* userdata);

inline size_t curl_read_callback(void* ptr, size_t size, size_t nmemb,
                                 void* stream);

struct http_client {
  inline http_client(const std::string& prefix = "") : url_prefix_(prefix) {
    curl_global_init(CURL_GLOBAL_ALL);
    curl_ = curl_easy_init();
  }

  inline ~http_client() { curl_easy_cleanup(curl_); }

  inline http_client& operator=(const http_client&) = delete;

  template <typename... A>
  inline auto operator()(const std::string_view& url, const A&... args) {

    struct curl_slist* headers_list = NULL;

    auto arguments = make_metamap(args...);
    // Generate url.
    std::stringstream url_ss;
    url_ss << url_prefix_ << url;

    // Get params
    auto get_params = iod::metamap::get_or(arguments, s::get_parameters, make_metamap());
    bool first = true;
    iod::metamap::map(get_params, [&](auto k, auto v) {
      if (first)
        url_ss << '?';
      else
        url_ss << "&";
      std::stringstream value_ss;
      value_ss << v;
      char* escaped = curl_easy_escape(curl_, value_ss.str().c_str(),
                                       value_ss.str().size());
      url_ss << iod::symbol::symbol_string(k) << '=' << escaped;
      first = false;
      curl_free(escaped);
    });

    std::cout << url_ss.str() << std::endl;
    // Pass the url to libcurl.
    curl_easy_setopt(curl_, CURLOPT_URL, url_ss.str().c_str());

    // POST parameters.
    bool is_urlencoded = not iod::metamap::has_key(arguments, s::jsonencoded);
    std::stringstream post_stream;
    std::string rq_body;
    if (is_urlencoded) { // urlencoded
      req_body_buffer_.str("");

      auto post_params =
          iod::metamap::get_or(arguments, s::post_parameters, make_metamap());
      first = true;
      iod::metamap::map(post_params, [&](auto k, auto v) {
        if (!first)
          post_stream << "&";
        post_stream << iod::symbol::symbol_string(k) << "=";
        std::stringstream value_str;
        value_str << v;
        char* escaped = curl_easy_escape(curl_, value_str.str().c_str(),
                                         value_str.str().size());
        first = false;
        post_stream << escaped;
        curl_free(escaped);
      });
      rq_body = post_stream.str();
      req_body_buffer_.str(rq_body);

    } else // Json encoded
      rq_body = iod::metajson::json_encode(
          iod::metamap::get_or(arguments, s::post_parameters, make_metamap()));

    enum { GET, POST, PUT, DELETE };

    int http_method = iod::metamap::get_or(arguments, s::method, GET);

    // HTTP POST
    if (http_method == POST) {
      curl_easy_setopt(curl_, CURLOPT_POST, 1);
      curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, rq_body.c_str());
    }

    // HTTP GET
    if (http_method == GET)
      curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1);

    // HTTP PUT
    if (http_method == PUT) {
      curl_easy_setopt(curl_, CURLOPT_UPLOAD, 1L);
      curl_easy_setopt(curl_, CURLOPT_READFUNCTION, curl_read_callback);
      curl_easy_setopt(curl_, CURLOPT_READDATA, this);
    }

    if (http_method == PUT or http_method == POST)
      if (is_urlencoded)
        headers_list = curl_slist_append(
            headers_list, "Content-Type: application/x-www-form-urlencoded");
      else
        headers_list =
            curl_slist_append(headers_list, "Content-Type: application/json");

    // HTTP DELETE
    if (http_method == DELETE)
      curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "DELETE");

    // Cookies
    curl_easy_setopt(curl_, CURLOPT_COOKIEJAR,
                     0); // Enable cookies but do no write a cookiejar.

    body_buffer_.clear();
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION,
                     curl_write_callback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, this);

    curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers_list);

    // Send the request.
    char errbuf[CURL_ERROR_SIZE];
    curl_easy_setopt(curl_, CURLOPT_ERRORBUFFER, errbuf);
    if (curl_easy_perform(curl_) != CURLE_OK) {
      std::stringstream errss;
      errss << "Libcurl error when sending request: " << errbuf;
      throw std::runtime_error(errss.str());
    }
    curl_slist_free_all(headers_list);
    // Read response code.
    long response_code;
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &response_code);

    // Decode result.
    return make_metamap(s::status = response_code, s::body = body_buffer_);
  }

  inline void read(char* ptr, int size) { body_buffer_.append(ptr, size); }

  inline size_t write(char* ptr, int size) {
    size_t ret = req_body_buffer_.sgetn(ptr, size);
    return ret;
  }

  CURL* curl_;
  std::map<std::string, std::string> cookies_;
  std::string body_buffer_;
  std::stringbuf req_body_buffer_;
  std::string url_prefix_;
};

inline size_t curl_read_callback(void* ptr, size_t size, size_t nmemb,
                                 void* userdata) {
  http_client* client = (http_client*)userdata;
  return client->write((char*)ptr, size * nmemb);
}

size_t curl_write_callback(char* ptr, size_t size, size_t nmemb,
                           void* userdata) {
  http_client* client = (http_client*)userdata;
  client->read(ptr, size * nmemb);
  return size * nmemb;
}

} // namespace http_client
} // namespace iod
