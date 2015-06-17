# - Try to find LibRevenge
# Once done this will define
#
#  LIBREVENGE_FOUND - system has LibRevenge
#  LIBREVENGE_INCLUDE_DIRS - the LibRevenge include directory
#  LIBREVENGE_LIBRARIES - Link these to use LibRevenge
#  LIBREVENGE_DEFINITIONS - Compiler switches required for using LibRevenge
#
#  Copyright (c) 2008 Joshua L. Blocher <verbalshadow@gmail.com>
#  Copyright (c) 2015 su_v <suv-sf@users.sf.net>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

include(${CMAKE_CURRENT_LIST_DIR}/../HelperMacros.cmake)

if (LIBREVENGE_LIBRARIES AND LIBREVENGE_INCLUDE_DIRS)
	# in cache already
	set(LIBREVENGE_FOUND TRUE)
else (LIBREVENGE_LIBRARIES AND LIBREVENGE_INCLUDE_DIRS)
	# use pkg-config to get the directories and then use these values
	# in the FIND_PATH() and FIND_LIBRARY() calls
	find_package(PkgConfig)
	if (PKG_CONFIG_FOUND)
		INKSCAPE_PKG_CONFIG_FIND(LIBREVENGE-0.0 librevenge-0.0 0 librevenge/librevenge.h librevenge-0.0 revenge-0.0)
		INKSCAPE_PKG_CONFIG_FIND(LIBREVENGE-STREAM-0.0 librevenge-stream-0.0 0 librevenge-0.0/librevenge-stream/librevenge-stream.h librevenge-stream-0.0 revenge-stream-0.0)
		if (LIBREVENGE-0.0_FOUND AND LIBREVENGE-STREAM-0.0_FOUND)
			list(APPEND LIBREVENGE_INCLUDE_DIRS ${LIBREVENGE-0.0_INCLUDE_DIRS})
			list(APPEND LIBREVENGE_LIBRARIES    ${LIBREVENGE-0.0_LIBRARIES})
			list(APPEND LIBREVENGE_INCLUDE_DIRS ${LIBREVENGE-STREAM-0.0_INCLUDE_DIRS})
			list(APPEND LIBREVENGE_LIBRARIES    ${LIBREVENGE-STREAM-0.0_LIBRARIES})
			set(LIBREVENGE00_FOUND TRUE)
		endif (LIBREVENGE-0.0_FOUND AND LIBREVENGE-STREAM-0.0_FOUND)
		if (LIBREVENGE-0.0_FOUND)
			set(LIBREVENGE_FOUND TRUE)
		endif (LIBREVENGE-0.0_FOUND)
	endif (PKG_CONFIG_FOUND)
endif (LIBREVENGE_LIBRARIES AND LIBREVENGE_INCLUDE_DIRS)

