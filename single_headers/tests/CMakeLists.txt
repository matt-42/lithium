cmake_minimum_required(VERSION 3.0)

project(li)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/single_headers) # tests include there headers

add_custom_target(
      single_headers_release2
      COMMAND python3 ${CMAKE_SOURCE_DIR}/single_headers/make_single_headers.py ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/single_headers)

function(add_sh_executable target_name)
  add_executable(${target_name} ${ARGN})
  target_link_libraries(${target_name} ${LIBS})
  add_dependencies(${target_name} single_headers_release2 symbols_generation)
endfunction(add_sh_executable)

#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native -mtune=native -flto") 

IF (NOT WIN32)
  add_definitions(-Wno-ignored-attributes) # to skip postgresql warnings.
ENDIF ()

add_sh_executable(json json.cc)
add_sh_executable(http_server http_server.cc)
add_sh_executable(http_client http_client.cc)
add_sh_executable(mysql mysql.cc)
add_sh_executable(pgsql pgsql.cc)
add_sh_executable(sqlite sqlite.cc)
add_sh_executable(symbol symbol.cc)
add_sh_executable(lithium lithium.cc)
add_sh_executable(metamap metamap.cc)
add_sh_executable(check_multiple_definition multiple_units1.cc multiple_units2.cc)
