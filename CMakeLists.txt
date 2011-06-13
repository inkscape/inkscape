
project(inkscape)

set(INKSCAPE_VERSION 0.48+devel)
set(PROJECT_NAME inkscape)
cmake_minimum_required(VERSION 2.4.6)
set(CMAKE_INCLUDE_CURRENT_DIR TRUE)

if(COMMAND cmake_policy)
  CMAKE_POLICY(SET CMP0003 NEW)
endif()

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeScripts")

# XXX, TODO: use pkgconfig, This part is WIP to get building
# - campbell

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pipe -fPIC -funsigned-char -fno-strict-aliasing  -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe -fPIC -funsigned-char -fno-strict-aliasing  -pthread")
 

include_directories(
	 /usr/include/freetype2
	 /usr/include/gnome-vfs-2.0
	 /usr/lib/gnome-vfs-2.0/include
	 /usr/include/gconf/2
	 /usr/include/orbit-2.0
	 /usr/include/glib-2.0
	 /usr/lib/glib-2.0/include
	 /usr/include/ImageMagick
	 /usr/include/poppler
	 /usr/include/poppler/glib
	 /usr/include/poppler
	 /usr/include/glib-2.0
	 /usr/lib/glib-2.0/include
	 /usr/include/cairo
	 /usr/include/gtk-2.0
	 /usr/lib/gtk-2.0/include
	 /usr/include/gdk-pixbuf-2.0
	 /usr/include/pixman-1
	 /usr/include/freetype2
	 /usr/include/libpng14
	 /usr/include/pango-1.0
	 /usr/include/glib-2.0
	 /usr/lib/glib-2.0/include
	 /usr/include/gtk-2.0
	 /usr/lib/gtk-2.0/include
	 /usr/include/atk-1.0
	 /usr/include/cairo
	 /usr/include/gdk-pixbuf-2.0
	 /usr/include/pango-1.0
	 /usr/include/pixman-1
	 /usr/include/freetype2
	 /usr/include/libpng14
	 /usr/include/libxml2
	 /usr/include/sigc++-2.0
	 /usr/lib/sigc++-2.0/include
	 /usr/include/gtkspell-2.0
	 /usr/include/glibmm-2.4
	 /usr/lib/glibmm-2.4/include
	 /usr/include/giomm-2.4
	 /usr/lib/giomm-2.4/include
	 /usr/include/gdkmm-2.4
	 /usr/lib/gdkmm-2.4/include
	 /usr/include/pangomm-1.4
	 /usr/lib/pangomm-1.4/include
	 /usr/include/cairomm-1.0
	 /usr/lib/cairomm-1.0/include
	 /usr/include/gtkmm-2.4
	 /usr/lib/gtkmm-2.4/include
	 /usr/include/atkmm-1.6
	 /usr/include/gtk-unix-print-2.0
)

add_definitions(-D_FORTIFY_SOURCE=2)
add_definitions(-DPOTRACE="potrace")
add_definitions(-DORBIT2=1)
add_definitions(-DHAVE_CONFIG_H)
add_definitions(-DHAVE_TR1_UNORDERED_SET) # XXX make an option!
add_definitions(-DVERSION="\\"0.48+devel\\"") # XXX make an option!
# XXX - end hackish defines!



include(ConfigPaths) #Installation Paths
include(DefineDependsandFlags) #Includes, Compiler Flags, and Link Libraries
include(HelperMacros) #Misc Utility Macros

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

set(folders
	src
)

foreach(dirlist ${folders})
	add_subdirectory(${dirlist})
endforeach()
