#pragma once

#include <cstring>
#include <functional>
#include <iostream>
#include <string_view>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <unordered_map>

#include <boost/lexical_cast.hpp>

#include <li/http_server/output_buffer.hh>
#include <li/http_server/input_buffer.hh>
#include <li/http_server/error.hh>
#include <li/http_server/symbols.hh>
#include <li/http_server/tcp_server.hh>
#include <li/http_server/url_unescape.hh>
#include <li/http_server/http_top_header_builder.hh>

#include <li/http_server/content_types.hh>

namespace li {

namespace http_async_impl {

static char* date_buf = nullptr;
static int date_buf_size = 0;

using ::li::content_types; // static std::unordered_map<std::string_view, std::string_view> content_types

static thread_local std::unordered_map<std::string, std::pair<std::string_view, std::string_view>> static_files;

http_top_header_builder http_top_header [[gnu::weak]];

template <typename FIBER>
struct generic_http_ctx {

  generic_http_ctx(input_buffer& _rb, FIBER& _fiber) : rb(_rb), fiber(_fiber) {
    get_parameters_map.reserve(10);
    response_headers.reserve(20);

    output_stream = output_buffer(50*1024, [&](const char* d, int s) { fiber.write(d, s); });

    headers_stream =
        output_buffer(1000, [&](const char* d, int s) { output_stream << std::string_view(d, s); });

    json_stream = output_buffer(
        50 * 1024, [&](const char* d, int s) { output_stream << std::string_view(d, s); });
  }

  generic_http_ctx& operator=(const generic_http_ctx&) = delete;
  generic_http_ctx(const generic_http_ctx&) = delete;

  std::string_view header(const char* key) {
    if (!header_map.size())
      index_headers();
    return header_map[key];
  }

  std::string_view cookie(const char* key) {
    if (!cookie_map.size())
      index_cookies();
    return cookie_map[key];
  }

  std::string_view get_parameter(const char* key) {
    if (!url_.size())
      parse_first_line();
    return get_parameters_map[key];
  }

  // std::string_view post_parameter(const char* key)
  // {
  //   if (!is_body_read_)
  //   {
  //     //read_whole_body();
  //     parse_post_parameters();
  //   }
  //   return post_parameters_map[key];
  // }

  // std::string_view read_body(char* buf, int buf_size)
  // {
  //   // Try to read Content-Length

  //   // if part of the body is in the request buffer
  //   //  return it and mark it as read.
  //   // otherwise call read on the socket.

  //   // handle chunked encoding here if needed.
  // }

  // void sendfile(std::string path)
  // {
  //   char buffer[10200];
  //   //std::cout << uint64_t(output_stream.buffer_) << " " << uint64_t(output_buffer_space) <<
  //   std::endl; output_buffer output_stream(buffer, sizeof(buffer));

  //   struct stat stat_buf;
  //   int read_fd = open (path.c_str(), O_RDONLY);
  //   fstat (read_fd, &stat_buf);

  //   format_top_headers(output_stream);
  //   output_stream << headers_stream.to_string_view();
  //   output_stream << "Content-Length: " << size_t(stat_buf.st_size) << "\r\n\r\n"; //Add body
  //   auto m = output_stream.to_string_view();
  //   write(m.data(), m.size());

  //   off_t offset = 0;
  //   while (true)
  //   {
  //     int ret = ::sendfile(socket_fd, read_fd, &offset, stat_buf.st_size);
  //     if (ret == EAGAIN)
  //       write(nullptr, 0);
  //     else break;
  //   }
  // }

  std::string_view url() {
    if (!url_.size())
      parse_first_line();
    return url_;
  }
  std::string_view method() {
    if (!method_.size())
      parse_first_line();
    return method_;
  }
  std::string_view http_version() {
    if (!url_.size())
      parse_first_line();
    return http_version_;
  }

  inline void format_top_headers(output_buffer& output_stream) {
    if (status_code_ == 200)
      output_stream << http_top_header.top_header_200();
    else
      output_stream << "HTTP/1.1 " << status_ << http_top_header.top_header();
    // output_stream << "HTTP/1.1 " << status_;
    // output_stream << "\r\nDate: " << std::string_view(date_buf, date_buf_size);
    // #ifdef LITHIUM_SERVER_NAME
    //   #define MACRO_TO_STR2(L) #L
    //   #define MACRO_TO_STR(L) MACRO_TO_STR2(L)
    //   output_stream << "\r\nConnection: keep-alive\r\nServer: " MACRO_TO_STR(LITHIUM_SERVER_NAME) "\r\n";
    //   #undef MACRO_TO_STR
    //   #undef MACRO_TO_STR2
    // #else
    //   output_stream << "\r\nConnection: keep-alive\r\nServer: Lithium\r\n";
    // #endif
  }

