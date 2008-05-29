# - Try to find GtkMM (glibmm-2.4 gdkmm-2.4 pangomm-1.4 atkmm-1.6)
#  Where not going to find gtk+-2.0 as this is covered using FindGTK2
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
SET(SubLibs
gtkmm-2.4
glibmm-2.4
gdkmm-2.4
pangomm-1.4
atkmm-1.6
)

if (GTKMM_LIBRARIES AND GTKMM_INCLUDE_DIRS)
  # in cache already
  set(GTKMM_FOUND TRUE)
else (GTKMM_LIBRARIES AND GTKMM_INCLUDE_DIRS)
FOREACH(_SUBLIB ${SubLibs})
  # Clean library name for header file
  STRING(REGEX REPLACE "[-]([^ ]+)" "" _H_${_SUBLIB}  "${_SUBLIB}" )
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  if (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    include(UsePkgConfig)
    pkgconfig(${_SUBLIB} _${_SUBLIB}_INCLUDEDIR _${_SUBLIB}_LIBDIR _${_SUBLIB}_LDFLAGS _${_SUBLIB}_CFLAGS)
  else (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    find_package(PkgConfig)
    if (PKG_CONFIG_FOUND)
      pkg_check_modules(_${_SUBLIB} ${_SUBLIB})
    endif (PKG_CONFIG_FOUND)
  endif (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
  find_path(${_SUBLIB}_INCLUDE_DIR
    NAMES
      ${_H_${_SUBLIB}}.h
    PATHS
      ${_${_SUBLIB}_INCLUDEDIR}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
    PATH_SUFFIXES
      ${_SUBLIB}
  )

  find_library(${_SUBLIB}_LIBRARY
    NAMES
      ${_SUBLIB}
    PATHS
      ${_${_SUBLIB}_LIBDIR}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  if (${_SUBLIB}_LIBRARY)
    set(${_SUBLIB}_FOUND TRUE)
  endif (${_SUBLIB}_LIBRARY)

  LIST(APPEND GTKMM_INCLUDE_DIRS
    ${${_SUBLIB}_INCLUDE_DIR}
  )

  if (${_SUBLIB}_FOUND)
    LIST(APPEND GTKMM_LIBRARIES
      ${${_SUBLIB}_LIBRARIES}
      ${${_SUBLIB}_LIBRARY}
    )
  endif (${_SUBLIB}_FOUND)
ENDFOREACH(_SUBLIB)

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

