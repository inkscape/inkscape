SET(INKSCAPE_DEPENDS
    gtk+-2.0
    gtkmm-2.4
    cairo
    cairomm-1.0
    sigc++-2.0
    lcms
    libgc1c2
    libpng
    glib-2.0
    glibmm-2.4
    libxml-2.0
    libxslt
    ImageMagick++
    libpopt
    freetype2)
SET(INKSCAPE_OPTIONAL
    gnome-vfs-2.0
    libwpg-0.1
    libssl)

include(UsePkgConfig)

# Dependencies Packages
message(STATUS "")
message(STATUS "")
message(STATUS "Checking For REQUIRED Libraries for Building Inkscape.")
SET(INKSCAPE_LINK_FLAGS "")
FOREACH(dep ${INKSCAPE_DEPENDS})
    # This is a hack due to a bug in Cmake vars system, Uncomment if using a version older than 2.4 //verbalshadow
#    IF("${dep}" MATCHES "gtk\\+-2.0")
#        SET(dep_name "GTK2")
#    ELSE("${dep}" MATCHES "gtk\\+-2.0")
       SET(dep_name "${dep}")
#    ENDIF("${dep}" MATCHES "gtk\\+-2.0")

    PKGCONFIG_FOUND(${dep} "${dep}_FOUND")
    PKGCONFIG(${dep} "${dep_name}_INCLUDE_DIR" "${dep_name}_LINK_DIR" "${dep_name}_LINK_FLAGS" "${dep_name}_CFLAGS")
#    PKGCONFIG_VERSION(${dep} "${dep}_VERSION")
    IF("${dep}_FOUND")
        message(STATUS "${dep}: FOUND")
        # Set Compiler Flags
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${${dep_name}_CFLAGS}")
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${${dep_name}_CFLAGS}")
        SET(INKSCAPE_LINK_FLAGS "${INKSCAPE_LINK_FLAGS} ${${dep_name}_LINK_FLAGS}")
    ELSE("${dep}_FOUND")
        message(STATUS "${dep}: NOT FOUND")
    ENDIF("${dep}_FOUND")
ENDFOREACH(dep)
# Include non pkg-config dependencies:
INCLUDE(FindBoost)
INCLUDE(FindGC)
INCLUDE(IncludeJava)
# end Dependencies

# Optional Dependencies Packages
message(STATUS "")
message(STATUS "")
message(STATUS "Checking For OPTIONAL Libraries for Building Inkscape.")
message(STATUS "These add additional functionality to Inkscape.")
FOREACH(opt ${INKSCAPE_OPTIONAL})
    SET(opt_name "${opt}")
    PKGCONFIG_FOUND(${opt} "${opt}_FOUND")
    PKGCONFIG(${opt} "${opt_name}_INCLUDE_DIR" "${opt_name}_LINK_DIR" "${opt_name}_LINK_FLAGS" "${opt_name}_CFLAGS")
#    PKGCONFIG_VERSION(${opt} "${opt}_VERSION")
    IF("${opt}_FOUND")
        message(STATUS "${opt}: FOUND")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${${dep_name}_CFLAGS}")
        SET(INKSCAPE_LINK_FLAGS "${INKSCAPE_LINK_FLAGS} ${${dep_name}_LINK_FLAGS}")
    ELSE("${opt}_FOUND")
        message(STATUS "${opt}: NOT FOUND")
    ENDIF("${opt}_FOUND")
ENDFOREACH(opt)
# end Optional Dependencies
message(STATUS "")
message(STATUS "")

SET(CMAKE_MAKE_PROGRAM "${CMAKE_MAKE_PROGRAM} -j2")

#---------------
# From here on:
# Set all HAVE_XXX variables, to correctly set all defines in config.h

INCLUDE (CheckIncludeFiles)
INCLUDE (CheckFunctionExists)
INCLUDE (CheckStructMember)
# usage: CHECK_INCLUDE_FILES (<header> <RESULT_VARIABLE> )
# usage: CHECK_FUNCTION_EXISTS (<function name> <RESULT_VARIABLE> )
# usage: CHECK_STRUCT_MEMBER (<struct> <member> <header> <RESULT_VARIABLE>)

