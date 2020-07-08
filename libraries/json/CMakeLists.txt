cmake_minimum_required(VERSION 2.8)

project(json)

enable_testing()


li_add_executable(graphana tests/graphana.cc)
add_test(graphana graphana)

li_add_executable(json_decode tests/decode.cc)
add_test(json_decode json_decode)

li_add_executable(json_encode tests/encode.cc)
add_test(json_encode json_encode)

li_add_executable(json_unicode tests/unicode.cc)
add_test(json_unicode json_unicode)

li_add_executable(readme tests/readme.cc)

li_add_executable(number_parser tests/number_parser.cc)
add_test(number_parser number_parser)

