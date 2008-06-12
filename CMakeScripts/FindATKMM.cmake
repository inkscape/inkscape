# - Try to find ATKMM
# Once done this will define
#
#  ATKMM_FOUND - system has ATKMM
#  ATKMM_INCLUDE_DIRS - the ATKMM include directory
#  ATKMM_LIBRARIES - Link these to use ATKMM
#  ATKMM_DEFINITIONS - Compiler switches required for using ATKMM
#
#  Copyright (c) 2008 Joshua L. Blocher <verbalshadow@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (ATKMM_LIBRARIES AND ATKMM_INCLUDE_DIRS)
  # in cache already
  set(ATKMM_FOUND TRUE)
else (ATKMM_LIBRARIES AND ATKMM_INCLUDE_DIRS)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  if (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    include(UsePkgConfig)
    pkgconfig(atkmm-1.6 _ATKMM_INCLUDEDIR _ATKMM_LIBDIR _ATKMM_LDFLAGS _ATKMM_CFLAGS)
  else (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    find_package(PkgConfig)
    if (PKG_CONFIG_FOUND)
      pkg_check_modules(_ATKMM atkmm-1.6)
    endif (PKG_CONFIG_FOUND)
  endif (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
  find_path(ATKMM_INCLUDE_DIR
    NAMES
      atkmm.h
    PATHS
      ${_ATKMM_INCLUDEDIR}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
    PATH_SUFFIXES
      atkmm-1.6
  )

  find_library(ATKMM-1.6_LIBRARY
    NAMES
      atkmm-1.6
    PATHS
      ${_ATKMM_LIBDIR}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  if (ATKMM-1.6_LIBRARY)
    set(ATKMM-1.6_FOUND TRUE)
  endif (ATKMM-1.6_LIBRARY)

  set(ATKMM_INCLUDE_DIRS
    ${ATKMM_INCLUDE_DIR}
  )

  if (ATKMM-1.6_FOUND)
    set(ATKMM_LIBRARIES
      ${ATKMM_LIBRARIES}
      ${ATKMM-1.6_LIBRARY}
    )
  endif (ATKMM-1.6_FOUND)

  if (ATKMM_INCLUDE_DIRS AND ATKMM_LIBRARIES)
     set(ATKMM_FOUND TRUE)
  endif (ATKMM_INCLUDE_DIRS AND ATKMM_LIBRARIES)

  if (ATKMM_FOUND)
    if (NOT ATKMM_FIND_QUIETLY)
      message(STATUS "Found ATKMM: ${ATKMM_LIBRARIES}")
    endif (NOT ATKMM_FIND_QUIETLY)
  else (ATKMM_FOUND)
    if (ATKMM_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find ATKMM")
    endif (ATKMM_FIND_REQUIRED)
  endif (ATKMM_FOUND)

  # show the ATKMM_INCLUDE_DIRS and ATKMM_LIBRARIES variables only in the advanced view
  mark_as_advanced(ATKMM_INCLUDE_DIRS ATKMM_LIBRARIES)

endif (ATKMM_LIBRARIES AND ATKMM_INCLUDE_DIRS)

