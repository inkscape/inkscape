# - Try to find CAIROMM
# Once done this will define
#
#  CAIROMM_FOUND - system has CAIROMM
#  CAIROMM_INCLUDE_DIRS - the CAIROMM include directory
#  CAIROMM_LIBRARIES - Link these to use CAIROMM
#  CAIROMM_DEFINITIONS - Compiler switches required for using CAIROMM
#
#  Copyright (c) 2008 Joshua L. Blocher <verbalshadow@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (CAIROMM_LIBRARIES AND CAIROMM_INCLUDE_DIRS)
  # in cache already
  set(CAIROMM_FOUND TRUE)
else (CAIROMM_LIBRARIES AND CAIROMM_INCLUDE_DIRS)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  if (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    include(UsePkgConfig)
    pkgconfig(cairomm-1.0 _CAIROMM_INCLUDEDIR _CAIROMM_LIBDIR _CAIROMM_LDFLAGS _CAIROMM_CFLAGS)
  else (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    find_package(PkgConfig)
    if (PKG_CONFIG_FOUND)
      pkg_check_modules(_CAIROMM cairomm-1.0)
    endif (PKG_CONFIG_FOUND)
  endif (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
  find_path(CAIROMM_INCLUDE_DIR
    NAMES
      cairomm/cairomm.h
    PATHS
      ${_CAIROMM_INCLUDEDIR}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
    PATH_SUFFIXES
      cairomm-1.0
  )

  find_library(CAIROMM-1.0_LIBRARY
    NAMES
      cairomm-1.0
    PATHS
      ${_CAIROMM_LIBDIR}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  if (CAIROMM-1.0_LIBRARY)
    set(CAIROMM-1.0_FOUND TRUE)
  endif (CAIROMM-1.0_LIBRARY)

  set(CAIROMM_INCLUDE_DIRS
    ${CAIROMM_INCLUDE_DIR}
  )

  if (CAIROMM-1.0_FOUND)
    set(CAIROMM_LIBRARIES
      ${CAIROMM_LIBRARIES}
      ${CAIROMM-1.0_LIBRARY}
    )
  endif (CAIROMM-1.0_FOUND)

  if (CAIROMM_INCLUDE_DIRS AND CAIROMM_LIBRARIES)
     set(CAIROMM_FOUND TRUE)
  endif (CAIROMM_INCLUDE_DIRS AND CAIROMM_LIBRARIES)

  if (CAIROMM_FOUND)
    if (NOT CAIROMM_FIND_QUIETLY)
      message(STATUS "Found CAIROMM: ${CAIROMM_LIBRARIES}")
    endif (NOT CAIROMM_FIND_QUIETLY)
  else (CAIROMM_FOUND)
    if (CAIROMM_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find CAIROMM")
    endif (CAIROMM_FIND_REQUIRED)
  endif (CAIROMM_FOUND)

  # show the CAIROMM_INCLUDE_DIRS and CAIROMM_LIBRARIES variables only in the advanced view
  mark_as_advanced(CAIROMM_INCLUDE_DIRS CAIROMM_LIBRARIES)

endif (CAIROMM_LIBRARIES AND CAIROMM_INCLUDE_DIRS)