  void prepare_request() {
    // parse_first_line();
    response_headers.clear();
    content_length_ = 0;
    chunked_ = 0;

    for (int i = 1; i < header_lines.size() - 1; i++) {
      const char* line_end = header_lines[i + 1]; // last line is just an empty line.
      const char* cur = header_lines[i];

      if (*cur != 'C' and *cur != 'c')
        continue;

      std::string_view key = split(cur, line_end, ':');

      auto get_value = [&] {
        std::string_view value = split(cur, line_end, '\r');
        while (value[0] == ' ')
          value = std::string_view(value.data() + 1, value.size() - 1);
        return value;
      };

      if (key == "Content-Length")
        content_length_ = atoi(get_value().data());
      else if (key == "Content-Type") {
        content_type_ = get_value();
        chunked_ = (content_type_ == "chunked");
      }

    }
  }

  // void respond(std::string s) {return respond(std::string_view(s)); }

  // void respond(const char* s)
  // {
  //   return respond(std::string_view(s, strlen(s)));
  // }

  void respond(const std::string_view& s) {
    response_written_ = true;
    format_top_headers(output_stream);
    headers_stream.flush();                                             // flushes to output_stream.
    output_stream << "Content-Length: " << s.size() << "\r\n\r\n" << s; // Add body
  }

  template <typename O> void respond_json(const O& obj) {
    response_written_ = true;
    json_stream.reset();
    json_encode(json_stream, obj);

    format_top_headers(output_stream);
    headers_stream.flush(); // flushes to output_stream.
    output_stream << "Content-Length: " << json_stream.to_string_view().size() << "\r\n\r\n";
    json_stream.flush(); // flushes to output_stream.
  }

  template <typename F> void respond_json_generator(int N, F callback) {
    response_written_ = true;
    json_stream.reset();
    json_encode_generator(json_stream, N, callback);

    format_top_headers(output_stream);
    headers_stream.flush(); // flushes to output_stream.
    output_stream << "Content-Length: " << json_stream.to_string_view().size() << "\r\n\r\n";
    json_stream.flush(); // flushes to output_stream.
    
  }


  void respond_if_needed() {
    if (!response_written_) {
      response_written_ = true;

      format_top_headers(output_stream);
      output_stream << headers_stream.to_string_view();
      output_stream << "Content-Length: 0\r\n\r\n"; // Add body
    }
  }

  void set_header(std::string_view k, std::string_view v) {
    headers_stream << k << ": " << v << "\r\n";
  }

  void set_cookie(std::string_view k, std::string_view v) {
    headers_stream << "Set-Cookie: " << k << '=' << v << "\r\n";
  }

  void set_status(int status) {

    status_code_ = status;
    switch (status) {
    case 200:
      status_ = "200 OK";
      break;
    case 201:
      status_ = "201 Created";
      break;
    case 204:
      status_ = "204 No Content";
      break;
    case 304:
      status_ = "304 Not Modified";
      break;
    case 400:
      status_ = "400 Bad Request";
      break;
    case 401:
      status_ = "401 Unauthorized";
      break;
    case 402:
      status_ = "402 Not Found";
      break;
    case 403:
      status_ = "403 Forbidden";
      break;
    case 404:
      status_ = "404 Not Found";
      break;
    case 409:
      status_ = "409 Conflict";
      break;
    case 500:
      status_ = "500 Internal Server Error";
      break;
    default:
      status_ = "200 OK";
      break;
    }
  }

  void send_static_file(const char* path) {
    auto it = static_files.find(path);
    if (static_files.end() == it or !it->second.first.size()) {
      int fd = open(path, O_RDONLY);
      if (fd == -1)
        throw http_error::not_found("File not found.");

      int file_size = lseek(fd, (size_t)0, SEEK_END);
      auto content =
          std::string_view((char*)mmap(0, file_size, PROT_READ, MAP_SHARED, fd, 0), file_size);
      if (!content.data()) throw http_error::not_found("File not found.");
      close(fd);

      size_t ext_pos = std::string_view(path).rfind('.');
      std::string_view content_type("");
      if (ext_pos != std::string::npos)
      {
        auto type_itr = content_types.find(std::string_view(path).substr(ext_pos + 1).data());
        if (type_itr != content_types.end())
        {
          content_type = type_itr->second;
          set_header("Content-Type", content_type);
        }
      }
      static_files.insert({path, {content, content_type}});
      respond(content);
    } else {
      if (it->second.second.size())
      {
        set_header("Content-Type", it->second.second);
      }
      respond(it->second.first);
    }
  }

