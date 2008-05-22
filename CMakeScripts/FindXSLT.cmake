# - Try to find XSLT
# Once done this will define
#
#  XSLT_FOUND - system has XSLT
#  XSLT_INCLUDE_DIRS - the XSLT include directory
#  XSLT_LIBRARIES - Link these to use XSLT
#  XSLT_DEFINITIONS - Compiler switches required for using XSLT
#
#  Copyright (c) 2008 Joshua L. Blocher <verbalshadow@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (XSLT_LIBRARIES AND XSLT_INCLUDE_DIRS)
  # in cache already
  set(XSLT_FOUND TRUE)
else (XSLT_LIBRARIES AND XSLT_INCLUDE_DIRS)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  if (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    include(UsePkgConfig)
    pkgconfig(libxslt _XSLT_INCLUDEDIR _XSLT_LIBDIR _XSLT_LDFLAGS _XSLT_CFLAGS)
  else (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    find_package(PkgConfig)
    if (PKG_CONFIG_FOUND)
      pkg_check_modules(_XSLT libxslt)
    endif (PKG_CONFIG_FOUND)
  endif (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
  find_path(XSLT_INCLUDE_DIR
    NAMES
      xslt.h
    PATHS
      ${_XSLT_INCLUDEDIR}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
    PATH_SUFFIXES
      libxslt
  )

  find_library(XSLT_LIBRARY
    NAMES
      xslt
    PATHS
      ${_XSLT_LIBDIR}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  if (XSLT_LIBRARY)
    set(XSLT_FOUND TRUE)
  endif (XSLT_LIBRARY)

  set(XSLT_INCLUDE_DIRS
    ${XSLT_INCLUDE_DIR}
  )

  if (XSLT_FOUND)
    set(XSLT_LIBRARIES
      ${XSLT_LIBRARIES}
      ${XSLT_LIBRARY}
    )
  endif (XSLT_FOUND)

  if (XSLT_INCLUDE_DIRS AND XSLT_LIBRARIES)
     set(XSLT_FOUND TRUE)
  endif (XSLT_INCLUDE_DIRS AND XSLT_LIBRARIES)

  if (XSLT_FOUND)
    if (NOT XSLT_FIND_QUIETLY)
      message(STATUS "Found XSLT: ${XSLT_LIBRARIES}")
    endif (NOT XSLT_FIND_QUIETLY)
  else (XSLT_FOUND)
    if (XSLT_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find XSLT")
    endif (XSLT_FIND_REQUIRED)
  endif (XSLT_FOUND)

  # show the XSLT_INCLUDE_DIRS and XSLT_LIBRARIES variables only in the advanced view
  mark_as_advanced(XSLT_INCLUDE_DIRS XSLT_LIBRARIES)

endif (XSLT_LIBRARIES AND XSLT_INCLUDE_DIRS)

