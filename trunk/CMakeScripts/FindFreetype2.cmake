# - Try to find FREETYPE2
# Once done this will define
#
#  FREETYPE2_FOUND - system has FREETYPE2
#  FREETYPE2_INCLUDE_DIRS - the FREETYPE2 include directory
#  FREETYPE2_LIBRARIES - Link these to use FREETYPE2
#  FREETYPE2_DEFINITIONS - Compiler switches required for using FREETYPE2
#
#  Copyright (c) 2008 Joshua L. Blocher <verbalshadow@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (FREETYPE2_LIBRARIES AND FREETYPE2_INCLUDE_DIRS)
  # in cache already
  set(FREETYPE2_FOUND TRUE)
else (FREETYPE2_LIBRARIES AND FREETYPE2_INCLUDE_DIRS)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  if (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    include(UsePkgConfig)
    pkgconfig(freetype2 _FREETYPE2_INCLUDEDIR _FREETYPE2_LIBDIR _FREETYPE2_LDFLAGS _FREETYPE2_CFLAGS)
  else (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    find_package(PkgConfig)
    if (PKG_CONFIG_FOUND)
      pkg_check_modules(_FREETYPE2 freetype2)
    endif (PKG_CONFIG_FOUND)
  endif (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
  find_path(FREETYPE2_INCLUDE_DIR
    NAMES
      freetype/freetype.h
    PATHS
      ${_FREETYPE2_INCLUDEDIR}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
      $ENV{DEVLIBS_PATH}//include//
    PATH_SUFFIXES
      freetype2
  )

  find_library(FREETYPE_LIBRARY
    NAMES
      freetype
    PATHS
      ${_FREETYPE2_LIBDIR}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  if (FREETYPE_LIBRARY)
    set(FREETYPE_FOUND TRUE)
  endif (FREETYPE_LIBRARY)

  set(FREETYPE2_INCLUDE_DIRS
    ${FREETYPE2_INCLUDE_DIR}
  )

  if (FREETYPE_FOUND)
    set(FREETYPE2_LIBRARIES
      ${FREETYPE2_LIBRARIES}
      ${FREETYPE_LIBRARY}
    )
  endif (FREETYPE_FOUND)

  if (FREETYPE2_INCLUDE_DIRS AND FREETYPE2_LIBRARIES)
     set(FREETYPE2_FOUND TRUE)
  endif (FREETYPE2_INCLUDE_DIRS AND FREETYPE2_LIBRARIES)

  if (FREETYPE2_FOUND)
    if (NOT FREETYPE2_FIND_QUIETLY)
      message(STATUS "Found FREETYPE2: ${FREETYPE2_LIBRARIES}")
    endif (NOT FREETYPE2_FIND_QUIETLY)
  else (FREETYPE2_FOUND)
    if (FREETYPE2_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find FREETYPE2")
    endif (FREETYPE2_FIND_REQUIRED)
  endif (FREETYPE2_FOUND)

  # show the FREETYPE2_INCLUDE_DIRS and FREETYPE2_LIBRARIES variables only in the advanced view
  mark_as_advanced(FREETYPE2_INCLUDE_DIRS FREETYPE2_LIBRARIES)

endif (FREETYPE2_LIBRARIES AND FREETYPE2_INCLUDE_DIRS)

