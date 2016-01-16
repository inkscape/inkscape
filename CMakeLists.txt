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
	cmake_policy(SET CMP0003 NEW) # don't be prolific with library paths
	cmake_policy(SET CMP0005 NEW) # proper define quoting
	cmake_policy(SET CMP0009 NEW) # don't follow symbolic links when using GLOB
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
# Man pages
# -----------------------------------------------------------------------------

find_program(POD2MAN pod2man)
if(NOT POD2MAN)
    message(STATUS "Could not find pod2man - man pages disabled")
endif()

macro(pod2man PODFILE_FULL RELEASE SECTION CENTER)
    get_filename_component(PODFILE ${PODFILE_FULL} NAME)
    string(REPLACE "." ";" PODFILE_LIST ${PODFILE})
    list(GET PODFILE_LIST 0 NAME)
    list(GET PODFILE_LIST 1 LANG)
    string(TOUPPER ${NAME} NAME_UPCASE)
    if(${LANG} STREQUAL "pod")
        set(LANG "")
    endif()

    if(NOT EXISTS ${PODFILE_FULL})
        message(FATAL ERROR "Could not find pod file ${PODFILE_FULL} to generate man page")
    endif(NOT EXISTS ${PODFILE_FULL})

    if(POD2MAN)
        if(LANG)
            set(MANPAGE_TARGET "man-${NAME}-${LANG}")
            set(MANFILE_TEMP "${CMAKE_CURRENT_BINARY_DIR}/man/${NAME}.${LANG}.tmp")
            set(MANFILE_FULL "${CMAKE_CURRENT_BINARY_DIR}/man/${NAME}.${LANG}.${SECTION}")
        else()
            set(MANPAGE_TARGET "man-${NAME}")
            set(MANFILE_TEMP "${CMAKE_CURRENT_BINARY_DIR}/man/${NAME}.tmp")
            set(MANFILE_FULL "${CMAKE_CURRENT_BINARY_DIR}/man/${NAME}.${SECTION}")
        endif()
        add_custom_command(
            OUTPUT ${MANFILE_TEMP}
            COMMAND ${POD2MAN} --utf8 --section="${SECTION}" --center="${CENTER}"
                --release="${RELEASE}" --name="${NAME_UPCASE}" "${PODFILE_FULL}" "${MANFILE_TEMP}"
        )
        add_custom_command(
            OUTPUT ${MANFILE_FULL}
            COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/fix-roff-punct "${MANFILE_TEMP}" > ${MANFILE_FULL}
	    DEPENDS ${MANFILE_TEMP}
        )
        add_custom_target(${MANPAGE_TARGET} ALL
            DEPENDS ${MANFILE_FULL}
        )
        install(
            FILES ${MANFILE_FULL}
            DESTINATION ${CMAKE_INSTALL_MANDIR}/man${SECTION}
        )
    endif()
endmacro(pod2man PODFILE NAME SECTION CENTER)

function(JOIN OUTPUT GLUE)
    set(_TMP_RESULT "")
    set(_GLUE "") # effective glue is empty at the beginning
    foreach(arg ${ARGN})
        # Skip empty lines
        if(NOT arg STREQUAL "\n")
            set(_TMP_RESULT "${_TMP_RESULT}${_GLUE}${arg}")
            set(_GLUE "${GLUE}")
        endif()
    endforeach()
    set(${OUTPUT} "${_TMP_RESULT}" PARENT_SCOPE)
endfunction()

# Load AUTHORS file contents into $INKSCAPE_AUTHORS
FILE(READ ${CMAKE_CURRENT_SOURCE_DIR}/AUTHORS content)
STRING(REGEX REPLACE "^([^\n#]+)\n" "\\1;\n" content_list "${content}")
JOIN(INKSCAPE_AUTHORS "," "${content_list}")

foreach(podfile
        man/inkscape.pod
        man/inkscape.de.pod
        man/inkscape.el.pod
        man/inkscape.fr.pod
        man/inkscape.ja.pod
        man/inkscape.sk.pod
        man/inkscape.zh_TW.pod)
    set(POD_IN ${CMAKE_CURRENT_SOURCE_DIR}/${podfile}.in)
    set(POD_OUT ${CMAKE_CURRENT_BINARY_DIR}/${podfile})
    configure_file(${POD_IN} ${POD_OUT})
    pod2man(${POD_OUT} ${INKSCAPE_VERSION} 1 "Inkscape Commands Manual")
endforeach()

# -----------------------------------------------------------------------------
# Installation
# -----------------------------------------------------------------------------

if(UNIX)
	# TODO: locale

	install(
                PROGRAMS ${EXECUTABLE_OUTPUT_PATH}/inkscape ${EXECUTABLE_OUTPUT_PATH}/inkview
		DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
	)

	install(
		FILES ${CMAKE_BINARY_DIR}/inkscape.desktop
		DESTINATION ${SHARE_INSTALL}/applications
	)

	if(POD2MAN)
		install(
			FILES ${CMAKE_CURRENT_BINARY_DIR}/inkscape.1
			DESTINATION ${SHARE_INSTALL}/man/man1
		)
	endif()

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
