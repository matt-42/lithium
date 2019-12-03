cmake_minimum_required(VERSION 2.8)

project(json)

enable_testing()

add_executable(json_decode tests/decode.cc)
add_test(json_decode json_decode)

add_executable(json_encode tests/encode.cc)
add_test(json_encode json_encode)

add_executable(json_unicode tests/unicode.cc)
add_test(json_unicode json_unicode)

add_executable(readme tests/readme.cc)

add_executable(number_parser tests/number_parser.cc)
add_test(number_parser number_parser)

install(DIRECTORY json DESTINATION include/li
  FILES_MATCHING PATTERN "*.hh")
install(DIRECTORY json DESTINATION include/li
  FILES_MATCHING PATTERN "*.hpp")
