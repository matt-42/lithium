#pragma once

#include <time.h>

namespace internal {

struct double_buffer {

  double_buffer() {
    this->p1 = this->b1;
    this->p2 = this->b2;
  }

  void swap() { std::swap(this->p1, this->p2); }

  char* current_buffer() { return this->p1; }
  char* next_buffer() { return this->p2; }
  int size() { return 150; }

  char* p1;
  char* p2;
  char b1[150];
  char b2[150];
};

} // namespace internal

struct http_top_header_builder {

  std::string_view top_header() { return std::string_view(tmp.current_buffer(), top_header_size); };
  std::string_view top_header_200() {
    return std::string_view(tmp_200.current_buffer(), top_header_200_size);
  };

  void tick() {
    time_t t = time(NULL);
    struct tm tm;
#if defined _WIN32
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif

    top_header_size =
        int(strftime(tmp.next_buffer(), tmp.size(),
#ifdef LITHIUM_SERVER_NAME
#define MACRO_TO_STR2(L) #L
#define MACRO_TO_STR(L) MACRO_TO_STR2(L)
                 "\r\nServer: " MACRO_TO_STR(LITHIUM_SERVER_NAME) "\r\n"

#undef MACRO_TO_STR
#undef MACRO_TO_STR2
#else
                 "\r\nServer: Lithium\r\n"
#endif
                                                                  "Date: %a, %d %b %Y %T GMT\r\n",
                 &tm));
    tmp.swap();

    top_header_200_size =
        int(strftime(tmp_200.next_buffer(), tmp_200.size(),
                 "HTTP/1.1 200 OK\r\n"
#ifdef LITHIUM_SERVER_NAME
#define MACRO_TO_STR2(L) #L
#define MACRO_TO_STR(L) MACRO_TO_STR2(L)
                 "Server: " MACRO_TO_STR(LITHIUM_SERVER_NAME) "\r\n"

#undef MACRO_TO_STR
#undef MACRO_TO_STR2
#else
                 "Server: Lithium\r\n"
#endif

                                                              // LITHIUM_SERVER_NAME_HEADER
                                                              "Date: %a, %d %b %Y %T GMT\r\n",
                 &tm));

    tmp_200.swap();
  }

  internal::double_buffer tmp;
  int top_header_size;

  internal::double_buffer tmp_200;
  int top_header_200_size;
};

#undef LITHIUM_SERVER_NAME_HEADER
