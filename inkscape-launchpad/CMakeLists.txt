cmake_minimum_required(VERSION 2.8.0)

message("------------------------------")
message("Building Makefile for Inkscape")
message("------------------------------")
message("Source Dir: ${CMAKE_CURRENT_SOURCE_DIR}")
message("Binary Dir: ${CMAKE_CURRENT_BINARY_DIR}")

# -----------------------------------------------------------------------------
# CMake Configuration
# -----------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeScripts/Modules")

# avoid having empty buildtype
set(CMAKE_BUILD_TYPE_INIT "Release")

project(inkscape)

set(INKSCAPE_VERSION 0.91+devel)
set(PROJECT_NAME inkscape)
set(CMAKE_INCLUDE_CURRENT_DIR TRUE)

cmake_policy(SET CMP0003 NEW) # don't be prolific with library paths
cmake_policy(SET CMP0005 NEW) # proper define quoting
cmake_policy(SET CMP0009 NEW) # don't follow symbolic links when using GLOB

# workaround for omission in cmake 2.8.4's GNU.cmake, fixed in 2.8.5
if(CMAKE_COMPILER_IS_GNUCC)
    if(NOT DARWIN)
	set(CMAKE_INCLUDE_SYSTEM_FLAG_C "-isystem ")
    endif()
endif()

include(CMakeScripts/HelperFunctions.cmake)

# -----------------------------------------------------------------------------
# Set platform defaults (originally copied from darktable)
# -----------------------------------------------------------------------------
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
    pkg_check_variable(gtk+-2.0 target)
    message("GTK2 backend: ${GTK+_2.0_TARGET}")

endif(APPLE)

# -----------------------------------------------------------------------------
# Redirect output files
# -----------------------------------------------------------------------------
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

# -----------------------------------------------------------------------------
# Dist Target
# -----------------------------------------------------------------------------
set(INKSCAPE_DIST_PREFIX "${PROJECT_NAME}-${INKSCAPE_VERSION}")
add_custom_target(dist
    COMMAND bzr export --root=${INKSCAPE_DIST_PREFIX}
    "${CMAKE_BINARY_DIR}/${INKSCAPE_DIST_PREFIX}.tar.bz2"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# -----------------------------------------------------------------------------
# Uninstall Target
# -----------------------------------------------------------------------------
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
include("CMakeScripts/Pod2man.cmake")

# Load AUTHORS file contents into $INKSCAPE_AUTHORS
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/AUTHORS content)
string(REGEX REPLACE "^([^\n#]+)\n" "\\1;\n" content_list "${content}")
join(INKSCAPE_AUTHORS "," "${content_list}")

foreach(podfile
        man/inkscape.pod
        man/inkscape.de.pod
        man/inkscape.el.pod
        man/inkscape.fr.pod
        man/inkscape.ja.pod
        man/inkscape.sk.pod
        man/inkscape.zh_TW.pod
	man/inkview.pod)
    set(POD_IN ${CMAKE_CURRENT_SOURCE_DIR}/${podfile}.in)
    set(POD_OUT ${CMAKE_CURRENT_BINARY_DIR}/${podfile})
    configure_file(${POD_IN} ${POD_OUT})
    pod2man(${POD_OUT} ${INKSCAPE_VERSION} 1 "Inkscape Commands Manual")
endforeach()

# -----------------------------------------------------------------------------
# Installation
# -----------------------------------------------------------------------------
if(UNIX)
    install(
        PROGRAMS ${EXECUTABLE_OUTPUT_PATH}/inkscape ${EXECUTABLE_OUTPUT_PATH}/inkview
	DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
	)

    install(
	FILES ${CMAKE_BINARY_DIR}/inkscape.desktop
	DESTINATION ${CMAKE_INSTALL_PREFIX}/${SHARE_INSTALL}/applications
	)

    # this should probably be done no matter what the platform is, just set SHARE_INSTALL first
    add_subdirectory(share) 

else()
    # TODO, WIN32, APPLE/OSX, MinGW
endif()

# -----------------------------------------------------------------------------
# Clean
# -----------------------------------------------------------------------------
add_custom_target(clean-cmake-files
    COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_SOURCE_DIR}/CMakeScripts/CleanAll.cmake"
)

add_custom_target(clean-all
    COMMAND ${CMAKE_BUILD_TOOL} clean
    COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_SOURCE_DIR}/CMakeScripts/CleanAll.cmake"
)

#-----------------------------------------------------------------------------

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})

if(WITH_GTEST)
    enable_testing()
    add_subdirectory(test EXCLUDE_FROM_ALL)
endif()
