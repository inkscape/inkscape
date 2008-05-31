# - Try to find LCMS
# Once done this will define
#
#  LCMS_FOUND - system has LCMS
#  LCMS_INCLUDE_DIRS - the LCMS include directory
#  LCMS_LIBRARIES - Link these to use LCMS
#  LCMS_DEFINITIONS - Compiler switches required for using LCMS
#
#  Copyright (c) 2008 Joshua L. Blocher <verbalshadow@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (LCMS_LIBRARIES AND LCMS_INCLUDE_DIRS)
  # in cache already
  set(LCMS_FOUND TRUE)
else (LCMS_LIBRARIES AND LCMS_INCLUDE_DIRS)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  if (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    include(UsePkgConfig)
    pkgconfig(lcms _LCMS_INCLUDEDIR _LCMS_LIBDIR _LCMS_LDFLAGS _LCMS_CFLAGS)
  else (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    find_package(PkgConfig)
    if (PKG_CONFIG_FOUND)
      pkg_check_modules(_LCMS lcms)
    endif (PKG_CONFIG_FOUND)
  endif (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
  find_path(LCMS_INCLUDE_DIR
    NAMES
      lcms.h
    PATHS
      ${_LCMS_INCLUDEDIR}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
      $ENV{DEVLIBS_PATH}//include//
    PATH_SUFFIXES
      lcms
  )

  find_library(LCMS_LIBRARY
    NAMES
      lcms
    PATHS
      ${_LCMS_LIBDIR}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  if (LCMS_LIBRARY)
    set(LCMS_FOUND TRUE)
  endif (LCMS_LIBRARY)

  set(LCMS_INCLUDE_DIRS
    ${LCMS_INCLUDE_DIR}
  )

  if (LCMS_FOUND)
    set(LCMS_LIBRARIES
      ${LCMS_LIBRARIES}
      ${LCMS_LIBRARY}
    )
  endif (LCMS_FOUND)

  if (LCMS_INCLUDE_DIRS AND LCMS_LIBRARIES)
     set(LCMS_FOUND TRUE)
  endif (LCMS_INCLUDE_DIRS AND LCMS_LIBRARIES)

  if (LCMS_FOUND)
    if (NOT LCMS_FIND_QUIETLY)
      message(STATUS "Found LCMS: ${LCMS_LIBRARIES}")
    endif (NOT LCMS_FIND_QUIETLY)
  else (LCMS_FOUND)
    if (LCMS_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find LCMS")
    endif (LCMS_FIND_REQUIRED)
  endif (LCMS_FOUND)

  # show the LCMS_INCLUDE_DIRS and LCMS_LIBRARIES variables only in the advanced view
  mark_as_advanced(LCMS_INCLUDE_DIRS LCMS_LIBRARIES)

endif (LCMS_LIBRARIES AND LCMS_INCLUDE_DIRS)

