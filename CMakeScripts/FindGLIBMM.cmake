# - Try to find glibmm
# Once done this will define
#
#  GLIBMM_FOUND - system has glibmm
#  GLIBMM_INCLUDE_DIRS - the glibmm include directory
#  GLIBMM_LIBRARIES - Link these to use glibmm
#  GLIBMM_DEFINITIONS - Compiler switches required for using glibmm
#
#  Copyright (c) 2008 Joshua L. Blocher <verbalshadow@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (GLIBMM_LIBRARIES AND GLIBMM_INCLUDE_DIRS)
  # in cache already
  set(GLIBMM_FOUND TRUE)
else (GLIBMM_LIBRARIES AND GLIBMM_INCLUDE_DIRS)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  if (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    include(UsePkgConfig)
    pkgconfig(glibmm-2.4 _GLIBMM_INCLUDEDIR _GLIBMM_LIBDIR _GLIBMM_LDFLAGS _GLIBMM_CFLAGS)
  else (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    find_package(PkgConfig)
    if (PKG_CONFIG_FOUND)
      pkg_check_modules(_GLIBMM glibmm-2.4)
    endif (PKG_CONFIG_FOUND)
  endif (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
  find_path(GLIBMM_INCLUDE_DIR
    NAMES
      glibmm.h
    PATHS
      ${_GLIBMM_INCLUDEDIR}
      /usr/include
      /usr/local/include
      /opt/local/include
      /opt/local/lib/glibmm-2.4/include
      /sw/include
    PATH_SUFFIXES
      glibmm-2.4
  )

  find_path(GLIBMM_CONFIG_INCLUDE_DIR
    NAMES
      glibmmconfig.h
    PATHS
      ${_GLIBMM_INCLUDEDIR}
      /usr/include
      /usr/local/include
      /opt/local/include
      /opt/local/lib/glibmm-2.4/include
      /sw/include
      /usr/lib/glibmm-2.4/include
      /usr/lib64/glibmm-2.4/include
  )
  find_library(GLIBMM-2.4_LIBRARY
    NAMES
      glibmm-2.4
    PATHS
      ${_GLIBMM_LIBDIR}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  if (GLIBMM-2.4_LIBRARY)
    set(GLIBMM-2.4_FOUND TRUE)
  endif (GLIBMM-2.4_LIBRARY)

  set(GLIBMM_INCLUDE_DIRS
    ${GLIBMM_INCLUDE_DIR}
    ${GLIBMM_CONFIG_INCLUDE_DIR}
  )

  if (GLIBMM-2.4_FOUND)
    set(GLIBMM_LIBRARIES
      ${GLIBMM_LIBRARIES}
      ${GLIBMM-2.4_LIBRARY}
    )
  endif (GLIBMM-2.4_FOUND)

  if (GLIBMM_INCLUDE_DIRS AND GLIBMM_LIBRARIES)
     set(GLIBMM_FOUND TRUE)
  endif (GLIBMM_INCLUDE_DIRS AND GLIBMM_LIBRARIES)

  if (GLIBMM_FOUND)
    if (NOT glibmm_FIND_QUIETLY)
      message(STATUS "Found glibmm: ${GLIBMM_LIBRARIES}")
    endif (NOT glibmm_FIND_QUIETLY)
  else (GLIBMM_FOUND)
    if (glibmm_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find glibmm")
    endif (glibmm_FIND_REQUIRED)
  endif (GLIBMM_FOUND)

  # show the GLIBMM_INCLUDE_DIRS and GLIBMM_LIBRARIES variables only in the advanced view
  mark_as_advanced(GLIBMM_INCLUDE_DIRS GLIBMM_LIBRARIES)

endif (GLIBMM_LIBRARIES AND GLIBMM_INCLUDE_DIRS)

