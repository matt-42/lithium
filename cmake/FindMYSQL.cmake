# - Try to find MySQL.
# Once done this will define:
# MYSQL_FOUND			- If false, do not try to use MySQL.
# MYSQL_INCLUDE_DIRS	- Where to find mysql.h, etc.
# MYSQL_LIBRARIES		- The libraries to link against.
# MYSQL_VERSION_STRING	- Version in a string of MySQL.
#
# Created by RenatoUtsch based on eAthena implementation.
#
# Please note that this module only supports Windows and Linux officially, but
# should work on all UNIX-like operational systems too.
#

#=============================================================================
# Copyright 2012 RenatoUtsch
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

if (CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
	set (X86 TRUE)
	message("X86 detected")
else ()
    set (X86 FALSE)
endif ()

if( WIN32 )
	if (X86)
		file(GLOB MARIADB_LIB_DIR "$ENV{ProgramFiles(x86)}/MariaDB*/lib")
		file(GLOB MARIADB_INC_DIR  "$ENV{ProgramFiles(x86)}/MariaDB*/include/mysql")
		file(GLOB MYSQLDB_LIB_DIR "$ENV{ProgramFiles(x86)}/MySQL/*/lib")
		file(GLOB MYSQLDB_INC_DIR  "$ENV{ProgramFiles(x86)}/MySQL/*/include")
	else()
		file(GLOB MARIADB_LIB_DIR "$ENV{PROGRAMFILES}/MariaDB*/lib")
		file(GLOB MARIADB_INC_DIR  "$ENV{PROGRAMFILES}/MariaDB*/include/mysql")
		file(GLOB MYSQLDB_LIB_DIR "$ENV{PROGRAMFILES}/MySQL/*/lib")
		file(GLOB MYSQLDB_INC_DIR  "$ENV{PROGRAMFILES}/MySQL/*/include")
	endif()

	find_path( MYSQL_INCLUDE_DIR
		NAMES "mysql.h"
		PATHS  ${MARIADB_INC_DIR} ${MYSQLDB_INC_DIR}   )
	
	find_library( MYSQLCLIENT_LIBRARY
	NAMES "mariadbclient" "mysqlclient" "mysqlclient_r"
	PATHS ${MARIADB_LIB_DIR} ${MYSQLDB_LIB_DIR} )
	find_library( MYSQL_LIBRARY
		NAMES "libmariadb" "libmysql"
		PATHS ${MYSQLDB_LIB_DIR} ${MARIADB_LIB_DIRX})
	set(MYSQL_LIBRARY ${MYSQL_LIBRARY} ${MYSQLCLIENT_LIBRARY})
	  
else()
	find_path( MYSQL_INCLUDE_DIR
		NAMES "mysql.h"
		PATHS "/usr/include/mysql/"
			  "/usr/local/include/mysql/"
              "/usr/mysql/include/mysql/" 
              "/usr/local/opt/mysql-client/include/mysql/")
	
	find_library( MYSQL_LIBRARY
		NAMES "mysqlclient" "mysqlclient_r"
		PATHS "/lib/mysql"
			"/lib64/mysql"
			"/usr/lib/mysql"
			"/usr/lib64/mysql"
			"/usr/local/lib/mysql"
			"/usr/local/lib64/mysql"
			"/usr/mysql/lib/mysql"
			"/usr/mysql/lib64/mysql"
			"/usr/local/opt/mysql-client/lib" )
endif()


# if( MYSQL_INCLUDE_DIR AND EXISTS "${MYSQL_INCLUDE_DIRS}/mysql_version.h" )
# 	file( STRINGS "${MYSQL_INCLUDE_DIRS}/mysql_version.h"
# 		MYSQL_VERSION_H REGEX "^#define[ \t]+MYSQL_SERVER_VERSION[ \t]+\"[^\"]+\".*$" )
# 	string( REGEX REPLACE
# 		"^.*MYSQL_SERVER_VERSION[ \t]+\"([^\"]+)\".*$" "\\1" MYSQL_VERSION_STRING
# 		"${MYSQL_VERSION_H}" )
# endif()

# handle the QUIETLY and REQUIRED arguments and set MYSQL_FOUND to TRUE if
# all listed variables are TRUE
include( FindPackageHandleStandardArgs )
find_package_handle_standard_args( MYSQL DEFAULT_MSG
		MYSQL_LIBRARY MYSQL_INCLUDE_DIR)

set( MYSQL_INCLUDE_DIRS ${MYSQL_INCLUDE_DIR} )

mark_as_advanced( MYSQL_INCLUDE_DIR MYSQL_LIBRARY )
