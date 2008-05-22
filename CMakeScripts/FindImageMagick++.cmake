# - Try to find ImageMagick++
# Once done this will define
#
#  IMAGEMAGICK++_FOUND - system has ImageMagick++
#  IMAGEMAGICK++_INCLUDE_DIRS - the ImageMagick++ include directory
#  IMAGEMAGICK++_LIBRARIES - Link these to use ImageMagick++
#  IMAGEMAGICK++_DEFINITIONS - Compiler switches required for using ImageMagick++
#
#  Copyright (c) 2008 Joshua L. Blocher <verbalshadow@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (IMAGEMAGICK++_LIBRARIES AND IMAGEMAGICK++_INCLUDE_DIRS)
  # in cache already
  set(IMAGEMAGICK++_FOUND TRUE)
else (IMAGEMAGICK++_LIBRARIES AND IMAGEMAGICK++_INCLUDE_DIRS)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  if (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    include(UsePkgConfig)
    pkgconfig(ImageMagick++ _IMAGEMAGICK++_INCLUDEDIR _IMAGEMAGICK++_LIBDIR _IMAGEMAGICK++_LDFLAGS _IMAGEMAGICK++_CFLAGS)
  else (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    find_package(PkgConfig)
    if (PKG_CONFIG_FOUND)
      pkg_check_modules(_IMAGEMAGICK++ ImageMagick++)
    endif (PKG_CONFIG_FOUND)
  endif (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
  find_path(IMAGEMAGICK++_INCLUDE_DIR
    NAMES
      Image.h
    PATHS
      ${_IMAGEMAGICK++_INCLUDEDIR}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
    PATH_SUFFIXES
      Magick++
  )

  find_library(MAGICK++_LIBRARY
    NAMES
      Magick++
    PATHS
      ${_IMAGEMAGICK++_LIBDIR}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  if (MAGICK++_LIBRARY)
    set(MAGICK++_FOUND TRUE)
  endif (MAGICK++_LIBRARY)

  set(IMAGEMAGICK++_INCLUDE_DIRS
    ${IMAGEMAGICK++_INCLUDE_DIR}
  )

  if (MAGICK++_FOUND)
    set(IMAGEMAGICK++_LIBRARIES
      ${IMAGEMAGICK++_LIBRARIES}
      ${MAGICK++_LIBRARY}
    )
  endif (MAGICK++_FOUND)

  if (IMAGEMAGICK++_INCLUDE_DIRS AND IMAGEMAGICK++_LIBRARIES)
     set(IMAGEMAGICK++_FOUND TRUE)
  endif (IMAGEMAGICK++_INCLUDE_DIRS AND IMAGEMAGICK++_LIBRARIES)

  if (IMAGEMAGICK++_FOUND)
    if (NOT ImageMagick++_FIND_QUIETLY)
      message(STATUS "Found ImageMagick++: ${IMAGEMAGICK++_LIBRARIES}")
    endif (NOT ImageMagick++_FIND_QUIETLY)
  else (IMAGEMAGICK++_FOUND)
    if (ImageMagick++_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find ImageMagick++")
    endif (ImageMagick++_FIND_REQUIRED)
  endif (IMAGEMAGICK++_FOUND)

  # show the IMAGEMAGICK++_INCLUDE_DIRS and IMAGEMAGICK++_LIBRARIES variables only in the advanced view
  mark_as_advanced(IMAGEMAGICK++_INCLUDE_DIRS IMAGEMAGICK++_LIBRARIES)

endif (IMAGEMAGICK++_LIBRARIES AND IMAGEMAGICK++_INCLUDE_DIRS)

