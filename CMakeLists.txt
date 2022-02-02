cmake_minimum_required(VERSION 3.0)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                      ${CMAKE_SOURCE_DIR}/cmake)

project(lithium)

include(FetchContent)

set(CMAKE_VERBOSE_MAKEFILE ON)

# On MacOS use -DCMAKE_PREFIX_PATH='/usr/local/opt/openssl/;/usr/local/opt/curl;/usr/local/opt/libpq'
# to have find_package look into brew installed locations first.

# On windows with vcpkg, use
# cmake ..\lithium -DCMAKE_TOOLCHAIN_FILE="C:\vcpkg\scripts\buildsystems\vcpkg.cmake" -DCMAKE_PREFIX_PATH="C:\vcpkg\installed\x86-windows"




find_package(Boost REQUIRED context)
#find_package(SQLite3 REQUIRED)
find_package(CURL REQUIRED)
find_package(PostgreSQL REQUIRED)
find_package(Threads REQUIRED)
set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)
find_package(unofficial-libmariadb CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)

IF (WIN32)
find_package(Wepoll REQUIRED)
set(WINDOWS_LIBS wsock32 ws2_32)
add_definitions(-DNOMINMAX)
ENDIF()

include_directories(${Wepoll_INCLUDE_DIR})

message("WINDOWS LIBS: " ${Wepoll_LIBRARY} " -- " ${WINDOWS_LIBS})
set(LIBS ${CMAKE_THREAD_LIBS_INIT} ${Wepoll_LIBRARY} ${WINDOWS_LIBS})
set(LIBS ${LIBS} CURL::libcurl
        OpenSSL::SSL OpenSSL::Crypto 
        unofficial::libmariadb 
        unofficial::sqlite3::sqlite3
        PostgreSQL::PostgreSQL
        Boost::boost
        Boost::context)


if (APPLE)
  # needed by mariadbclient on macos:
#  set(LIBS ${LIBS} z iconv)
endif()

set(CMAKE_CXX_STANDARD 20)

enable_testing()
list(APPEND CMAKE_CTEST_ARGUMENTS "--output-on-failure")

# Create the include dir in the build directory.
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include/li)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/single_headers)

function (link_build_include lib_name target_dir)
  IF (WIN32)
      STRING(REGEX REPLACE "/" "\\\\"  CMAKE_SOURCE_DIR_BACKSLASH ${CMAKE_SOURCE_DIR}) 
      STRING(REGEX REPLACE "/" "\\\\"  TARGET_BS ${target_dir}) 
      message("CREATE INCLUDE DIRECTORY " ${target_dir})
      execute_process(COMMAND cmd.exe /c mklink /J ${lib_name}
      "${CMAKE_SOURCE_DIR_BACKSLASH}${TARGET_BS}"
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/include/li/)
  ELSE()
    if(NOT EXISTS "${CMAKE_BINARY_DIR}/include/li/${lib_name}")
    execute_process(COMMAND ln -s
                    "${CMAKE_SOURCE_DIR}${target_dir}" ${lib_name}
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/include/li/)
    ENDIF()
  ENDIF()
endfunction()

# Create the li/{lib}/header tree to build the tests.
link_build_include("metamap" "/libraries/metamap/metamap")
link_build_include("json" "/libraries/json/json")
link_build_include("sqlite" "/libraries/sqlite")
link_build_include("callable_traits" "/libraries/callable_traits")
link_build_include("http_server" "/libraries/http_server/http_server")
link_build_include("http_client" "/libraries/http_client/http_client")
link_build_include("symbol" "/libraries/symbol/symbol")
link_build_include("sql" "/libraries/sql/sql")                               

include_directories(${CMAKE_BINARY_DIR}/single_headers) # tests include there headers
include_directories(${CMAKE_BINARY_DIR}/include) # just to enable vscode intellisense.

#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native -mtune=native -flto") 
if(NOT WIN32)
add_definitions(-Wno-ignored-attributes) # to skip postgresql warnings.
endif()

########
# Generate content type definition header file
########
set(GENERATED_CONTENT_TYPES "${CMAKE_SOURCE_DIR}/libraries/http_server/http_server/content_types.hh")

file(DOWNLOAD http://svn.apache.org/repos/asf/httpd/httpd/trunk/docs/conf/mime.types /tmp/lithium_mime.types)
file(READ  /tmp/lithium_mime.types mimeTypes)
foreach(X  RANGE 1 20)
  string(REGEX REPLACE "([^\t\n]+)[\t]+([^\n ]+) ([^\n]+)\n" "\\1\t\\2\n\\1\t\\3\n" mimeTypes "${mimeTypes}")
endforeach()
string(REGEX REPLACE "([^\t\n]+)[\t]+([^\n]+)" "{\"\\2\", \"\\1\"}," mimeTypes "${mimeTypes}")
string(REGEX REPLACE "[^\n]*#[^\n]*\n" "" mimeTypes "${mimeTypes}")
set(mimeTypes "// This file is generated do not edit it.\n"
"#pragma once\n"
"#include <unordered_map>\n"
"#include <string_view>\n"
"namespace li {\n"
"static std::unordered_map<std::string_view, std::string_view> content_types = {\n"
${mimeTypes}
"}\;}\n"
)
file(WRITE ${GENERATED_CONTENT_TYPES} ${mimeTypes})


########
# End of type definition header file
########

# Automatic symbol generation
add_custom_target(
    symbols_generation DEPENDS li_symbol_generator
    COMMAND $<TARGET_FILE:li_symbol_generator> ${CMAKE_CURRENT_SOURCE_DIR}/libraries ${CMAKE_CURRENT_SOURCE_DIR}/single_headers ${CMAKE_CURRENT_SOURCE_DIR}/docs)


# Automatic single header generation for release (without line directives)
add_custom_target(
      single_headers_release
      COMMAND python3 ${CMAKE_SOURCE_DIR}/single_headers/make_single_headers.py ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/single_headers)

# Automatic single header generation (for tests with line directives to map compile error to original headers)
add_custom_target(
  single_headers DEPENDS single_headers_release
  COMMAND python3 ${CMAKE_SOURCE_DIR}/single_headers/make_single_headers.py --with-line-directives ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/single_headers)

# add_library(precompiled_header_target ${CMAKE_SOURCE_DIR}/single_headers/test.cc)
# target_precompile_headers(precompiled_header_target 
#   PUBLIC
#   <lithium.hh>)

function(li_add_executable target_name)
  add_executable(${target_name} ${ARGN})
  target_link_libraries(${target_name} ${LIBS})
  add_dependencies(${target_name} symbols_generation single_headers)
  # target_precompile_headers(${target_name}  REUSE_FROM precompiled_header_target)
endfunction(li_add_executable)

add_subdirectory(docs)
add_subdirectory(libraries/metamap)
add_subdirectory(libraries/callable_traits)
add_subdirectory(libraries/json)
add_subdirectory(libraries/symbol)
add_subdirectory(libraries/http_server)
add_subdirectory(libraries/http_client)
add_subdirectory(libraries/sql)
add_subdirectory(single_headers/tests)

install(DIRECTORY single_headers/ DESTINATION include FILES_MATCHING PATTERN "*.hh")
