#
# try to find GTK2 (and glib) and GTK2GLArea
#
# GTK2_INCLUDE_DIRS  - Directories to include to use GTK2
# GTK2_LIBRARIES     - Files to link against to use GTK2
# GTK2_FOUND         - If false, don't try to use GTK2
# GTK2_GL_FOUND      - If false, don't try to use GTK2's GL features
#
###################################################################
#
#  Copyright (c) 2004 Jan Woetzel
#  Copyright (c) 2006 Andreas Schneider <mail@cynapses.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor,
# Boston, MA  02110-1301, USA.
#
###################################################################
#
#  Copyright (c) 2004 Jan Woetzel
#  Copyright (c) 2006 Andreas Schneider <mail@cynapses.org>
#  All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in
#   the documentation and/or other materials provided with the
#   distribution.
#
# * Neither the name of the <ORGANIZATION> nor the names of its
#   contributors may be used to endorse or promote products derived
#   from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

IF (GTK2_LIBRARIES AND GTK2_INCLUDE_DIRS)
  # in cache already
  SET(GTK2_FOUND TRUE)
ELSE (GTK2_LIBRARIES AND GTK2_INCLUDE_DIRS)

    #check whether cmake version is 2.4 or greater
    if (${CMAKE_MAJOR_VERSION} GREATER 1 AND ${CMAKE_MINOR_VERSION} GREATER 3)
      SET(GTK2_CMAKE_24 TRUE)
    endif (${CMAKE_MAJOR_VERSION} GREATER 1 AND ${CMAKE_MINOR_VERSION} GREATER 3)

    # use pkg-config to get the directories and then use these values
    # in the FIND_PATH() and FIND_LIBRARY() calls
    if (GTK2_CMAKE_24)
      include(UsePkgConfig)
    else (GTK2_CMAKE_24)
      find_package(PkgConfig)
    endif (GTK2_CMAKE_24)
    INCLUDE(UsePkgConfig)

    if (GTK2_CMAKE_24)
      pkgconfig(gtk+-2.0 _GTK2_INCLUDEDIR _GTK2_LIBDIR _GTK2_LDFLAGS _GTK2_CFLAGS)
     MESSAGE(STATUS "DEBUG pkgconfig found: ${_GTK2_INCLUDEDIR} ${_GTK2_LIBDIR} ${_GTK2_LDFLAGS} ${_GTK2_CFLAGS}")
    else (GTK2_CMAKE_24)
      if (PKG_CONFIG_FOUND)
        pkg_check_modules(_GTK2 gtk+-2.0)
      endif (PKG_CONFIG_FOUND)
    endif (GTK2_CMAKE_24)

    FIND_PATH(GTK2_GTK_INCLUDE_PATH gtk/gtk.h
      $ENV{GTK2_HOME}
      ${_GTK2_INCLUDEDIR}
      /usr/include/gtk-2.0
      /usr/local/include/gtk-2.0
      /opt/gnome/include/gtk-2.0
      /opt/local/include/gtk-2.0
      $ENV{DEVLIBS_PATH}//include//
    )

    # Some Linux distributions (e.g. Red Hat) have glibconfig.h
    # and glib.h in different directories, so we need to look
    # for both.
    #  - Atanas Georgiev <atanas@cs.columbia.edu>
    if (GTK2_CMAKE_24)
      pkgconfig(glib-2.0 _GLIB2_INCLUDEDIR _GLIB2inkDir _GLIB2_LDFLAGS _GLIB2_CFLAGS)
      pkgconfig(gmodule-2.0 _GMODULE2_INCLUDEDIR _GMODULE2inkDir _GMODULE2_LDFLAGS _GMODULE2_CFLAGS)
    else (GTK2_CMAKE_24)
      if (PKG_CONFIG_FOUND)
        pkg_check_modules(_GLIB2 glib-2.0)
        pkg_check_modules(_GMODULE2 gmodule-2.0)
      endif (PKG_CONFIG_FOUND)
    endif (GTK2_CMAKE_24)

    SET(GDIR /opt/gnome/lib/glib-2.0/include)
    FIND_PATH(GTK2_GLIBCONFIG_INCLUDE_PATH glibconfig.h
      ${_GLIB2_INCLUDEDIR}
      /opt/gnome/lib64/glib-2.0/include
      /opt/gnome/lib/glib-2.0/include
      /usr/lib64/glib-2.0/include
      /usr/lib/glib-2.0/include
      /opt/local/lib/glib-2.0/include
      $ENV{DEVLIBS_PATH}//include//
    )
    #MESSAGE(STATUS "DEBUG: GTK2_GLIBCONFIG_INCLUDE_PATH = ${GTK2_GLIBCONFIG_INCLUDE_PATH}")

    FIND_PATH(GTK2_GLIB_INCLUDE_PATH glib.h
      ${_GLIB2_INCLUDEDIR}
      /opt/gnome/include/glib-2.0
      /usr/include/glib-2.0
      /opt/local/include/glib-2.0
      $ENV{DEVLIBS_PATH}//include//
    )
    #MESSAGE(STATUS "DEBUG: GTK2_GLIBCONFIG_INCLUDE_PATH = ${GTK2_GLIBCONFIG_INCLUDE_PATH}")

    FIND_PATH(GTK2_GTKGL_INCLUDE_PATH gtkgl/gtkglarea.h
      ${_GLIB2_INCLUDEDIR}
      /usr/include
      /usr/local/include
      /usr/openwin/share/include
      /opt/gnome/include
      /opt/local/include
      $ENV{DEVLIBS_PATH}//include//
    )

    if (GTK2_CMAKE_24)
      pkgconfig(pango _PANGO_INCLUDEDIR _PANGOinkDir _PANGO_LDFLAGS _PANGO_CFLAGS)
    else (GTK2_CMAKE_24)
      if (PKG_CONFIG_FOUND)
        pkg_check_modules(_PANGO pango)
      endif (PKG_CONFIG_FOUND)
    endif (GTK2_CMAKE_24)

    FIND_PATH(GTK2_PANGO_INCLUDE_PATH pango/pango.h
      ${_PANGO_INCLUDEDIR}
      /opt/gnome/include/pango-1.0
      /usr/include/pango-1.0
      /opt/local/include/pango-1.0
      $ENV{DEVLIBS_PATH}//include//
    )

    if (GTK2_CMAKE_24)
      pkgconfig(gdk-2.0 _GDK2_INCLUDEDIR _GDK2inkDir _GDK2_LDFLAGS _GDK2_CFLAGS)
    else (GTK2_CMAKE_24)
      if (PKG_CONFIG_FOUND)
        pkg_check_modules(_GDK2 gdk-2.0)
      endif (PKG_CONFIG_FOUND)
    endif (GTK2_CMAKE_24)

    FIND_PATH(GTK2_GDKCONFIG_INCLUDE_PATH gdkconfig.h
      ${_GDK2_INCLUDEDIR}
      /opt/gnome/lib/gtk-2.0/include
      /opt/gnome/lib64/gtk-2.0/include
      /usr/lib/gtk-2.0/include
      /usr/lib64/gtk-2.0/include
      /opt/local/lib/gtk-2.0/include
      $ENV{DEVLIBS_PATH}//include//
    )

    if (GTK2_CMAKE_24)
      pkgconfig(cairo _CAIRO_INCLUDEDIR _CAIROinkDir _CAIRO_LDFLAGS _CAIRO_CFLAGS)
    else (GTK2_CMAKE_24)
      if (PKG_CONFIG_FOUND)
        pkg_check_modules(_CAIRO cairo)
      endif (PKG_CONFIG_FOUND)
    endif (GTK2_CMAKE_24)

    FIND_PATH(GTK2_CAIRO_INCLUDE_PATH cairo.h
      ${_CAIRO_INCLUDEDIR}
      /opt/gnome/include/cairo
      /usr/include
      /usr/include/cairo
      /opt/local/include/cairo
      $ENV{DEVLIBS_PATH}//include//
    )
    #MESSAGE(STATUS "DEBUG: GTK2_CAIRO_INCLUDE_PATH = ${GTK2_CAIRO_INCLUDE_PATH}")

    if (GTK2_CMAKE_24)
      pkgconfig(atk _ATK_INCLUDEDIR _ATKinkDir _ATK_LDFLAGS _ATK_CFLAGS)
    else (GTK2_CMAKE_24)
      if (PKG_CONFIG_FOUND)
        pkg_check_modules(_ATK atk)
      endif (PKG_CONFIG_FOUND)
    endif (GTK2_CMAKE_24)

    FIND_PATH(GTK2_ATK_INCLUDE_PATH atk/atk.h
      ${_ATK_INCLUDEDIR}
      /opt/gnome/include/atk-1.0
      /usr/include/atk-1.0
      /opt/local/include/atk-1.0
      $ENV{DEVLIBS_PATH}//include//
    )
    #MESSAGE(STATUS "DEBUG: GTK2_ATK_INCLUDE_PATH = ${GTK2_ATK_INCLUDE_PATH}")

    FIND_LIBRARY(GTK2_GTKGL_LIBRARY
      NAMES
        gtkgl
      PATHS
        ${_GTK2_INCLUDEDIR}
        /usr/lib
        /usr/local/lib
        /usr/openwin/lib
        /usr/X11R6/lib
        /opt/gnome/lib
    )

    FIND_LIBRARY(GTK2_GTK_LIBRARY
      NAMES
        gtk-x11-2.0
      PATHS
        ${_GTK2_LIBDIR}
        /usr/lib
        /usr/local/lib
        /usr/openwin/lib
        /usr/X11R6/lib
        /opt/gnome/lib
    )

    FIND_LIBRARY(GTK2_GDK_LIBRARY
      NAMES
        gdk-x11-2.0
      PATHS
        ${_GDK2_LIBDIR}
        /usr/lib
        /usr/local/lib
        /usr/openwin/lib
        /usr/X11R6/lib
        /opt/gnome/lib
    )

    FIND_LIBRARY(GTK2_GMODULE_LIBRARY
      NAMES
        gmodule-2.0
      PATHS
        ${_GMODULE2_LIBDIR}
        /usr/lib
        /usr/local/lib
        /usr/openwin/lib
        /usr/X11R6/lib
        /opt/gnome/lib
    )

    FIND_LIBRARY(GTK2_GLIB_LIBRARY
      NAMES
        glib-2.0
      PATHS
        ${_GLIB2_LIBDIR}
        /usr/lib
        /usr/local/lib
        /usr/openwin/lib
        /usr/X11R6/lib
        /opt/gnome/lib
    )

    FIND_LIBRARY(GTK2_Xi_LIBRARY 
      NAMES
        Xi
      PATHS 
        /usr/lib
        /usr/local/lib
        /usr/openwin/lib
        /usr/X11R6/lib
        /opt/gnome/lib
    )

    FIND_LIBRARY(GTK2_GTHREAD_LIBRARY
      NAMES
        gthread-2.0
      PATHS
        /usr/lib
        /usr/local/lib
        /usr/openwin/lib
        /usr/X11R6/lib
        /opt/gnome/lib
    )


    FIND_LIBRARY(GTK2_GOBJECT_LIBRARY
      NAMES
        gobject-2.0
      PATHS
        /usr/lib
        /usr/local/lib
        /usr/openwin/lib
        /usr/X11R6/lib
        /opt/gnome/lib
    )

    IF(GTK2_GTK_INCLUDE_PATH)
      IF(GTK2_GLIBCONFIG_INCLUDE_PATH)
        IF(GTK2_GLIB_INCLUDE_PATH)
          IF(GTK2_GTK_LIBRARY)
            IF(GTK2_GLIB_LIBRARY)
              IF(GTK2_PANGO_INCLUDE_PATH)
                IF(GTK2_ATK_INCLUDE_PATH)
                  IF(GTK2_CAIRO_INCLUDE_PATH)
                    # Assume that if gtk and glib were found, the other
                    # supporting libraries have also been found.

                    SET(GTK2_FOUND TRUE)

                    SET(GTK2_INCLUDE_DIRS
                      ${GTK2_GTK_INCLUDE_PATH}
                      ${GTK2_GLIBCONFIG_INCLUDE_PATH}
                      ${GTK2_GLIB_INCLUDE_PATH}
                      ${GTK2_PANGO_INCLUDE_PATH}
                      ${GTK2_GDKCONFIG_INCLUDE_PATH}
                      ${GTK2_ATK_INCLUDE_PATH}
                      ${GTK2_CAIRO_INCLUDE_PATH}
                    )

                    SET(GTK2_LIBRARIES
                      ${GTK2_GTK_LIBRARY}
                      ${GTK2_GDK_LIBRARY}
                      ${GTK2_GLIB_LIBRARY}
                    )
                    #${GTK2_GOBJECT_LIBRARY})

                    IF(GTK2_GMODULE_LIBRARY)
                      SET(GTK2_LIBRARIES
                        ${GTK2_LIBRARIES}
                        ${GTK2_GMODULE_LIBRARY}
                      )
                    ENDIF(GTK2_GMODULE_LIBRARY)

                    IF(GTK2_GTHREAD_LIBRARY)
                      SET(GTK2_LIBRARIES
                        ${GTK2_LIBRARIES}
                        ${GTK2_GTHREAD_LIBRARY}
                      )
                    SET(GTK2_LIBRARIES ${GTK2_LIBRARIES})
                    ENDIF(GTK2_GTHREAD_LIBRARY)
                  ELSE(GTK2_CAIRO_INCLUDE_PATH)
                    MESSAGE(STATUS "Can not find cairo")
                  ENDIF(GTK2_CAIRO_INCLUDE_PATH)
                ELSE(GTK2_ATK_INCLUDE_PATH)
                  MESSAGE(STATUS "Can not find atk")
                ENDIF(GTK2_ATK_INCLUDE_PATH)
              ELSE(GTK2_PANGO_INCLUDE_PATH)
                MESSAGE(STATUS "Can not find pango includes")
              ENDIF(GTK2_PANGO_INCLUDE_PATH)
            ELSE(GTK2_GLIB_LIBRARY)
              MESSAGE(STATUS "Can not find glib lib")
            ENDIF(GTK2_GLIB_LIBRARY)
          ELSE(GTK2_GTK_LIBRARY)
            MESSAGE(STATUS "Can not find gtk lib")
          ENDIF(GTK2_GTK_LIBRARY)
        ELSE(GTK2_GLIB_INCLUDE_PATH)
          MESSAGE(STATUS "Can not find glib includes")
        ENDIF(GTK2_GLIB_INCLUDE_PATH)
      ELSE(GTK2_GLIBCONFIG_INCLUDE_PATH)
        MESSAGE(STATUS "Can not find glibconfig")
      ENDIF(GTK2_GLIBCONFIG_INCLUDE_PATH)
    ELSE (GTK2_GTK_INCLUDE_PATH)
      MESSAGE(STATUS "Can not find gtk includes")
    ENDIF (GTK2_GTK_INCLUDE_PATH)

    IF (GTK2_FOUND)
      IF (NOT GTK2_FIND_QUIETLY)
        MESSAGE(STATUS "Found GTK2: ${GTK2_LIBRARIES}")
      ENDIF (NOT GTK2_FIND_QUIETLY)
    ELSE (GTK2_FOUND)
      IF (GTK2_FIND_REQUIRED)
        MESSAGE(SEND_ERROR "Could NOT find GTK2")
      ENDIF (GTK2_FIND_REQUIRED)
    ENDIF (GTK2_FOUND)

    MARK_AS_ADVANCED(
      GTK2_GDK_LIBRARY
      GTK2_GLIB_INCLUDE_PATH
      GTK2_GLIB_LIBRARY
      GTK2_GLIBCONFIG_INCLUDE_PATH
      GTK2_GMODULE_LIBRARY
      GTK2_GTHREAD_LIBRARY
      GTK2_Xi_LIBRARY
      GTK2_GTK_INCLUDE_PATH
      GTK2_GTK_LIBRARY
      GTK2_GTKGL_INCLUDE_PATH
      GTK2_GTKGL_LIBRARY
      GTK2_ATK_INCLUDE_PATH
      GTK2_GDKCONFIG_INCLUDE_PATH
      #GTK2_GOBJECT_LIBRARY
      GTK2_PANGO_INCLUDE_PATH
    )
ENDIF (GTK2_LIBRARIES AND GTK2_INCLUDE_DIRS)

# vim:et ts=2 sw=2 comments=\:\#
