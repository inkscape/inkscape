# - Try to find GDKMM
# Once done this will define
#
#  GDKMM_FOUND - system has GDKMM
#  GDKMM_INCLUDE_DIRS - the GDKMM include directory
#  GDKMM_LIBRARIES - Link these to use GDKMM
#  GDKMM_DEFINITIONS - Compiler switches required for using GDKMM
#
#  Copyright (c) 2008 Joshua L. Blocher <verbalshadow@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (GDKMM_LIBRARIES AND GDKMM_INCLUDE_DIRS)
  # in cache already
  set(GDKMM_FOUND TRUE)
else (GDKMM_LIBRARIES AND GDKMM_INCLUDE_DIRS)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  if (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    include(UsePkgConfig)
    pkgconfig(gdkmm-2.4 _GDKMM_INCLUDEDIR _GDKMM_LIBDIR _GDKMM_LDFLAGS _GDKMM_CFLAGS)
  else (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    find_package(PkgConfig)
    if (PKG_CONFIG_FOUND)
      pkg_check_modules(_GDKMM gdkmm-2.4)
    endif (PKG_CONFIG_FOUND)
  endif (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
  find_path(GDKMM_INCLUDE_DIR
    NAMES
      gdkmm.h
    PATHS
      ${_GDKMM_INCLUDEDIR}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
    PATH_SUFFIXES
      gdkmm-2.4
  )

  find_path(GDKMM_CONFIG_INCLUDE_DIR
    NAMES
      gdkmmconfig.h
    PATHS
      ${_GDKMM_INCLUDEDIR}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
      /usr/lib/gdkmm-2.4/include
      /usr/lib64/gdkmm-2.4/include
  )
  
  find_library(GDKMM-2.4_LIBRARY
    NAMES
      gdkmm-2.4
    PATHS
      ${_GDKMM_LIBDIR}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  if (GDKMM-2.4_LIBRARY)
    set(GDKMM-2.4_FOUND TRUE)
  endif (GDKMM-2.4_LIBRARY)

  set(GDKMM_INCLUDE_DIRS
    ${GDKMM_INCLUDE_DIR}
    ${GDKMM_CONFIG_INCLUDE_DIR}
  )

  if (GDKMM-2.4_FOUND)
    set(GDKMM_LIBRARIES
      ${GDKMM_LIBRARIES}
      ${GDKMM-2.4_LIBRARY}
    )
  endif (GDKMM-2.4_FOUND)

  if (GDKMM_INCLUDE_DIRS AND GDKMM_LIBRARIES)
     set(GDKMM_FOUND TRUE)
  endif (GDKMM_INCLUDE_DIRS AND GDKMM_LIBRARIES)

  if (GDKMM_FOUND)
    if (NOT GDKMM_FIND_QUIETLY)
      message(STATUS "Found GDKMM: ${GDKMM_LIBRARIES}")
    endif (NOT GDKMM_FIND_QUIETLY)
  else (GDKMM_FOUND)
    if (GDKMM_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find GDKMM")
    endif (GDKMM_FIND_REQUIRED)
  endif (GDKMM_FOUND)

  # show the GDKMM_INCLUDE_DIRS and GDKMM_LIBRARIES variables only in the advanced view
  mark_as_advanced(GDKMM_INCLUDE_DIRS GDKMM_LIBRARIES)

endif (GDKMM_LIBRARIES AND GDKMM_INCLUDE_DIRS)

