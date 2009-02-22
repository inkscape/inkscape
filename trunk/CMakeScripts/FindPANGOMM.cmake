# - Try to find PANGOMM
# Once done this will define
#
#  PANGOMM_FOUND - system has PANGOMM
#  PANGOMM_INCLUDE_DIRS - the PANGOMM include directory
#  PANGOMM_LIBRARIES - Link these to use PANGOMM
#  PANGOMM_DEFINITIONS - Compiler switches required for using PANGOMM
#
#  Copyright (c) 2008 Joshua L. Blocher <verbalshadow@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (PANGOMM_LIBRARIES AND PANGOMM_INCLUDE_DIRS)
  # in cache already
  set(PANGOMM_FOUND TRUE)
else (PANGOMM_LIBRARIES AND PANGOMM_INCLUDE_DIRS)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  if (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    include(UsePkgConfig)
    pkgconfig(pangomm-1.4 _PANGOMM_INCLUDEDIR _PANGOMM_LIBDIR _PANGOMM_LDFLAGS _PANGOMM_CFLAGS)
  else (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    find_package(PkgConfig)
    if (PKG_CONFIG_FOUND)
      pkg_check_modules(_PANGOMM pangomm-1.4)
    endif (PKG_CONFIG_FOUND)
  endif (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
  find_path(PANGOMM_INCLUDE_DIR
    NAMES
      pangomm.h
    PATHS
      ${_PANGOMM_INCLUDEDIR}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
    PATH_SUFFIXES
      pangomm-1.4
  )

  find_library(PANGOMM-1.4_LIBRARY
    NAMES
      pangomm-1.4
    PATHS
      ${_PANGOMM_LIBDIR}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  if (PANGOMM-1.4_LIBRARY)
    set(PANGOMM-1.4_FOUND TRUE)
  endif (PANGOMM-1.4_LIBRARY)

  set(PANGOMM_INCLUDE_DIRS
    ${PANGOMM_INCLUDE_DIR}
  )

  if (PANGOMM-1.4_FOUND)
    set(PANGOMM_LIBRARIES
      ${PANGOMM_LIBRARIES}
      ${PANGOMM-1.4_LIBRARY}
    )
  endif (PANGOMM-1.4_FOUND)

  if (PANGOMM_INCLUDE_DIRS AND PANGOMM_LIBRARIES)
     set(PANGOMM_FOUND TRUE)
  endif (PANGOMM_INCLUDE_DIRS AND PANGOMM_LIBRARIES)

  if (PANGOMM_FOUND)
    if (NOT PANGOMM_FIND_QUIETLY)
      message(STATUS "Found PANGOMM: ${PANGOMM_LIBRARIES}")
    endif (NOT PANGOMM_FIND_QUIETLY)
  else (PANGOMM_FOUND)
    if (PANGOMM_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find PANGOMM")
    endif (PANGOMM_FIND_REQUIRED)
  endif (PANGOMM_FOUND)

  # show the PANGOMM_INCLUDE_DIRS and PANGOMM_LIBRARIES variables only in the advanced view
  mark_as_advanced(PANGOMM_INCLUDE_DIRS PANGOMM_LIBRARIES)

endif (PANGOMM_LIBRARIES AND PANGOMM_INCLUDE_DIRS)

