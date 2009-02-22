# - Try to find SSL
# Once done this will define
#
#  SSL_FOUND - system has SSL
#  SSL_INCLUDE_DIRS - the SSL include directory
#  SSL_LIBRARIES - Link these to use SSL
#  SSL_DEFINITIONS - Compiler switches required for using SSL
#
#  Copyright (c) 2008 Joshua L. Blocher <verbalshadow@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (SSL_LIBRARIES AND SSL_INCLUDE_DIRS)
  # in cache already
  set(SSL_FOUND TRUE)
else (SSL_LIBRARIES AND SSL_INCLUDE_DIRS)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  if (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    include(UsePkgConfig)
    pkgconfig(libssl _SSL_INCLUDEDIR _SSL_LIBDIR _SSL_LDFLAGS _SSL_CFLAGS)
  else (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    find_package(PkgConfig)
    if (PKG_CONFIG_FOUND)
      pkg_check_modules(_SSL libssl)
    endif (PKG_CONFIG_FOUND)
  endif (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
  find_path(SSL_INCLUDE_DIR
    NAMES
      ssl.h
    PATHS
      ${_SSL_INCLUDEDIR}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
    PATH_SUFFIXES
      openssl
  )

  find_library(SSL_LIBRARY
    NAMES
      ssl
    PATHS
      ${_SSL_LIBDIR}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  if (SSL_LIBRARY)
    set(SSL_FOUND TRUE)
  endif (SSL_LIBRARY)

  set(SSL_INCLUDE_DIRS
    ${SSL_INCLUDE_DIR}
  )

  if (SSL_FOUND)
    set(SSL_LIBRARIES
      ${SSL_LIBRARIES}
      ${SSL_LIBRARY}
    )
  endif (SSL_FOUND)

  if (SSL_INCLUDE_DIRS AND SSL_LIBRARIES)
     set(SSL_FOUND TRUE)
  endif (SSL_INCLUDE_DIRS AND SSL_LIBRARIES)

  if (SSL_FOUND)
    if (NOT SSL_FIND_QUIETLY)
      message(STATUS "Found SSL: ${SSL_LIBRARIES}")
    endif (NOT SSL_FIND_QUIETLY)
  else (SSL_FOUND)
    if (SSL_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find SSL")
    endif (SSL_FIND_REQUIRED)
  endif (SSL_FOUND)

  # show the SSL_INCLUDE_DIRS and SSL_LIBRARIES variables only in the advanced view
  mark_as_advanced(SSL_INCLUDE_DIRS SSL_LIBRARIES)

endif (SSL_LIBRARIES AND SSL_INCLUDE_DIRS)

