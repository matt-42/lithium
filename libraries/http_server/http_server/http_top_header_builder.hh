

// #ifdef LITHIUM_SERVER_NAME
//   #define MACRO_TO_STR2(L) #L
//   #define MACRO_TO_STR(L) MACRO_TO_STR2(L)

//   #define LITHIUM_SERVER_NAME_HEADER "Server: " MACRO_TO_STR(LITHIUM_SERVER_NAME) "\r\n"

//   #undef MACRO_TO_STR
//   #undef MACRO_TO_STR2
// #else
//   #define LITHIUM_SERVER_NAME_HEADER "Server: Lithium\r\n"
// #endif

struct http_top_header_builder {

  std::string_view top_header() { return std::string_view(tmp2, top_header_size); }; 
  std::string_view top_header_200() { return std::string_view(tmp2_200, top_header_200_size); }; 

  void tick() {
    time_t t = time(NULL);
    struct tm tm;
    gmtime_r(&t, &tm);

    top_header_size = strftime(tmp1, sizeof(tmp1), 
#ifdef LITHIUM_SERVER_NAME
  #define MACRO_TO_STR2(L) #L
  #define MACRO_TO_STR(L) MACRO_TO_STR2(L)
  "\r\nServer: " MACRO_TO_STR(LITHIUM_SERVER_NAME) "\r\n"

  #undef MACRO_TO_STR
  #undef MACRO_TO_STR2
#else
  "\r\nServer: Lithium\r\n"
#endif
      "Date: %a, %d %b %Y %T GMT\r\n", &tm);
    std::swap(tmp1, tmp2);

    top_header_200_size = strftime(tmp1_200, sizeof(tmp1_200), 
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
      "Date: %a, %d %b %Y %T GMT\r\n", &tm);

    std::swap(tmp1_200, tmp2_200);
  }


  char tmp1[150];
  char tmp2[150];
  int top_header_size;

  char tmp1_200[150];
  char tmp2_200[150];
  int top_header_200_size;
};

#undef LITHIUM_SERVER_NAME_HEADER