  // private:

  void add_header_line(const char* l) { header_lines.push_back(l); }
  const char* last_header_line() { return header_lines.back(); }

  // split a string, starting from cur and ending with split_char.
  // Advance cur to the end of the split.
  std::string_view split(const char*& cur, const char* line_end, char split_char) {

    const char* start = cur;
    while (start < (line_end - 1) and *start == split_char)
      start++;

#if 0
    const char* end = (const char*)memchr(start + 1, split_char, line_end - start - 2);
    if (!end) end = line_end - 1;
#else    
    const char* end = start + 1;
    while (end < (line_end - 1) and *end != split_char)
      end++;
#endif
    cur = end + 1;
    if (*end == split_char)
      return std::string_view(start, cur - start - 1);
    else
      return std::string_view(start, cur - start);
  }

  void index_headers() {
    for (int i = 1; i < header_lines.size() - 1; i++) {
      const char* line_end = header_lines[i + 1]; // last line is just an empty line.
      const char* cur = header_lines[i];

      std::string_view key = split(cur, line_end, ':');
      std::string_view value = split(cur, line_end, '\r');
      while (value[0] == ' ')
        value = std::string_view(value.data() + 1, value.size() - 1);
      header_map[key] = value;
      // std::cout << key << " -> " << value << std::endl;
    }
  }

  void index_cookies() {
    std::string_view cookies = header("Cookie");
    if (!cookies.data())
      return;
    const char* line_end = &cookies.back() + 1;
    const char* cur = &cookies.front();
    while (cur < line_end) {

      std::string_view key = split(cur, line_end, '=');
      std::string_view value = split(cur, line_end, ';');
      while (key[0] == ' ')
        key = std::string_view(key.data() + 1, key.size() - 1);
      cookie_map[key] = value;
    }
  }

  template <typename C> void url_decode_parameters(std::string_view content, C kv_callback) {
    const char* c = content.data();
    const char* end = c + content.size();
    if (c < end) {
      while (c < end) {
        std::string_view key = split(c, end, '=');
        std::string_view value = split(c, end, '&');
        kv_callback(key, value);
        // printf("kv: '%.*s' -> '%.*s'\n", key.size(), key.data(), value.size(), value.data());
      }
    }
  }

  void parse_first_line() {
    const char* c = header_lines[0];
    const char* end = header_lines[1];

    method_ = split(c, end, ' ');
    url_ = split(c, end, ' ');
    http_version_ = split(c, end, '\r');

    // url get parameters.
    c = url_.data();
    end = c + url_.size();
    url_ = split(c, end, '?');
    get_parameters_string_ = std::string_view(c, end - c);
  }

  std::string_view get_parameters_string() {
    if (!get_parameters_string_.data())
      parse_first_line();
    return get_parameters_string_;
  }
  template <typename F> void parse_get_parameters(F processor) {
    url_decode_parameters(get_parameters_string(), processor);
  }

  template <typename F> void read_body(F callback) {
    is_body_read_ = true;

    if (!chunked_ and !content_length_)
      body_end_ = body_start.data();
    else if (content_length_) {
      std::string_view res;
      int n_body_read = 0;

      // First return part of the body already in memory.
      int l = std::min(int(body_start.size()), content_length_);
      callback(std::string_view(body_start.data(), l));
      n_body_read += l;

      while (content_length_ > n_body_read) {
        std::string_view part = rb.read_more_str(fiber);
        int l = part.size();
        int bl = std::min(l, content_length_ - n_body_read);
        part = std::string_view(part.data(), bl);
        callback(part);
        rb.free(part);
        body_end_ = part.data();
        n_body_read += part.size();
      }

    } else if (chunked_) {
      // Chunked decoding.
      const char* cur = body_start.data();
      int chunked_size = strtol(rb.read_until(read, cur, '\r').data(), nullptr, 16);
      cur++; // skip \n
      while (chunked_size > 0) {
        // Read chunk.
        std::string_view chunk = rb.read_n(read, cur, chunked_size);
        callback(chunk);
        rb.free(chunk);
        cur += chunked_size + 2; // skip \r\n.

        // Read next chunk size.
        chunked_size = strtol(rb.read_until(read, cur, '\r').data(), nullptr, 16);
        cur++; // skip \n
      }
      cur += 2; // skip the terminaison chunk.
      body_end_ = cur;
      body_ = std::string_view(body_start.data(), cur - body_start.data());
    }
  }

