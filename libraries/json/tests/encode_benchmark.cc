

#include <iostream>
#include <li/http_server/timer.hh>
#include <lithium_json.hh>
#include <lithium_http_server.hh>
// #include <fmt/format.h>
#include "symbols.hh"

using namespace li;

int main() {
  timer timer;
  timer.start();

  auto json_stream = output_buffer(
      50 * 1024, [&](const char* d, int s) { });

  int N = 10000000;
  for (int i = 0; i < N; i++)
  {
    json_encode(json_stream, mmm(s::message = 42));
    json_stream.reset();
  }
  timer.end();
  std::cout << (N / timer.ms())  << " encode/ms" << std::endl;

  timer.start();
  for (int i = 0; i < N; i++)
  {
    json_stream << "{\"message\":" << 42 << "}";
    json_stream.reset();
  }
  timer.end();
  std::cout << (N / timer.ms())  << " encode/ms" << std::endl;

  // fmt::memory_buffer out;
  // timer.start();
  // for (int i = 0; i < N; i++)
  // {
  //   fmt::format_to(out, "{{\"message\":{}}}", 42);
  //   out.clear();
  // }
  // timer.end();
  // std::cout << (N / timer.ms())  << " encode/ms" << std::endl;

}