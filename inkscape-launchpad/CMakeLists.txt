# CMake TODO
# - Remove hard coded defines (see below)
# - Test on MinGW
# √ Test on OSX
# - Add configurable options for Python/Perl, see configure --help
#
# ideasman42

cmake_minimum_required(VERSION 2.8.0)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeScripts/Modules")

# avoid having empty buildtype
set(CMAKE_BUILD_TYPE_INIT "Release")

project(inkscape)

set(INKSCAPE_VERSION 0.91+devel)
set(PROJECT_NAME inkscape)
set(CMAKE_INCLUDE_CURRENT_DIR TRUE)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
	cmake_policy(SET CMP0005 NEW) # propper define quoting
endif()

# workaround for omission in cmake 2.8.4's GNU.cmake, fixed in 2.8.5
if(CMAKE_COMPILER_IS_GNUCC)
	if(NOT DARWIN)
		set(CMAKE_INCLUDE_SYSTEM_FLAG_C "-isystem ")
	endif()
endif()

#
# Set platform defaults (originally copied from darktable)
#
if(APPLE)
	message("-- Mac OS X build detected, setting default features")
	# prefer macports and/or user-installed libraries over system ones
	#LIST(APPEND CMAKE_PREFIX_PATH /opt/local /usr/local)
	set(CMAKE_FIND_FRAMEWORK "LAST")

	# test and display relevant env variables
	if(DEFINED ENV{CMAKE_PREFIX_PATH})
		message("CMAKE_PREFIX_PATH: $ENV{CMAKE_PREFIX_PATH}")
	endif()
	if(DEFINED ENV{GTKMM_BASEPATH})
		message("GTKMM_BASEPATH: $ENV{GTKMM_BASEPATH}")
	endif()

	# detect current GTK+ backend
	include(${CMAKE_SOURCE_DIR}/CMakeScripts/HelperFunctions.cmake)
	pkg_check_variable(gtk+-2.0 target)
	message("GTK2 backend: ${GTK+_2.0_TARGET}")

endif(APPLE)

#-----------------------------------------------------------------------------
# Redirect output files

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin CACHE INTERNAL "" FORCE )
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib CACHE INTERNAL "" FORCE )

# -----------------------------------------------------------------------------
# Test Harness
# -----------------------------------------------------------------------------
set(GMOCK_DIR "${CMAKE_SOURCE_DIR}/gtest/gmock-1.7.0"
  CACHE PATH "The path to the GoogleMock test framework.")

if(EXISTS "${GMOCK_DIR}" AND IS_DIRECTORY "${GMOCK_DIR}")
	set(GMOCK_PRESENT ON)
endif()

# -----------------------------------------------------------------------------
# Options
# -----------------------------------------------------------------------------
option(WITH_DBUS "Compile with support for DBus interface" OFF)
option(ENABLE_LCMS "Compile with LCMS support" ON)
option(WITH_GNOME_VFS "Compile with support for Gnome VFS" ON)
#option(WITH_INKJAR "Enable support for openoffice files (SVG jars)" ON)
option(WITH_GTEST "Compile with Google Test support" ${GMOCK_PRESENT})
option(WITH_OPENMP "Compile with OpenMP support" ON)

option(WITH_PROFILING "Turn on profiling" OFF) # Set to true if compiler/linker should enable profiling

option(WITH_GTKSPELL "Compile with support for GTK spelling widget" ON)
option(ENABLE_POPPLER "Compile with support of libpoppler" ON)
option(ENABLE_POPPLER_CAIRO "Compile with support of libpoppler-cairo for rendering PDF preview (depends on ENABLE_POPPLER)" ON)
option(WITH_IMAGE_MAGICK "Compile with support of ImageMagick for raster extensions and image import resolution" ON)
option(WITH_LIBCDR "Compile with support of libcdr for CorelDRAW Diagrams" ON)
option(WITH_LIBVISIO "Compile with support of libvisio for Microsoft Visio Diagrams" ON)
option(WITH_LIBWPG "Compile with support of libwpg for WordPerfect Graphics" ON)
option(WITH_NLS "Compile with Native Language Support (using gettext)" ON)
option(WITH_GTK3_EXPERIMENTAL "Enable compilation with GTK+3 (EXPERIMENTAL!)" OFF)

include(CMakeScripts/ConfigPaths.cmake) # Installation Paths
include(CMakeScripts/DefineDependsandFlags.cmake) # Includes, Compiler Flags, and Link Libraries
include(CMakeScripts/HelperMacros.cmake) # Misc Utility Macros


# -----------------------------------------------------------------------------
# BAD HACKS, NEED TO INVESTIGATE MAKING THESE LESS BAD
add_definitions(-D_FORTIFY_SOURCE=2)
add_definitions(-DPOTRACE="potrace")
add_definitions(-DORBIT2=1)
add_definitions(-DHAVE_CONFIG_H)
add_definitions(-DHAVE_CAIRO_PDF=1)  # needed for src/libnrtype/Layout-TNG.h
add_definitions(-DHAVE_TR1_UNORDERED_SET) # XXX make an option!
#
# end badness
# -----------------------------------------------------------------------------


# make dist target
set(INKSCAPE_DIST_PREFIX "${PROJECT_NAME}-${INKSCAPE_VERSION}")
add_custom_target(dist
		COMMAND bzr export --root=${INKSCAPE_DIST_PREFIX}
			"${CMAKE_BINARY_DIR}/${INKSCAPE_DIST_PREFIX}.tar.bz2"
		WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

#  svn export --force -q "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/${INKSCAPE_DIST_PREFIX}" 
#  COMMAND tar -czf "${CMAKE_BINARY_DIR}/${INKSCAPE_DIST_PREFIX}.tar.gz" -C "${CMAKE_BINARY_DIR}" --exclude=".hidden" ${INKSCAPE_DIST_PREFIX}

# make unistall target
configure_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
		"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
		IMMEDIATE @ONLY)

configure_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/inkscape.desktop.in"
		"${CMAKE_BINARY_DIR}/inkscape.desktop"
		IMMEDIATE @ONLY)

add_custom_target(uninstall
		"${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")


# -----------------------------------------------------------------------------
# Subdirs (on only 1 atm), but a lot happens here
# -----------------------------------------------------------------------------

add_subdirectory(src)
if(ENABLE_NLS)
  add_subdirectory(po)
endif(ENABLE_NLS)


# -----------------------------------------------------------------------------
# Installation
# -----------------------------------------------------------------------------

if(UNIX)
	# TODO: man, locale

	install(
                PROGRAMS ${EXECUTABLE_OUTPUT_PATH}/inkscape ${EXECUTABLE_OUTPUT_PATH}/inkview
		DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
	)

	install(
		FILES ${CMAKE_BINARY_DIR}/inkscape.desktop
		DESTINATION ${SHARE_INSTALL}/applications
	)

  # this should probably be done no matter what the platform is, just set SHARE_INSTALL first
  add_subdirectory(share) 

else()
	# TODO, WIN32/APPLE
endif()

#-----------------------------------------------------------------------------

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})

if (WITH_GTEST)
	enable_testing()
	add_subdirectory(test EXCLUDE_FROM_ALL)
endif()
