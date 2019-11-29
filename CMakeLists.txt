cmake_minimum_required(VERSION 2.8)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                      ${CMAKE_SOURCE_DIR}/cmake)

project(iod)

set(CMAKE_CXX_STANDARD 17)

enable_testing()

include_directories(${CMAKE_BINARY_DIR}/include)

IF (WIN32)
    set(bash_path  "C:/Program Files/Git/bin/bash")
ELSE()
    set(bash_path sh)
ENDIF()

add_custom_target(prepare_include_dir ALL
    COMMAND ${bash_path} ${CMAKE_SOURCE_DIR}/tools/prepare_include_directory.sh ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}/include)

  
add_subdirectory(libraries/metamap)
add_subdirectory(libraries/metajson)
add_subdirectory(libraries/symbol)
add_subdirectory(libraries/silicon)
add_subdirectory(libraries/http_client)
add_subdirectory(libraries/sqlite)
