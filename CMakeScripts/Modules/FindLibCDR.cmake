# - Try to find LibCDR
# Once done this will define
#
#  LIBCDR_FOUND - system has LibCDR
#  LIBCDR_INCLUDE_DIRS - the LibCDR include directory
#  LIBCDR_LIBRARIES - Link these to use LibCDR
#  LIBCDR_DEFINITIONS - Compiler switches required for using LibCDR
#
#  Copyright (c) 2008 Joshua L. Blocher <verbalshadow@gmail.com>
#  Copyright (c) 2015 su_v <suv-sf@users.sf.net>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

include(${CMAKE_CURRENT_LIST_DIR}/../HelperMacros.cmake)

if (LIBCDR_LIBRARIES AND LIBCDR_INCLUDE_DIRS)
	# in cache already
	set(LIBCDR_FOUND TRUE)
else (LIBCDR_LIBRARIES AND LIBCDR_INCLUDE_DIRS)
	# use pkg-config to get the directories and then use these values
	# in the FIND_PATH() and FIND_LIBRARY() calls
	find_package(PkgConfig)
	if (PKG_CONFIG_FOUND)
		INKSCAPE_PKG_CONFIG_FIND(LIBCDR-0.1 libcdr-0.1 0 libcdr/libcdr.h libcdr-0.1 cdr-0.1)
		if (LIBCDR-0.1_FOUND)
			find_package(LibRevenge)
			if (LIBREVENGE_FOUND)
				list(APPEND LIBCDR_INCLUDE_DIRS ${LIBCDR-0.1_INCLUDE_DIRS})
				list(APPEND LIBCDR_LIBRARIES    ${LIBCDR-0.1_LIBRARIES})
				list(APPEND LIBCDR_INCLUDE_DIRS ${LIBREVENGE_INCLUDE_DIRS})
				list(APPEND LIBCDR_LIBRARIES    ${LIBREVENGE_LIBRARIES})
				set(LIBCDR01_FOUND TRUE)
			endif (LIBREVENGE_FOUND)
		else()
			INKSCAPE_PKG_CONFIG_FIND(LIBCDR-0.0 libcdr-0.0 0 libcdr/libcdr.h libcdr-0.0 cdr-0.0)
			INKSCAPE_PKG_CONFIG_FIND(LIBWPD-0.9 libwpd-0.9 0 libwpd/libwpd.h libwpd-0.9 wpd-0.9)
			INKSCAPE_PKG_CONFIG_FIND(LIBWPD-STREAM-0.9 libwpd-stream-0.9 0 libwpd/libwpd.h libwpd-0.9 wpd-stream-0.9)
			if (LIBCDR-0.0_FOUND AND LIBWPD-STREAM-0.9_FOUND AND LIBWPD-0.9_FOUND)
				list(APPEND LIBCDR_INCLUDE_DIRS ${LIBCDR-0.0_INCLUDE_DIRS})
				list(APPEND LIBCDR_LIBRARIES    ${LIBCDR-0.0_LIBRARIES})
				list(APPEND LIBCDR_INCLUDE_DIRS ${LIBWPD-0.9_INCLUDE_DIRS})
				list(APPEND LIBCDR_LIBRARIES    ${LIBWPD-0.9_LIBRARIES})
				list(APPEND LIBCDR_INCLUDE_DIRS ${LIBWPD-STREAM-0.9_INCLUDE_DIRS})
				list(APPEND LIBCDR_LIBRARIES    ${LIBWPD-STREAM-0.9_LIBRARIES})
				set(LIBCDR00_FOUND TRUE)
			endif (LIBCDR-0.0_FOUND AND LIBWPD-STREAM-0.9_FOUND AND LIBWPD-0.9_FOUND)
		endif (LIBCDR-0.1_FOUND)
		if (LIBCDR-0.1_FOUND OR LIBCDR-0.0_FOUND)
			set(LIBCDR_FOUND TRUE)
		endif (LIBCDR-0.1_FOUND OR LIBCDR-0.0_FOUND)
	endif (PKG_CONFIG_FOUND)
endif (LIBCDR_LIBRARIES AND LIBCDR_INCLUDE_DIRS)

