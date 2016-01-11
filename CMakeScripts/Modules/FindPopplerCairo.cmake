# - Try to find PopplerCairo
# Once done this will define
#
#  POPPLER_FOUND - system has PopplerCairo
#  POPPLER_INCLUDE_DIRS - the PopplerCairo include directory
#  POPPLER_LIBRARIES - Link these to use PopplerCairo
#  POPPLER_DEFINITIONS - Compiler switches required for using PopplerCairo
#
#  Copyright (c) 2008 Joshua L. Blocher <verbalshadow@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

include(${CMAKE_CURRENT_LIST_DIR}/../HelperMacros.cmake)

if (POPPLER_LIBRARIES AND POPPLER_INCLUDE_DIRS)
	# in cache already
	set(POPPLER_FOUND TRUE)
else (POPPLER_LIBRARIES AND POPPLER_INCLUDE_DIRS)
	# use pkg-config to get the directories and then use these values
	# in the FIND_PATH() and FIND_LIBRARY() calls
	find_package(PkgConfig)
	if (PKG_CONFIG_FOUND)
		INKSCAPE_PKG_CONFIG_FIND(POPPLER poppler >=0.20.0 poppler-config.h poppler poppler)
		if (POPPLER_FOUND)
			INKSCAPE_PKG_CONFIG_FIND(POPPLER_GLIB poppler-glib >=0.20.0 poppler/glib/poppler.h "" poppler-glib)
			if (POPPLER_GLIB_FOUND)
				list(APPEND POPPLER_INCLUDE_DIRS ${POPPLER_GLIB_INCLUDE_DIRS})
				list(APPEND POPPLER_LIBRARIES    ${POPPLER_GLIB_LIBRARIES})
				INKSCAPE_PKG_CONFIG_FIND(CAIRO_SVG cairo-svg 0 cairo-svg.h cairo cairo)
				if (CAIRO_SVG_FOUND)
					list(APPEND POPPLER_INCLUDE_DIRS ${CAIRO_SVG_INCLUDE_DIRS})
					list(APPEND POPPLER_LIBRARIES    ${CAIRO_SVG_LIBRARIES})
				endif (CAIRO_SVG_FOUND)
			endif (POPPLER_GLIB_FOUND)
			if (ENABLE_POPPLER_CAIRO)
				INKSCAPE_PKG_CONFIG_FIND(POPPLER_CAIRO poppler-cairo >=0.20.0 cairo.h cairo cairo)
				if (POPPLER_GLIB_FOUND AND POPPLER_CAIRO_FOUND AND NOT CAIRO_SVG_FOUND)
					list(APPEND POPPLER_INCLUDE_DIRS ${POPPLER_CAIRO_INCLUDE_DIRS})
					list(APPEND POPPLER_LIBRARIES    ${POPPLER_CAIRO_LIBRARIES})
				endif (POPPLER_GLIB_FOUND AND POPPLER_CAIRO_FOUND AND NOT CAIRO_SVG_FOUND)
			endif (ENABLE_POPPLER_CAIRO)
		endif (POPPLER_FOUND)
	endif (PKG_CONFIG_FOUND)
endif (POPPLER_LIBRARIES AND POPPLER_INCLUDE_DIRS)
