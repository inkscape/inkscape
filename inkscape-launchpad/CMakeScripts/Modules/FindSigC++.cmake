# - Try to find SIGC++
# Once done this will define
#
#  SIGC++_FOUND - system has SIGC++
#  SIGC++_INCLUDE_DIRS - the SIGC++ include directory
#  SIGC++_LIBRARIES - Link these to use SIGC++
#  SIGC++_DEFINITIONS - Compiler switches required for using SIGC++
#
#  Copyright (c) 2008 Andreas Schneider <mail@cynapses.org>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (SIGC++_LIBRARIES AND SIGC++_INCLUDE_DIRS)
  # in cache already
  set(SIGC++_FOUND TRUE)
else (SIGC++_LIBRARIES AND SIGC++_INCLUDE_DIRS)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  if (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    include(UsePkgConfig)
    pkgconfig(sigc++-2.0 _SIGC++_INCLUDEDIR _SIGC++_LIBDIR _SIGC++_LDFLAGS _SIGC++_CFLAGS)
  else (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    find_package(PkgConfig)
    if (PKG_CONFIG_FOUND)
      pkg_check_modules(_SIGC++ sigc++-2.0)
    endif (PKG_CONFIG_FOUND)
  endif (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)

  find_path(SIGC++_INCLUDE_DIR
    NAMES
      sigc++/sigc++.h
    PATHS
      ${_SIGC++_INCLUDEDIR}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
    PATH_SUFFIXES
      sigc++-2.0
  )

  find_path(SIGC++_CONFIG_INCLUDE_DIR
    NAMES
      sigc++config.h
    PATHS
      ${_SIGC++_INCLUDEDIR}
      /usr/include
      /usr/local/include
      /opt/local/include
      /opt/local/lib/sigc++-2.0/include
      /sw/include
      /usr/lib/sigc++-2.0/include
      /usr/lib64/sigc++-2.0/include
  )

  find_library(SIGC-2.0_LIBRARY
    NAMES
      sigc-2.0
    PATHS
      ${_SIGC++_LIBDIR}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  if (SIGC-2.0_LIBRARY)
    set(SIGC-2.0_FOUND TRUE)
  endif (SIGC-2.0_LIBRARY)

  set(SIGC++_INCLUDE_DIRS
    ${SIGC++_INCLUDE_DIR}
    ${SIGC++_CONFIG_INCLUDE_DIR}
  )

  if (SIGC-2.0_FOUND)
    set(SIGC++_LIBRARIES
      ${SIGC++_LIBRARIES}
      ${SIGC-2.0_LIBRARY}
    )
  endif (SIGC-2.0_FOUND)

  if (SIGC++_INCLUDE_DIRS AND SIGC++_LIBRARIES)
     set(SIGC++_FOUND TRUE)
  endif (SIGC++_INCLUDE_DIRS AND SIGC++_LIBRARIES)

  if (SIGC++_FOUND)
    if (NOT SIGC++_FIND_QUIETLY)
      message(STATUS "Found SIGC++: ${SIGC++_LIBRARIES}")
    endif (NOT SIGC++_FIND_QUIETLY)
  else (SIGC++_FOUND)
    if (SIGC++_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find SIGC++")
    endif (SIGC++_FIND_REQUIRED)
  endif (SIGC++_FOUND)

  # show the SIGC++_INCLUDE_DIRS and SIGC++_LIBRARIES variables only in the advanced view
  mark_as_advanced(SIGC++_INCLUDE_DIRS SIGC++_LIBRARIES)

endif (SIGC++_LIBRARIES AND SIGC++_INCLUDE_DIRS)


