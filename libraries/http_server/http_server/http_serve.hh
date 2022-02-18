#pragma once

#include <cstring>
#include <functional>
#include <iostream>
#include <string_view>
#include <sys/stat.h>
#include <sys/types.h>
#if not defined(_WIN32)
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>
#endif

#include <unordered_map>

#include <boost/lexical_cast.hpp>

#include <li/http_server/error.hh>
#include <li/http_server/http_top_header_builder.hh>
#include <li/http_server/input_buffer.hh>
#include <li/http_server/output_buffer.hh>
#include <li/http_server/symbols.hh>
#include <li/http_server/tcp_server.hh>
#include <li/http_server/url_unescape.hh>

#include <li/http_server/content_types.hh>
#include <li/http_server/http_ctx.hh>
#include <li/http_server/request.hh>
#include <li/http_server/response.hh>

namespace li {

template <typename... O>
void http_serve(api<http_request, http_response> api, int port, O... opts) {

  auto options = mmm(opts...);

  int nthreads = get_or(options, s::nthreads, std::thread::hardware_concurrency());

  std::string ip = get_or(options, s::ip, "");

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
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  });

  auto server_thread = std::make_shared<std::thread>([=] {
    std::cout << "Starting lithium::http_server on port " << port << std::endl;

    if constexpr (has_key<decltype(options)>(s::ssl_key)) {
      static_assert(has_key<decltype(options)>(s::ssl_certificate),
                    "You need to provide both the ssl_certificate option and the ssl_key option.");
      std::string ssl_key = options.ssl_key;
      std::string ssl_cert = options.ssl_certificate;
      std::string ssl_ciphers = get_or(options, s::ssl_ciphers, "");
      start_tcp_server(ip, port, SOCK_STREAM, nthreads,
                       http_async_impl::make_http_processor(std::move(handler)), ssl_key,
                       ssl_cert, ssl_ciphers);
    } else {
      start_tcp_server(ip, port, SOCK_STREAM, nthreads,
                       http_async_impl::make_http_processor(std::move(handler)));
    }
    date_thread->join();
  });

  if (has_key<decltype(options), s::non_blocking_t>()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    date_thread->detach();
    server_thread->detach();
    // return mmm(s::server_thread = server_thread, s::date_thread = date_thread);
  } else
    server_thread->join();
}

} // namespace li
