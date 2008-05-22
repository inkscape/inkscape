# - Try to find LibWPG
# Once done this will define
#
#  LIBWPG_FOUND - system has LibWPG
#  LIBWPG_INCLUDE_DIRS - the LibWPG include directory
#  LIBWPG_LIBRARIES - Link these to use LibWPG
#  LIBWPG_DEFINITIONS - Compiler switches required for using LibWPG
#
#  Copyright (c) 2008 Joshua L. Blocher <verbalshadow@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (LIBWPG_LIBRARIES AND LIBWPG_INCLUDE_DIRS)
  # in cache already
  set(LIBWPG_FOUND TRUE)
else (LIBWPG_LIBRARIES AND LIBWPG_INCLUDE_DIRS)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  if (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    include(UsePkgConfig)
    pkgconfig(libwpg-0.1 _LIBWPG_INCLUDEDIR _LIBWPG_LIBDIR _LIBWPG_LDFLAGS _LIBWPG_CFLAGS)
  else (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    find_package(PkgConfig)
    if (PKG_CONFIG_FOUND)
      pkg_check_modules(_LIBWPG libwpg-0.1)
    endif (PKG_CONFIG_FOUND)
  endif (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
  find_path(LIBWPG_INCLUDE_DIR
    NAMES
      libwpg/libwpg.h
    PATHS
      ${_LIBWPG_INCLUDEDIR}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
    PATH_SUFFIXES
      libwpg-0.1
  )

  find_library(LIBWPG-0.1_LIBRARY
    NAMES
      wpg-0.1
    PATHS
      ${_LIBWPG_LIBDIR}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  if (LIBWPG-0.1_LIBRARY)
    set(LIBWPG-0.1_FOUND TRUE)
  endif (LIBWPG-0.1_LIBRARY)

  set(LIBWPG_INCLUDE_DIRS
    ${LIBWPG_INCLUDE_DIR}
  )

  if (LIBWPG-0.1_FOUND)
    set(LIBWPG_LIBRARIES
      ${LIBWPG_LIBRARIES}
      ${LIBWPG-0.1_LIBRARY}
    )
  endif (LIBWPG-0.1_FOUND)

  if (LIBWPG_INCLUDE_DIRS AND LIBWPG_LIBRARIES)
     set(LIBWPG_FOUND TRUE)
  endif (LIBWPG_INCLUDE_DIRS AND LIBWPG_LIBRARIES)

  if (LIBWPG_FOUND)
    if (NOT LibWPG_FIND_QUIETLY)
      message(STATUS "Found LibWPG: ${LIBWPG_LIBRARIES}")
    endif (NOT LibWPG_FIND_QUIETLY)
  else (LIBWPG_FOUND)
    if (LibWPG_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find LibWPG")
    endif (LibWPG_FIND_REQUIRED)
  endif (LIBWPG_FOUND)

  # show the LIBWPG_INCLUDE_DIRS and LIBWPG_LIBRARIES variables only in the advanced view
  mark_as_advanced(LIBWPG_INCLUDE_DIRS LIBWPG_LIBRARIES)

endif (LIBWPG_LIBRARIES AND LIBWPG_INCLUDE_DIRS)

