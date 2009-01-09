# - FindGTK2.cmake
# This module finds the GTK2/GTKMM widget libraries
#
# Specify one or more of the following components
# as you call this Find macro:
#
#   gtk
#   gtkmm
#   glade
#   glademm
#
# The following variables will be defined for your use
#   GTK2_FOUND - Were all of your specified components found?
#   GTK2_INCLUDE_DIRS - All include directories
#   GTK2_LIBRARIES - All libraries
#
# Optional variables you can define prior to calling this module
#   GTK2_DEBUG - Enables verbose debugging of the module
#   GTK2_SKIP_MARK_AS_ADVANCED - Disable marking cache variables as advanced
#
#=================
# Example Usage:
#
#  FIND_PACKAGE(GTK2 COMPONENTS gtk)
#      or
#  FIND_PACKAGE(GTK2 COMPONENTS gtk glade) # if you're also using glade
#
#  INCLUDE_DIRECTORIES(${GTK2_INCLUDE_DIRS})
#  ADD_EXECUTABLE(foo foo.cc)
#  TARGET_LINK_LIBRARIES(foo ${GTK2_LIBRARIES})
#=================
# 
# Copyright (c) 2008
#     Philip Lowman <philip@yhbt.com>
#
# Version 0.6 (1/8/08)
#   Added GTK2_SKIP_MARK_AS_ADVANCED option
# Version 0.5 (12/19/08)
#   Second release to cmake mailing list
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

