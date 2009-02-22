# - Try to find GSL
# Once done this will define
#
#  GSL_FOUND - system has GSL
#  GSL_INCLUDE_DIRS - the GSL include directory
#  GSL_LIBRARIES - Link these to use GSL
#  GSL_DEFINITIONS - Compiler switches required for using GSL
#
#  Copyright (c) 2008 Joshua L. Blocher <verbalshadow@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (GSL_LIBRARIES AND GSL_INCLUDE_DIRS)
  # in cache already
  set(GSL_FOUND TRUE)
else (GSL_LIBRARIES AND GSL_INCLUDE_DIRS)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  if (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    include(UsePkgConfig)
    pkgconfig(gsl _GSL_INCLUDEDIR _GSL_LIBDIR _GSL_LDFLAGS _GSL_CFLAGS)
  else (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    find_package(PkgConfig)
    if (PKG_CONFIG_FOUND)
      pkg_check_modules(_GSL gsl)
    endif (PKG_CONFIG_FOUND)
  endif (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
  find_path(GSL_INCLUDE_DIR
    NAMES
      gsl_blas.h
    PATHS
      ${_GSL_INCLUDEDIR}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
    PATH_SUFFIXES
      gsl
  )

  find_library(GSL_LIBRARY
    NAMES
      gsl
    PATHS
      ${_GSL_LIBDIR}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  if (GSL_LIBRARY)
    set(GSL_FOUND TRUE)
  endif (GSL_LIBRARY)

  set(GSL_INCLUDE_DIRS
    ${GSL_INCLUDE_DIR}
  )

  if (GSL_FOUND)
    set(GSL_LIBRARIES
      ${GSL_LIBRARIES}
      ${GSL_LIBRARY}
    )
  endif (GSL_FOUND)

  if (GSL_INCLUDE_DIRS AND GSL_LIBRARIES)
     set(GSL_FOUND TRUE)
  endif (GSL_INCLUDE_DIRS AND GSL_LIBRARIES)

  if (GSL_FOUND)
    if (NOT GSL_FIND_QUIETLY)
      message(STATUS "Found GSL: ${GSL_LIBRARIES}")
    endif (NOT GSL_FIND_QUIETLY)
  else (GSL_FOUND)
    if (GSL_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find GSL")
    endif (GSL_FIND_REQUIRED)
  endif (GSL_FOUND)

  # show the GSL_INCLUDE_DIRS and GSL_LIBRARIES variables only in the advanced view
  mark_as_advanced(GSL_INCLUDE_DIRS GSL_LIBRARIES)

endif (GSL_LIBRARIES AND GSL_INCLUDE_DIRS)

