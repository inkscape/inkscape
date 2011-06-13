# CMake TODO
# - Remove hard coded defines (see bwelow)
# - Test on MinGW and OSX
# - Add configurable options for Python/Perl/Whiteboard, see configure --help
#
# ideasman42

project(inkscape)

set(INKSCAPE_VERSION 0.48+devel)
set(PROJECT_NAME inkscape)
cmake_minimum_required(VERSION 2.8.0)
set(CMAKE_INCLUDE_CURRENT_DIR TRUE)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
  cmake_policy(SET CMP0005 NEW) # propper define quoting
endif()

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeScripts")


# XXX, TODO: use pkgconfig, This part is WIP to get building
# - campbell
add_definitions(-D_FORTIFY_SOURCE=2)
add_definitions(-DPOTRACE="potrace")
add_definitions(-DORBIT2=1)
add_definitions(-DHAVE_CONFIG_H)
add_definitions(-DHAVE_CAIRO_PDF=1)  # needed for src/libnrtype/Layout-TNG.h
add_definitions(-DHAVE_TR1_UNORDERED_SET) # XXX make an option!
# XXX - end hackish defines!


include(ConfigPaths) # Installation Paths
include(DefineDependsandFlags) # Includes, Compiler Flags, and Link Libraries
include(HelperMacros) # Misc Utility Macros

option(ENABLE_PROFILING "Turn on profiling" OFF) # Set to true if compiler/linker should enable profiling

# make dist target
set(INKSCAPE_DIST_PREFIX "${PROJECT_NAME}-${INKSCAPE_VERSION}")
add_custom_target(dist svn export --force -q "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/${INKSCAPE_DIST_PREFIX}" 
		COMMAND tar -czf "${CMAKE_BINARY_DIR}/${INKSCAPE_DIST_PREFIX}.tar.gz" -C "${CMAKE_BINARY_DIR}" --exclude=".hidden" ${INKSCAPE_DIST_PREFIX}
		WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# make unistall target
configure_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
		"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
		IMMEDIATE @ONLY)

add_custom_target(uninstall
		"${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")


add_subdirectory(src)