CHECK_INCLUDE_FILES (boost/concept_check.hpp HAVE_BOOST_CONCEPT_CHECK_HPP)
CHECK_INCLUDE_FILES (cairo-pdf.h HAVE_CAIRO_PDF)
CHECK_FUNCTION_EXISTS(floor HAVE_FLOOR)
CHECK_FUNCTION_EXISTS(fpsetmask HAVE_FPSETMASK)
CHECK_INCLUDE_FILES (gc/gc.h HAVE_GC_GC_H)
CHECK_INCLUDE_FILES (gc.h HAVE_GC_H)
CHECK_INCLUDE_FILES (getopt.h HAVE_GETOPT_H)
CHECK_FUNCTION_EXISTS(gettext HAVE_GETTEXT)
CHECK_FUNCTION_EXISTS(gettimeofday HAVE_GETTIMEOFDAY)
CHECK_FUNCTION_EXISTS(gtk_window_fullscreen HAVE_GTK_WINDOW_FULLSCREEN)
CHECK_FUNCTION_EXISTS(gtk_window_set_default_icon_from_file HAVE_GTK_WINDOW_SET_DEFAULT_ICON_FROM_FILE)
CHECK_INCLUDE_FILES (ieeefp.h HAVE_IEEEFP_H)
CHECK_INCLUDE_FILES (inttypes.h HAVE_INTTYPES_H)
CHECK_INCLUDE_FILES (locale.h HAVE_LC_MESSAGES)
CHECK_INCLUDE_FILES (locale.h HAVE_LOCALE_H)
CHECK_INCLUDE_FILES (libintl.h HAVE_LIBINTL_H)
CHECK_INCLUDE_FILES (fcntl.h HAVE_FCNTL_H)
CHECK_FUNCTION_EXISTS(mallinfo HAVE_MALLINFO)
CHECK_INCLUDE_FILES (malloc.h HAVE_MALLOC_H)
CHECK_FUNCTION_EXISTS(memmove HAVE_MEMMOVE)
CHECK_INCLUDE_FILES (memory.h HAVE_MEMORY_H)
CHECK_FUNCTION_EXISTS(memset HAVE_MEMSET)
CHECK_FUNCTION_EXISTS(mkdir HAVE_MKDIR)
CHECK_FUNCTION_EXISTS(pow HAVE_POW)
CHECK_FUNCTION_EXISTS(sqrt HAVE_SQRT)
CHECK_INCLUDE_FILES (stddef.h HAVE_STDDEF_H)
CHECK_INCLUDE_FILES (stdint.h HAVE_STDINT_H)
CHECK_INCLUDE_FILES (stdlib.h HAVE_STDLIB_H)
CHECK_INCLUDE_FILES (strings.h HAVE_STRINGS_H)
CHECK_INCLUDE_FILES (string.h HAVE_STRING_H)
CHECK_FUNCTION_EXISTS(strncasecmp HAVE_STRNCASECMP)
CHECK_FUNCTION_EXISTS(strpbrk HAVE_STRPBRK)
CHECK_FUNCTION_EXISTS(strrchr HAVE_STRRCHR)
CHECK_FUNCTION_EXISTS(strspn HAVE_STRSPN)
CHECK_FUNCTION_EXISTS(strstr HAVE_STRSTR)
CHECK_FUNCTION_EXISTS(strtoul HAVE_STRTOUL)
CHECK_STRUCT_MEMBER(fordblks mallinfo malloc.h HAVE_STRUCT_MALLINFO_FORDBLKS)
CHECK_STRUCT_MEMBER(fsmblks mallinfo malloc.h HAVE_STRUCT_MALLINFO_FSMBLKS)
CHECK_STRUCT_MEMBER(hblkhd mallinfo malloc.h HAVE_STRUCT_MALLINFO_HBLKHD)
CHECK_STRUCT_MEMBER(uordblks mallinfo malloc.h HAVE_STRUCT_MALLINFO_UORDBLKS)
CHECK_STRUCT_MEMBER(usmblks mallinfo malloc.h HAVE_STRUCT_MALLINFO_USMBLKS)
CHECK_INCLUDE_FILES (sys/filio.h HAVE_SYS_FILIO_H)
CHECK_INCLUDE_FILES (sys/stat.h HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILES (sys/time.h HAVE_SYS_TIME_H)
CHECK_INCLUDE_FILES (sys/types.h HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILES (unistd.h HAVE_UNISTD_H)
CHECK_INCLUDE_FILES (zlib.h HAVE_ZLIB_H)

# Enable pango defines, necessary for compilation on Win32, how about Linux?
IF (HAVE_CAIRO_PDF)
    SET(PANGO_ENABLE_ENGINE TRUE)
    SET(RENDER_WITH_PANGO_CAIRO TRUE)
ENDIF(HAVE_CAIRO_PDF)

# Create the two configuration files: config.h and inkscape_version.h
# Create them in the binary root dir
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/config.h.cmake ${CMAKE_BINARY_DIR}/config.h)
FILE(WRITE ${CMAKE_BINARY_DIR}/inkscape_version.h "#define INKSCAPE_VERSION \"${INKSCAPE_VERSION}\"\n")
INCLUDE_DIRECTORIES ("${CMAKE_BINARY_DIR}")  # Include base dir, so other files can refer to the generated files.
# CMAKE_INCLUDE_CURRENT_DIR is not enough as it only includes the current dir and not the basedir with config.h in it