  std::string_view read_whole_body() {
    if (!chunked_ and !content_length_) {
      is_body_read_ = true;
      body_end_ = body_start.data();
      return std::string_view(); // No body.
    }

    if (content_length_) {
      body_ = rb.read_n(fiber, body_start.data(), content_length_);
      body_end_ = body_.data() + content_length_;
    } else if (chunked_) {
      // Chunked decoding.
      char* out = (char*)body_start.data();
      const char* cur = body_start.data();
      int chunked_size = strtol(rb.read_until(fiber, cur, '\r').data(), nullptr, 16);
      cur++; // skip \n
      while (chunked_size > 0) {
        // Read chunk.
        std::string_view chunk = rb.read_n(fiber, cur, chunked_size);
        cur += chunked_size + 2; // skip \r\n.
        // Copy the body into a contiguous string.
        if (out + chunk.size() > chunk.data()) // use memmove if overlap.
          std::memmove(out, chunk.data(), chunk.size());
        else
          std::memcpy(out, chunk.data(), chunk.size());

        out += chunk.size();

        // Read next chunk size.
        chunked_size = strtol(rb.read_until(fiber, cur, '\r').data(), nullptr, 16);
        cur++; // skip \n
      }
      cur += 2; // skip the terminaison chunk.
      body_end_ = cur;
      body_ = std::string_view(body_start.data(), out - body_start.data());
    }

    is_body_read_ = true;
    return body_;
  }

  void read_multipart_formdata() {}

  template <typename F> void post_iterate(F kv_callback) {
    if (is_body_read_) // already in memory.
      url_decode_parameters(body_, kv_callback);
    else // stream the body.
    {
      std::string_view current_key;

      read_body([](std::string_view part) {
        // read key if needed.
        // if (!current_key.size())
        // call kv callback if some value is available.
      });
    }
  }

  // Read post parameters in the body.
  std::unordered_map<std::string_view, std::string_view> post_parameters() {
    if (content_type_ == "application/x-www-form-urlencoded") {
      if (!is_body_read_)
        read_whole_body();
      url_decode_parameters(body_, [&](auto key, auto value) { post_parameters_map[key] = value; });
      return post_parameters_map;
    } else {
      // fixme: return bad request here.
    }
    return post_parameters_map;
  }

  void prepare_next_request() {
    if (!is_body_read_)
      read_whole_body();

    // std::cout <<"free line0: " << uint64_t(header_lines[0]) << std::endl;
    // std::cout << rb.current_size() << " " << rb.cursor << std::endl;
    rb.free(header_lines[0], body_end_);
    // std::cout << rb.current_size() << " " << rb.cursor << std::endl;
    // rb.cursor = rb.end = 0;
    // assert(rb.cursor == 0);
    headers_stream.reset();
    status_ = "200 OK";
    method_ = std::string_view();
    url_ = std::string_view();
    http_version_ = std::string_view();
    content_type_ = std::string_view();
    header_map.clear();
    cookie_map.clear();
    response_headers.clear();
    get_parameters_map.clear();
    post_parameters_map.clear();
    get_parameters_string_ = std::string_view();
    response_written_ = false;
  }

  void flush_responses() { output_stream.flush(); }

  int socket_fd;
  input_buffer& rb;

  int status_code_ = 200;
  const char* status_ = "200 OK";
  std::string_view method_;
  std::string_view url_;
  std::string_view http_version_;
  std::string_view content_type_;
  bool chunked_;
  int content_length_;
  std::unordered_map<std::string_view, std::string_view> header_map;
  std::unordered_map<std::string_view, std::string_view> cookie_map;
  std::vector<std::pair<std::string_view, std::string_view>> response_headers;
  std::unordered_map<std::string_view, std::string_view> get_parameters_map;
  std::unordered_map<std::string_view, std::string_view> post_parameters_map;
  std::string_view get_parameters_string_;
  // std::vector<std::string> strings_saver;

  bool is_body_read_ = false;
  std::string body_local_buffer_;
  std::string_view body_;
  std::string_view body_start;
  const char* body_end_ = nullptr;
  std::vector<const char*> header_lines;
  FIBER& fiber;

  output_buffer headers_stream;
  bool response_written_ = false;

