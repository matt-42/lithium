
# INPUTS:

# MICROHTTPD_ROOT_DIR

# OUTPUTS:

# MICROHTTPD_FOUND        - system has microhttpd developement header
# MICROHTTPD_INCLUDE_DIRS - microhttpd include directories
# MICROHTTPD_LIBRARIES    - libraries needed to use microhttpd

include(FindPackageHandleStandardArgs)

if(MICROHTTPD_INCLUDE_DIRS AND MICROHTTPD_LIBRARIES)
  set(MICROHTTPD_FIND_QUIETLY TRUE)
else()
  find_path(
    MICROHTTPD_INCLUDE_DIR
    NAMES microhttpd.h
    HINTS ${MICROHTTPD_ROOT_DIR}
    PATH_SUFFIXES include "")

  find_library(
    MICROHTTPD_LIBRARY
    NAMES microhttpd libmicrohttpd libmicrohttpd_d
    HINTS ${MICROHTTPD_ROOT_DIR}
    PATH_SUFFIXES ${LIBRARY_PATH_PREFIX} "")

    set(MICROHTTPD_INCLUDE_DIRS ${MICROHTTPD_INCLUDE_DIR})
  set(MICROHTTPD_LIBRARIES ${MICROHTTPD_LIBRARY})
  message(${MICROHTTPD_LIBRARIES})
  message(${MICROHTTPD_INCLUDE_DIRS})

  find_package_handle_standard_args(
    microhttpd
    DEFAULT_MSG
    MICROHTTPD_LIBRARY MICROHTTPD_INCLUDE_DIR)

  mark_as_advanced(MICROHTTPD_LIBRARY MICROHTTPD_INCLUDE_DIR)
endif()

if (NOT MICROHTTPD_FOUND)
  message (STATUS "MicroHttpd not found, no httpd access available.")
endif()
