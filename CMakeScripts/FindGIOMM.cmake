# - Try to find GIOMM
# Once done this will define
#
#  GIOMM_FOUND - system has GIOMM
#  GIOMM_INCLUDE_DIRS - the GIOMM include directory
#  GIOMM_LIBRARIES - Link these to use GIOMM
#  GIOMM_DEFINITIONS - Compiler switches required for using GIOMM
#
#  Copyright (c) 2008 Joshua L. Blocher <verbalshadow@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (GIOMM_LIBRARIES AND GIOMM_INCLUDE_DIRS)
  # in cache already
  set(GIOMM_FOUND TRUE)
else (GIOMM_LIBRARIES AND GIOMM_INCLUDE_DIRS)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  if (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    include(UsePkgConfig)
    pkgconfig(giomm-2.4 _GIOMM_INCLUDEDIR _GIOMM_LIBDIR _GIOMM_LDFLAGS _GIOMM_CFLAGS)
  else (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    find_package(PkgConfig)
    if (PKG_CONFIG_FOUND)
      pkg_check_modules(_GIOMM giomm-2.4)
    endif (PKG_CONFIG_FOUND)
  endif (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
  find_path(GIOMM_INCLUDE_DIR
    NAMES
      giomm.h
    PATHS
      ${_GIOMM_INCLUDEDIR}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
    PATH_SUFFIXES
      giomm-2.4
  )

  find_library(GIOMM-2.4_LIBRARY
    NAMES
      giomm-2.4
    PATHS
      ${_GIOMM_LIBDIR}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  if (GIOMM-2.4_LIBRARY)
    set(GIOMM-2.4_FOUND TRUE)
  endif (GIOMM-2.4_LIBRARY)

  set(GIOMM_INCLUDE_DIRS
    ${GIOMM_INCLUDE_DIR}
  )

  if (GIOMM-2.4_FOUND)
    set(GIOMM_LIBRARIES
      ${GIOMM_LIBRARIES}
      ${GIOMM-2.4_LIBRARY}
    )
  endif (GIOMM-2.4_FOUND)

  if (GIOMM_INCLUDE_DIRS AND GIOMM_LIBRARIES)
     set(GIOMM_FOUND TRUE)
  endif (GIOMM_INCLUDE_DIRS AND GIOMM_LIBRARIES)

  if (GIOMM_FOUND)
    if (NOT GIOMM_FIND_QUIETLY)
      message(STATUS "Found GIOMM: ${GIOMM_LIBRARIES}")
    endif (NOT GIOMM_FIND_QUIETLY)
  else (GIOMM_FOUND)
    if (GIOMM_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find GIOMM")
    endif (GIOMM_FIND_REQUIRED)
  endif (GIOMM_FOUND)

  # show the GIOMM_INCLUDE_DIRS and GIOMM_LIBRARIES variables only in the advanced view
  mark_as_advanced(GIOMM_INCLUDE_DIRS GIOMM_LIBRARIES)

endif (GIOMM_LIBRARIES AND GIOMM_INCLUDE_DIRS)