#=============================================================
# _GTK2_FIND_INCLUDE_DIR
# Internal macro to find the GTK include directories
#   _var = variable to set
#   _hdr = header file to look for
#=============================================================
MACRO(_GTK2_FIND_INCLUDE_DIR _var _hdr)

    IF(GTK2_DEBUG)
        MESSAGE(STATUS "[FindGTK2.cmake:${CMAKE_CURRENT_LIST_LINE}]
            _GTK2_FIND_INCLUDE_DIR( ${_var} ${_hdr} )")
    ENDIF()

    SET(_relatives
        # FIXME
        glibmm-2.4
        glib-2.0
        atk-1.0
        atkmm-1.6
        cairo
        cairomm-1.0
        gdkmm-2.4
        giomm-2.4
        gtk-2.0
        gtkmm-2.4
        libglade-2.0
        libglademm-2.4
        pango-1.0
        pangomm-1.4
        sigc++-2.0
    )

    SET(_suffixes)
    FOREACH(_d ${_relatives})
        LIST(APPEND _suffixes ${_d})
        LIST(APPEND _suffixes ${_d}/include) # for /usr/lib/gtk-2.0/include
    ENDFOREACH()

    IF(GTK2_DEBUG)
        MESSAGE(STATUS "[FindGTK2.cmake:${CMAKE_CURRENT_LIST_LINE}] include suffixes = ${_suffixes}")
    ENDIF()

    FIND_PATH(${_var} ${_hdr}
        PATHS
            /usr/local/include
            /usr/local/lib
            /usr/include
            /usr/lib
            [HKEY_LOCAL_MACHINE\\SOFTWARE\\gtkmm\\2.4;Path]/include
            [HKEY_LOCAL_MACHINE\\SOFTWARE\\gtkmm\\2.4;Path]/lib
        PATH_SUFFIXES
            ${_suffixes}
    )

    IF(${_var})
        LIST(APPEND GTK2_INCLUDE_DIRS ${${_var}})
        IF(NOT GTK2_SKIP_MARK_AS_ADVANCED)
            MARK_AS_ADVANCED(${_var})
        ENDIF()
    ENDIF()
ENDMACRO()

#=============================================================
# _GTK2_FIND_LIBRARY
# Internal macro to find libraries packaged with GTK2
#   _var = library variable to create
#=============================================================
MACRO(_GTK2_FIND_LIBRARY _var _lib _expand_vc _append_version)

    IF(GTK2_DEBUG)
        MESSAGE(STATUS "[FindGTK2.cmake:${CMAKE_CURRENT_LIST_LINE}]
            _GTK2_FIND_LIBRARY( ${_var} ${_lib} ${_expand_vc} ${_append_version} )")
    ENDIF()

    # Not GTK versions per se but the versions encoded into Windows
    # import libraries (GtkMM 2.14.1 has a gtkmm-vc80-2_4.lib for example)
    # Also the MSVC libraries use _ for . (this is handled below)
    SET(_versions 2.20 2.18 2.16 2.14 2.12
                  2.10  2.8  2.6  2.4  2.2 2.0
                  1.20 1.18 1.16 1.14 1.12
                  1.10  1.8  1.6  1.4  1.2 1.0)

    SET(_library)
    SET(_library_d)

    SET(_library ${_lib})

    IF(${_expand_vc})
        # Add vc80/vc90 midfixes
        IF(MSVC80)
            SET(_library   ${_library}-vc80)
            SET(_library_d ${_library}-d)
        ELSEIF(MSVC90)
            SET(_library   ${_library}-vc90)
            SET(_library_d ${_library}-d)
        ENDIF()
    ENDIF()

    IF(GTK2_DEBUG)
        MESSAGE(STATUS "[FindGTK2.cmake:${CMAKE_CURRENT_LIST_LINE}] After midfix addition = ${_library} and ${_library_d}")
    ENDIF()

    SET(_lib_list)
    SET(_libd_list)
    IF(${_append_version})
        FOREACH(_ver ${_versions})
            LIST(APPEND _lib_list  "${_library}-${_ver}")
            LIST(APPEND _libd_list "${_library_d}-${_ver}")
        ENDFOREACH()
    ELSE()
        SET(_lib_list ${_library})
        SET(_libd_list ${_library_d})
    ENDIF()
    
    IF(GTK2_DEBUG)
        MESSAGE(STATUS "[FindGTK2.cmake:${CMAKE_CURRENT_LIST_LINE}] library list = ${_lib_list} and library debug list = ${_libd_list}")
    ENDIF()

    # For some silly reason the MSVC libraries use _ instead of .
    # in the version fields
    IF(${_expand_vc} AND MSVC)
        SET(_no_dots_lib_list)
        SET(_no_dots_libd_list)
        FOREACH(_l ${_lib_list})
            STRING(REPLACE "." "_" _no_dots_library ${_l})
            LIST(APPEND _no_dots_lib_list ${_no_dots_library})
        ENDFOREACH()
        # And for debug
        SET(_no_dots_libsd_list)
        FOREACH(_l ${_libd_list})
            STRING(REPLACE "." "_" _no_dots_libraryd ${_l})
            LIST(APPEND _no_dots_libd_list ${_no_dots_libraryd})
        ENDFOREACH()

        # Copy list back to original names
        SET(_lib_list ${_no_dots_lib_list})
        SET(_libd_list ${_no_dots_libd_list})
    ENDIF()

    IF(GTK2_DEBUG)
        MESSAGE(STATUS "[FindGTK2.cmake:${CMAKE_CURRENT_LIST_LINE}] Whilst searching for ${_var} our proposed library list is ${_lib_list}")
    ENDIF()

    FIND_LIBRARY(${_var} 
        NAMES ${_lib_list}
        PATHS
        [HKEY_LOCAL_MACHINE\\SOFTWARE\\gtkmm\\2.4;Path]/lib
        )
    IF(${_expand_vc} AND MSVC)

        IF(GTK2_DEBUG)
            MESSAGE(STATUS "[FindGTK2.cmake:${CMAKE_CURRENT_LIST_LINE}] Whilst searching for ${_var}_DEBUG our proposed library list is ${_libd_list}")
        ENDIF()

        FIND_LIBRARY(${_var}_DEBUG
            NAMES ${_libd_list}
            PATHS
            [HKEY_LOCAL_MACHINE\\SOFTWARE\\gtkmm\\2.4;Path]/lib
        )
        IF(NOT GTK2_SKIP_MARK_AS_ADVANCED)
            MARK_AS_ADVANCED(${_var}_DEBUG)
        ENDIF()
    ELSE()
        SET(${_var}_DEBUG ${${_var}})
    ENDIF()

    IF(${_var})
        LIST(APPEND GTK2_LIBRARIES optimized ${${_var}} debug ${${_var}_DEBUG} )
        IF(NOT GTK2_SKIP_MARK_AS_ADVANCED)
            MARK_AS_ADVANCED(${_var})
        ENDIF()
    ENDIF()
ENDMACRO()
#=============================================================

#
# main()
#

SET(GTK2_FOUND)
SET(GTK2_INCLUDE_DIRS)
SET(GTK2_LIBRARIES)

IF(NOT GTK2_FIND_COMPONENTS)
    MESSAGE(FATAL_ERROR "You must specify components with this module.  See the documentation at the top of FindGTK2.cmake")
ENDIF()

FOREACH(_component ${GTK2_FIND_COMPONENTS})
    IF(_component STREQUAL "gtk")
        
        _GTK2_FIND_INCLUDE_DIR(GTK2_GLIB_INCLUDE_DIR glib.h)
        _GTK2_FIND_INCLUDE_DIR(GTK2_GLIBCONFIG_INCLUDE_DIR glibconfig.h)
        _GTK2_FIND_LIBRARY    (GTK2_GLIB_LIBRARY glib 0 1)

        _GTK2_FIND_INCLUDE_DIR(GTK2_GDK_INCLUDE_DIR gdk/gdk.h)
        _GTK2_FIND_INCLUDE_DIR(GTK2_GDKCONFIG_INCLUDE_DIR gdkconfig.h)
        _GTK2_FIND_LIBRARY    (GTK2_GDK_LIBRARY gdk-x11 0 1)
        _GTK2_FIND_LIBRARY    (GTK2_GDK_LIBRARY gdk-win32 0 1)

        _GTK2_FIND_INCLUDE_DIR(GTK2_GTK_INCLUDE_DIR gtk/gtk.h)
        _GTK2_FIND_LIBRARY    (GTK2_GTK_LIBRARY gtk-x11 0 1)
        _GTK2_FIND_LIBRARY    (GTK2_GTK_LIBRARY gtk-win32 0 1)

        _GTK2_FIND_INCLUDE_DIR(GTK2_CAIRO_INCLUDE_DIR cairo.h)
        _GTK2_FIND_LIBRARY    (GTK2_CAIRO_LIBRARY cairo 0 0)

        _GTK2_FIND_INCLUDE_DIR(GTK2_PANGO_INCLUDE_DIR pango/pango.h)
        _GTK2_FIND_LIBRARY    (GTK2_PANGO_LIBRARY pango 0 1)

        _GTK2_FIND_INCLUDE_DIR(GTK2_ATK_INCLUDE_DIR atk/atk.h)
        _GTK2_FIND_LIBRARY    (GTK2_ATK_LIBRARY atk 0 1)

        #ELSEIF(_component STREQUAL "gdk_pixbuf")
        #_GTK2_FIND_INCLUDE_DIR(GTK2_GDKPIXBUF_INCLUDE_DIR gdk-pixbuf/gdk-pixbuf.h)
        #_GTK2_FIND_LIBRARY    (GTK2_GDKPIXBUF_LIBRARY gdk_pixbuf 0 1)

    ELSEIF(_component STREQUAL "gtkmm")

        _GTK2_FIND_INCLUDE_DIR(GTK2_GLIBMM_INCLUDE_DIR glibmm.h)
        _GTK2_FIND_INCLUDE_DIR(GTK2_GLIBMMCONFIG_INCLUDE_DIR glibmmconfig.h)
        _GTK2_FIND_LIBRARY    (GTK2_GLIBMM_LIBRARY glibmm 1 1)

        _GTK2_FIND_INCLUDE_DIR(GTK2_GDKMM_INCLUDE_DIR gdkmm.h)
        _GTK2_FIND_INCLUDE_DIR(GTK2_GDKMMCONFIG_INCLUDE_DIR gdkmmconfig.h)
        _GTK2_FIND_LIBRARY    (GTK2_GDKMM_LIBRARY gdkmm 1 1)

        _GTK2_FIND_INCLUDE_DIR(GTK2_GTKMM_INCLUDE_DIR gtkmm.h)
        _GTK2_FIND_INCLUDE_DIR(GTK2_GTKMMCONFIG_INCLUDE_DIR gtkmmconfig.h)
        _GTK2_FIND_LIBRARY    (GTK2_GTKMM_LIBRARY gtkmm 1 1)

        _GTK2_FIND_INCLUDE_DIR(GTK2_CAIROMM_INCLUDE_DIR cairomm/cairomm.h)
        _GTK2_FIND_LIBRARY    (GTK2_CAIROMM_LIBRARY cairomm 1 1)

        _GTK2_FIND_INCLUDE_DIR(GTK2_PANGOMM_INCLUDE_DIR pangomm.h)
        _GTK2_FIND_LIBRARY    (GTK2_PANGOMM_LIBRARY pangomm 1 1)

        _GTK2_FIND_INCLUDE_DIR(GTK2_SIGC++_INCLUDE_DIR sigc++/sigc++.h)
        _GTK2_FIND_INCLUDE_DIR(GTK2_SIGC++CONFIG_INCLUDE_DIR sigc++config.h)
        _GTK2_FIND_LIBRARY    (GTK2_SIGC++_LIBRARY sigc 1 1)

        _GTK2_FIND_INCLUDE_DIR(GTK2_GIOMM_INCLUDE_DIR giomm.h)
        _GTK2_FIND_INCLUDE_DIR(GTK2_GIOMMCONFIG_INCLUDE_DIR giommconfig.h)
        _GTK2_FIND_LIBRARY    (GTK2_GIOMM_LIBRARY giomm 1 1)

        _GTK2_FIND_INCLUDE_DIR(GTK2_ATKMM_INCLUDE_DIR atkmm.h)
        _GTK2_FIND_LIBRARY    (GTK2_ATKMM_LIBRARY atkmm 1 1)

    ELSEIF(_component STREQUAL "glade")

        _GTK2_FIND_INCLUDE_DIR(GTK2_GLADE_INCLUDE_DIR glade/glade.h)
        _GTK2_FIND_LIBRARY    (GTK2_GLADE_LIBRARY glade 0 1)
    
    ELSEIF(_component STREQUAL "glademm")

        _GTK2_FIND_INCLUDE_DIR(GTK2_GLADEMM_INCLUDE_DIR libglademm.h)
        _GTK2_FIND_INCLUDE_DIR(GTK2_GLADEMMCONFIG_INCLUDE_DIR libglademmconfig.h)
        _GTK2_FIND_LIBRARY    (GTK2_GLADEMM_LIBRARY glademm 1 1)

    ELSE()
        MESSAGE(FATAL_ERROR "Unknown GTK2 component ${_component}")
    ENDIF()
ENDFOREACH()

#
# Try to enforce components
#

SET(_did_we_find_everything true)  # This gets set to GTK2_FOUND

INCLUDE(FindPackageHandleStandardArgs)

FOREACH(_component ${GTK2_FIND_COMPONENTS})
    STRING(TOUPPER ${_component} _COMPONENT_UPPER)

    IF(_component STREQUAL "gtk")
        FIND_PACKAGE_HANDLE_STANDARD_ARGS(GTK2_${_COMPONENT_UPPER} "Some or all of the gtk libraries were not found."
            GTK2_GTK_LIBRARY
            GTK2_GTK_INCLUDE_DIR

            GTK2_GLIB_INCLUDE_DIR
            GTK2_GLIBCONFIG_INCLUDE_DIR
            GTK2_GLIB_LIBRARY

            GTK2_GDK_INCLUDE_DIR
            GTK2_GDKCONFIG_INCLUDE_DIR
            GTK2_GDK_LIBRARY
        )
    ELSEIF(_component STREQUAL "gtkmm")
        FIND_PACKAGE_HANDLE_STANDARD_ARGS(GTK2_${_COMPONENT_UPPER} "Some or all of the gtkmm libraries were not found."
            GTK2_GTKMM_LIBRARY
            GTK2_GTKMM_INCLUDE_DIR
            GTK2_GTKMMCONFIG_INCLUDE_DIR

            GTK2_GLIBMM_INCLUDE_DIR
            GTK2_GLIBMMCONFIG_INCLUDE_DIR
            GTK2_GLIBMM_LIBRARY

            GTK2_GDKMM_INCLUDE_DIR
            GTK2_GDKMMCONFIG_INCLUDE_DIR
            GTK2_GDKMM_LIBRARY
        )
    ELSEIF(_component STREQUAL "glade")
        FIND_PACKAGE_HANDLE_STANDARD_ARGS(GTK2_${_COMPONENT_UPPER} "The glade library was not found."
            GTK2_GLADE_LIBRARY
            GTK2_GLADE_INCLUDE_DIR
        )
    ELSEIF(_component STREQUAL "glademm")
        FIND_PACKAGE_HANDLE_STANDARD_ARGS(GTK2_${_COMPONENT_UPPER} "The glademm library was not found."
            GTK2_GLADEMM_LIBRARY
            GTK2_GLADEMM_INCLUDE_DIR
            GTK2_GLADEMMCONFIG_INCLUDE_DIR
        )
    ENDIF()

    IF(NOT GTK2_${_COMPONENT_UPPER}_FOUND)
        SET(_did_we_find_everything false)
    ENDIF()
ENDFOREACH()

SET(GTK2_FOUND ${_did_we_find_everything})
