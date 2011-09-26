# - Try to find GtkSpell
# Once done this will define
#
#  GTKSPELL_FOUND - system has GtkSpell
#  GTKSPELL_INCLUDE_DIRS - the GtkSpell include directory
#  GTKSPELL_LIBRARIES - Link these to use GtkSpell
#  GTKSPELL_DEFINITIONS - Compiler switches required for using GtkSpell
#
#  Copyright (c) 2008 Joshua L. Blocher <verbalshadow@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

include(${CMAKE_CURRENT_LIST_DIR}/../HelperMacros.cmake)

if (GTKSPELL_LIBRARIES AND GTKSPELL_INCLUDE_DIRS)
	# in cache already
	set(GTKSPELL_FOUND TRUE)
else (GTKSPELL_LIBRARIES AND GTKSPELL_INCLUDE_DIRS)
	# use pkg-config to get the directories and then use these values
	# in the FIND_PATH() and FIND_LIBRARY() calls
	find_package(PkgConfig)
	if (PKG_CONFIG_FOUND)
		INKSCAPE_PKG_CONFIG_FIND(GTKSPELL gtkspell-2.0 0 gtkspell/gtkspell.h gtkspell-2.0 gtkspell)
	endif (PKG_CONFIG_FOUND)
endif (GTKSPELL_LIBRARIES AND GTKSPELL_INCLUDE_DIRS)
