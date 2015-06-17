# - Try to find LibWPG
# Once done this will define
#
#  LIBWPG_FOUND - system has LibWPG
#  LIBWPG_INCLUDE_DIRS - the LibWPG include directory
#  LIBWPG_LIBRARIES - Link these to use LibWPG
#  LIBWPG_DEFINITIONS - Compiler switches required for using LibWPG
#
#  Copyright (c) 2008 Joshua L. Blocher <verbalshadow@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

include(${CMAKE_CURRENT_LIST_DIR}/../HelperMacros.cmake)

if (LIBWPG_LIBRARIES AND LIBWPG_INCLUDE_DIRS)
	# in cache already
	set(LIBWPG_FOUND TRUE)
else (LIBWPG_LIBRARIES AND LIBWPG_INCLUDE_DIRS)
	# use pkg-config to get the directories and then use these values
	# in the FIND_PATH() and FIND_LIBRARY() calls
	find_package(PkgConfig)
	if (PKG_CONFIG_FOUND)
		INKSCAPE_PKG_CONFIG_FIND(LIBWPG-0.3 libwpg-0.3 0 libwpg/libwpg.h libwpg-0.3 wpg-0.3)
		if (LIBWPG-0.3_FOUND)
			find_package(LibRevenge)
			if (LIBREVENGE_FOUND)
				list(APPEND LIBWPG_INCLUDE_DIRS ${LIBWPG-0.3_INCLUDE_DIRS})
				list(APPEND LIBWPG_LIBRARIES    ${LIBWPG-0.3_LIBRARIES})
				list(APPEND LIBWPG_INCLUDE_DIRS ${LIBREVENGE_INCLUDE_DIRS})
				list(APPEND LIBWPG_LIBRARIES    ${LIBREVENGE_LIBRARIES})
				set(LIBWPG03_FOUND TRUE)
			endif (LIBREVENGE_FOUND)
		else()
			INKSCAPE_PKG_CONFIG_FIND(LIBWPG-0.2 libwpg-0.2 0 libwpg/libwpg.h libwpg-0.2 wpg-0.2)
			INKSCAPE_PKG_CONFIG_FIND(LIBWPD-0.9 libwpd-0.9 0 libwpd/libwpd.h libwpd-0.9 wpd-0.9)
			INKSCAPE_PKG_CONFIG_FIND(LIBWPD-STREAM-0.9 libwpd-stream-0.9 0 libwpd/libwpd.h libwpd-0.9 wpd-stream-0.9)
			if (LIBWPG-0.2_FOUND AND LIBWPD-STREAM-0.9_FOUND AND LIBWPD-0.9_FOUND)
				list(APPEND LIBWPG_INCLUDE_DIRS ${LIBWPG-0.2_INCLUDE_DIRS})
				list(APPEND LIBWPG_LIBRARIES    ${LIBWPG-0.2_LIBRARIES})
				list(APPEND LIBWPG_INCLUDE_DIRS ${LIBWPD-0.9_INCLUDE_DIRS})
				list(APPEND LIBWPG_LIBRARIES    ${LIBWPD-0.9_LIBRARIES})
				list(APPEND LIBWPG_INCLUDE_DIRS ${LIBWPD-STREAM-0.9_INCLUDE_DIRS})
				list(APPEND LIBWPG_LIBRARIES    ${LIBWPD-STREAM-0.9_LIBRARIES})
				set(LIBWPG02_FOUND TRUE)
			else()
				INKSCAPE_PKG_CONFIG_FIND(LIBWPG-0.1 libwpg-0.1 0 libwpg/libwpg.h libwpg-0.1 wpg-0.1)
				INKSCAPE_PKG_CONFIG_FIND(LIBWPG-STREAM-0.1 libwpg-stream-0.1 0 libwpg/libwpg.h libwpg-0.1 wpg-stream-0.1)
				INKSCAPE_PKG_CONFIG_FIND(LIBWPD-0.8 libwpd-0.8 0 libwpd/libwpd.h libwpd-0.8 wpd-0.8)
				if (LIBWPG-0.1_FOUND AND LIBWPG-STREAM-0.1_FOUND AND LIBWPD-0.8_FOUND)
					list(APPEND LIBWPG_INCLUDE_DIRS ${LIBWPG-0.1_INCLUDE_DIRS})
					list(APPEND LIBWPG_LIBRARIES    ${LIBWPG-0.1_LIBRARIES})
					list(APPEND LIBWPG_INCLUDE_DIRS ${LIBWPG-STREAM-0.1_INCLUDE_DIRS})
					list(APPEND LIBWPG_LIBRARIES    ${LIBWPG-STREAM-0.1_LIBRARIES})
					list(APPEND LIBWPG_INCLUDE_DIRS ${LIBWPD-0.8_INCLUDE_DIRS})
					list(APPEND LIBWPG_LIBRARIES    ${LIBWPD-0.8_LIBRARIES})
					set(LIBWPG01_FOUND TRUE)
				endif (LIBWPG-0.1_FOUND AND LIBWPG-STREAM-0.1_FOUND AND LIBWPD-0.8_FOUND)
			endif (LIBWPG-0.2_FOUND AND LIBWPD-STREAM-0.9_FOUND AND LIBWPD-0.9_FOUND)
		endif (LIBWPG-0.3_FOUND)
		if (LIBWPG-0.1_FOUND OR LIBWPG-0.2_FOUND OR LIBWPG-0.3_FOUND)
			set(LIBWPG_FOUND TRUE)
		endif (LIBWPG-0.1_FOUND OR LIBWPG-0.2_FOUND OR LIBWPG-0.3_FOUND)
	endif (PKG_CONFIG_FOUND)
endif (LIBWPG_LIBRARIES AND LIBWPG_INCLUDE_DIRS)
