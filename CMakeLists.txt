cmake_minimum_required(VERSION 2.8)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                      ${CMAKE_SOURCE_DIR}/cmake)

project(li)

set(CMAKE_CXX_STANDARD 17)

enable_testing()

# Create the include dir in the build directory.
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include/li)

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

link_build_include("metamap" "/libraries/metamap/metamap")
link_build_include("json" "/libraries/json/json")
link_build_include("di" "/libraries/di")
link_build_include("sqlite" "/libraries/sqlite")
link_build_include("callable_traits" "/libraries/callable_traits")
link_build_include("http_backend" "/libraries/http_backend/http_backend")
link_build_include("http_client" "/libraries/http_client/http_client")
link_build_include("symbol" "/libraries/symbol/symbol")
link_build_include("sql" "/libraries/sql/sql")                               

include_directories(${CMAKE_BINARY_DIR}/include)

add_subdirectory(libraries/metamap)
add_subdirectory(libraries/json)
add_subdirectory(libraries/symbol)
add_subdirectory(libraries/http_backend)
add_subdirectory(libraries/http_client)
add_subdirectory(libraries/sql)
