# - Try to find LibVisio
# Once done this will define
#
#  LIBVISIO_FOUND - system has LibVisio
#  LIBVISIO_INCLUDE_DIRS - the LibVisio include directory
#  LIBVISIO_LIBRARIES - Link these to use LibVisio
#  LIBVISIO_DEFINITIONS - Compiler switches required for using LibVisio
#
#  Copyright (c) 2008 Joshua L. Blocher <verbalshadow@gmail.com>
#  Copyright (c) 2015 su_v <suv-sf@users.sf.net>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

include(${CMAKE_CURRENT_LIST_DIR}/../HelperMacros.cmake)

if (LIBVISIO_LIBRARIES AND LIBVISIO_INCLUDE_DIRS)
	# in cache already
	set(LIBVISIO_FOUND TRUE)
else (LIBVISIO_LIBRARIES AND LIBVISIO_INCLUDE_DIRS)
	# use pkg-config to get the directories and then use these values
	# in the FIND_PATH() and FIND_LIBRARY() calls
	find_package(PkgConfig)
	if (PKG_CONFIG_FOUND)
		INKSCAPE_PKG_CONFIG_FIND(LIBVISIO-0.1 libvisio-0.1 0 libvisio/libvisio.h libvisio-0.1 visio-0.1)
		if (LIBVISIO-0.1_FOUND)
			find_package(LibRevenge)
			if (LIBREVENGE_FOUND)
				list(APPEND LIBVISIO_INCLUDE_DIRS ${LIBVISIO-0.1_INCLUDE_DIRS})
				list(APPEND LIBVISIO_LIBRARIES    ${LIBVISIO-0.1_LIBRARIES})
				list(APPEND LIBVISIO_INCLUDE_DIRS ${LIBREVENGE_INCLUDE_DIRS})
				list(APPEND LIBVISIO_LIBRARIES    ${LIBREVENGE_LIBRARIES})
				set(LIBVISIO01_FOUND TRUE)
			endif (LIBREVENGE_FOUND)
		else()
			INKSCAPE_PKG_CONFIG_FIND(LIBVISIO-0.0 libvisio-0.0 0 libvisio/libvisio.h libvisio-0.0 visio-0.0)
			INKSCAPE_PKG_CONFIG_FIND(LIBWPD-0.9 libwpd-0.9 0 libwpd/libwpd.h libwpd-0.9 wpd-0.9)
			INKSCAPE_PKG_CONFIG_FIND(LIBWPD-STREAM-0.9 libwpd-stream-0.9 0 libwpd/libwpd.h libwpd-0.9 wpd-stream-0.9)
			if (LIBVISIO-0.0_FOUND AND LIBWPD-STREAM-0.9_FOUND AND LIBWPD-0.9_FOUND)
				list(APPEND LIBVISIO_INCLUDE_DIRS ${LIBVISIO-0.0_INCLUDE_DIRS})
				list(APPEND LIBVISIO_LIBRARIES    ${LIBVISIO-0.0_LIBRARIES})
				list(APPEND LIBVISIO_INCLUDE_DIRS ${LIBWPD-0.9_INCLUDE_DIRS})
				list(APPEND LIBVISIO_LIBRARIES    ${LIBWPD-0.9_LIBRARIES})
				list(APPEND LIBVISIO_INCLUDE_DIRS ${LIBWPD-STREAM-0.9_INCLUDE_DIRS})
				list(APPEND LIBVISIO_LIBRARIES    ${LIBWPD-STREAM-0.9_LIBRARIES})
				set(LIBVISIO00_FOUND TRUE)
			endif (LIBVISIO-0.0_FOUND AND LIBWPD-STREAM-0.9_FOUND AND LIBWPD-0.9_FOUND)
		endif (LIBVISIO-0.1_FOUND)
		if (LIBVISIO-0.1_FOUND OR LIBVISIO-0.0_FOUND)
			set(LIBVISIO_FOUND TRUE)
		endif (LIBVISIO-0.1_FOUND OR LIBVISIO-0.0_FOUND)
	endif (PKG_CONFIG_FOUND)
endif (LIBVISIO_LIBRARIES AND LIBVISIO_INCLUDE_DIRS)

