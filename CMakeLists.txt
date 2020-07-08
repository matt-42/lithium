cmake_minimum_required(VERSION 2.8)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                      ${CMAKE_SOURCE_DIR}/cmake)

project(li)

find_package(MYSQL REQUIRED)
find_package(PostgreSQL REQUIRED)
find_package(Boost REQUIRED filesystem context)
find_package(SQLite3 REQUIRED)
find_package(CURL REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(${SQLite3_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS} ${MICROHTTPD_INCLUDE_DIRS} ${MYSQL_INCLUDE_DIR})
include_directories(${PostgreSQL_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})

set(LIBS ${MICROHTTPD_LIBRARIES} ${SQLite3_LIBRARIES} ${CURL_LIBRARIES} ${MYSQL_LIBRARY} ${PostgreSQL_LIBRARIES} 
          ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${OPENSSL_LIBRARIES})

set(CMAKE_CXX_STANDARD 17)

enable_testing()

# Create the include dir in the build directory.
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include/li)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/single_headers)

function (link_build_include lib_name target_dir)
IF (WIN32)
    STRING(REGEX REPLACE "/" "\\\\"  CMAKE_SOURCE_DIR_BACKSLASH ${CMAKE_SOURCE_DIR}) 
    STRING(REGEX REPLACE "/" "\\\\"  TARGET_BS ${target_dir}) 
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
link_build_include("http_backend" "/libraries/http_backend/http_backend")
link_build_include("http_client" "/libraries/http_client/http_client")
link_build_include("symbol" "/libraries/symbol/symbol")
link_build_include("sql" "/libraries/sql/sql")                               

include_directories(${CMAKE_BINARY_DIR}/single_headers) # tests include there headers
include_directories(${CMAKE_BINARY_DIR}/include) # just to enable vscode intellisense.

#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native -mtune=native -flto") 
add_definitions(-Wno-ignored-attributes) # to skip postgresql warnings.

# Automatic symbol generation
add_custom_target(
    symbols_generation DEPENDS li_symbol_generator
    COMMAND ${CMAKE_BINARY_DIR}/libraries/symbol/li_symbol_generator ${CMAKE_CURRENT_SOURCE_DIR}/libraries)

# Automatic single header generation (for tests with line directives to map compile error to original headers)
add_custom_target(
      single_headers
      COMMAND python3 ${CMAKE_SOURCE_DIR}/single_headers/make_single_headers.py --with-line-directives ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/single_headers)

# Automatic single header generation for release (without line directives)
add_custom_target(
      single_headers_release
      COMMAND python3 ${CMAKE_SOURCE_DIR}/single_headers/make_single_headers.py ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/single_headers)

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

add_subdirectory(libraries/metamap)
add_subdirectory(libraries/callable_traits)
add_subdirectory(libraries/json)
add_subdirectory(libraries/symbol)
add_subdirectory(libraries/http_backend)
add_subdirectory(libraries/http_client)
add_subdirectory(libraries/sql)

install(DIRECTORY single_headers/ DESTINATION include FILES_MATCHING PATTERN "*.hh")
