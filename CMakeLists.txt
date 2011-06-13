
project(inkscape)

set(INKSCAPE_VERSION 0.48+devel)
set(PROJECT_NAME inkscape)
cmake_minimum_required(VERSION 2.4.6)
set(CMAKE_INCLUDE_CURRENT_DIR TRUE)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
  cmake_policy(SET CMP0005 NEW) # propper define quoting
endif()

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeScripts")


# XXX, TODO: use pkgconfig, This part is WIP to get building
# - campbell

include_directories(
	 /usr/include/atk-1.0
	 /usr/include/atkmm-1.6
	 /usr/include/cairo
	 /usr/include/cairomm-1.0
	 /usr/include/gconf/2
	 /usr/include/gdk-pixbuf-2.0
	 /usr/include/gdkmm-2.4
	 /usr/include/giomm-2.4
	 /usr/include/glib-2.0
	 /usr/include/glibmm-2.4
	 /usr/include/gnome-vfs-2.0
	 /usr/include/gtk-2.0
	 /usr/include/gtk-unix-print-2.0
	 /usr/include/gtkmm-2.4
	 /usr/include/gtkspell-2.0
	 /usr/include/ImageMagick
	 /usr/include/orbit-2.0
	 /usr/include/pango-1.0
	 /usr/include/pangomm-1.4
	 /usr/include/pixman-1
	 /usr/include/poppler
	 /usr/include/poppler/glib
	 /usr/include/sigc++-2.0
	 /usr/lib/cairomm-1.0/include
	 /usr/lib/gdkmm-2.4/include
	 /usr/lib/giomm-2.4/include
	 /usr/lib/glib-2.0/include
	 /usr/lib/glibmm-2.4/include
	 /usr/lib/gnome-vfs-2.0/include
	 /usr/lib/gtk-2.0/include
	 /usr/lib/gtkmm-2.4/include
	 /usr/lib/pangomm-1.4/include
	 /usr/lib/sigc++-2.0/include
)

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
