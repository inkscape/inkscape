# - Try to find BoehmGC
# Once done this will define
#
#  BOEHMGC_FOUND - system has BoehmGC
#  BOEHMGC_INCLUDE_DIRS - the BoehmGC include directory
#  BOEHMGC_LIBRARIES - Link these to use BoehmGC
#  BOEHMGC_DEFINITIONS - Compiler switches required for using BoehmGC
#
#  Copyright (c) 2008 Joshua L. Blocher <verbalshadow@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (BOEHMGC_LIBRARIES AND BOEHMGC_INCLUDE_DIRS)
  # in cache already
  set(BOEHMGC_FOUND TRUE)
else (BOEHMGC_LIBRARIES AND BOEHMGC_INCLUDE_DIRS)
  find_path(BOEHMGC_INCLUDE_DIR
    NAMES
      gc.h
    PATHS
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
      $ENV{DEVLIBS_PATH}/include
    PATH_SUFFIXES
      gc
  )

  find_library(GC_LIBRARY
    NAMES
      gc
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
      $ENV{DEVLIBS_PATH}/lib
  )

  if (GC_LIBRARY)
    set(GC_FOUND TRUE)
  endif (GC_LIBRARY)

  set(BOEHMGC_INCLUDE_DIRS
    ${BOEHMGC_INCLUDE_DIR}
  )

  if (GC_FOUND)
    set(BOEHMGC_LIBRARIES
      ${BOEHMGC_LIBRARIES}
      ${GC_LIBRARY}
    )
  endif (GC_FOUND)

  if (BOEHMGC_INCLUDE_DIRS AND BOEHMGC_LIBRARIES)
     set(BOEHMGC_FOUND TRUE)
  endif (BOEHMGC_INCLUDE_DIRS AND BOEHMGC_LIBRARIES)

  if (BOEHMGC_FOUND)
    if (NOT BoehmGC_FIND_QUIETLY)
      message(STATUS "Found BoehmGC: ${BOEHMGC_LIBRARIES}")
    endif (NOT BoehmGC_FIND_QUIETLY)
  else (BOEHMGC_FOUND)
    if (BoehmGC_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find BoehmGC")
    endif (BoehmGC_FIND_REQUIRED)
  endif (BOEHMGC_FOUND)

  # show the BOEHMGC_INCLUDE_DIRS and BOEHMGC_LIBRARIES variables only in the advanced view
  mark_as_advanced(BOEHMGC_INCLUDE_DIRS BOEHMGC_LIBRARIES)

endif (BOEHMGC_LIBRARIES AND BOEHMGC_INCLUDE_DIRS)