  output_buffer output_stream;
  output_buffer json_stream;
};
using http_ctx = generic_http_ctx<async_fiber_context>;

template <typename F> auto make_http_processor(F handler) {
  return [handler](auto& fiber) {
    try {
      input_buffer rb;
      bool socket_is_valid = true;

      auto ctx = generic_http_ctx(rb, fiber);
      ctx.socket_fd = fiber.socket_fd;
      
      while (true) {
        ctx.is_body_read_ = false;
        ctx.header_lines.clear();
        ctx.header_lines.reserve(100);
        // Read until there is a complete header.
        int header_start = rb.cursor;
        int header_end = rb.cursor;
        assert(header_start >= 0);
        assert(header_end >= 0);
        assert(ctx.header_lines.size() == 0);
        ctx.add_header_line(rb.data() + header_end);
        assert(ctx.header_lines.size() == 1);

        bool complete_header = false;

        if (rb.empty())
          if (!rb.read_more(fiber))
            return;

        const char* cur = rb.data() + header_end;
        const char* rbend = rb.data() + rb.end - 3;
        while (!complete_header) {
          // Look for end of header and save header lines.
#if 0
          // Memchr optimization. Does not seem to help but I can't find why.
          while (cur < rbend) {
           cur = (const char*) memchr(cur, '\r', 1 + rbend - cur);
           if (!cur) {
             cur = rbend + 1;
             break;
           }
           if (cur[1] == '\n') { // \n already checked by memchr. 
#else          
          while ((cur - rb.data()) < rb.end - 3) {
           if (cur[0] == '\r' and cur[1] == '\n') {
#endif
              cur += 2;// skip \r\n
              ctx.add_header_line(cur);
              // If we read \r\n twice the header is complete.
              if (cur[0] == '\r' and cur[1] == '\n')
              {
                complete_header = true;
                cur += 2; // skip \r\n
                header_end = cur - rb.data();
                break;
              }
            } else
              cur++;
          }

          // Read more data from the socket if the headers are not complete.
          if (!complete_header && 0 == rb.read_more(fiber)) return;
        }

        // Header is complete. Process it.
        // Run the handler.
        assert(rb.cursor <= rb.end);
        ctx.body_start = std::string_view(rb.data() + header_end, rb.end - header_end);
        ctx.prepare_request();
        handler(ctx);
        assert(rb.cursor <= rb.end);

        // Update the cursor the beginning of the next request.
        ctx.prepare_next_request();
        // if read buffer is empty, we can flush the output buffer.
        if (rb.empty())
          ctx.flush_responses();
      }
    } catch (const std::runtime_error& e) {
      std::cerr << "Error: " << e.what() << std::endl;
      return;
    }
  };
}

} // namespace http_async_impl
} // namespace li

#include <li/http_server/request.hh>
#include <li/http_server/response.hh>

namespace li {


template <typename... O>
auto http_serve(api<http_request, http_response> api, int port, O... opts) {

  auto options = mmm(opts...);

  int nthreads = get_or(options, s::nthreads, std::thread::hardware_concurrency());

  auto handler = [api](auto& ctx) {
    http_request rq{ctx};
    http_response resp(ctx);
    try {
      api.call(ctx.method(), ctx.url(), rq, resp);
    } catch (const http_error& e) {
      ctx.set_status(e.status());
      ctx.respond(e.what());
    } catch (const std::runtime_error& e) {
      std::cerr << "INTERNAL SERVER ERROR: " << e.what() << std::endl;
      ctx.set_status(500);
      ctx.respond("Internal server error.");
    }
    ctx.respond_if_needed();
  };

  auto date_thread = std::make_shared<std::thread>([&]() {
    while (!quit_signal_catched) {
      li::http_async_impl::http_top_header.tick();
      usleep(1e6);
    }
  });

  auto server_thread = std::make_shared<std::thread>([=]() {
    std::cout << "Starting lithium::http_server on port " << port << std::endl;

    if constexpr (has_key(options, s::ssl_key))
    {
      static_assert(has_key(options, s::ssl_certificate), "You need to provide both the ssl_certificate option and the ssl_key option.");
      std::string ssl_key = options.ssl_key;
      std::string ssl_cert = options.ssl_certificate;
      std::string ssl_ciphers = "";
      if constexpr (has_key(options, s::ssl_ciphers))
      {
        ssl_ciphers = options.ssl_ciphers;
      }
      start_tcp_server(port, SOCK_STREAM, nthreads,
                       http_async_impl::make_http_processor(std::move(handler)),
                       ssl_key, ssl_cert, ssl_ciphers);
    }
    else
      start_tcp_server(port, SOCK_STREAM, nthreads,
                       http_async_impl::make_http_processor(std::move(handler)));
    date_thread->join();
  });

  if constexpr (has_key<decltype(options), s::non_blocking_t>()) {
    usleep(0.1e6);
    date_thread->detach();
    server_thread->detach();
    // return mmm(s::server_thread = server_thread, s::date_thread = date_thread);
  } else
    server_thread->join();
}
} // namespace li
