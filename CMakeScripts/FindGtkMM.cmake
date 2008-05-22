# - Try to find GtkMM
# Once done this will define
#
#  GTKMM_FOUND - system has GtkMM
#  GTKMM_INCLUDE_DIRS - the GtkMM include directory
#  GTKMM_LIBRARIES - Link these to use GtkMM
#  GTKMM_DEFINITIONS - Compiler switches required for using GtkMM
#
#  Copyright (c) 2008 Joshua L. Blocher <verbalshadow@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (GTKMM_LIBRARIES AND GTKMM_INCLUDE_DIRS)
  # in cache already
  set(GTKMM_FOUND TRUE)
else (GTKMM_LIBRARIES AND GTKMM_INCLUDE_DIRS)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  if (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    include(UsePkgConfig)
    pkgconfig(gtkmm-2.4 _GTKMM_INCLUDEDIR _GTKMM_LIBDIR _GTKMM_LDFLAGS _GTKMM_CFLAGS)
  else (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    find_package(PkgConfig)
    if (PKG_CONFIG_FOUND)
      pkg_check_modules(_GTKMM gtkmm-2.4)
    endif (PKG_CONFIG_FOUND)
  endif (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
  find_path(GTKMM_INCLUDE_DIR
    NAMES
      gtkmm.h
    PATHS
      ${_GTKMM_INCLUDEDIR}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
    PATH_SUFFIXES
      gtkmm-2.4
  )

  find_library(GTKMM-2.4_LIBRARY
    NAMES
      gtkmm-2.4
    PATHS
      ${_GTKMM_LIBDIR}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  if (GTKMM-2.4_LIBRARY)
    set(GTKMM-2.4_FOUND TRUE)
  endif (GTKMM-2.4_LIBRARY)

  set(GTKMM_INCLUDE_DIRS
    ${GTKMM_INCLUDE_DIR}
  )

  if (GTKMM-2.4_FOUND)
    set(GTKMM_LIBRARIES
      ${GTKMM_LIBRARIES}
      ${GTKMM-2.4_LIBRARY}
    )
  endif (GTKMM-2.4_FOUND)

  if (GTKMM_INCLUDE_DIRS AND GTKMM_LIBRARIES)
     set(GTKMM_FOUND TRUE)
  endif (GTKMM_INCLUDE_DIRS AND GTKMM_LIBRARIES)

  if (GTKMM_FOUND)
    if (NOT GtkMM_FIND_QUIETLY)
      message(STATUS "Found GtkMM: ${GTKMM_LIBRARIES}")
    endif (NOT GtkMM_FIND_QUIETLY)
  else (GTKMM_FOUND)
    if (GtkMM_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find GtkMM")
    endif (GtkMM_FIND_REQUIRED)
  endif (GTKMM_FOUND)

  # show the GTKMM_INCLUDE_DIRS and GTKMM_LIBRARIES variables only in the advanced view
  mark_as_advanced(GTKMM_INCLUDE_DIRS GTKMM_LIBRARIES)

endif (GTKMM_LIBRARIES AND GTKMM_INCLUDE_DIRS)

