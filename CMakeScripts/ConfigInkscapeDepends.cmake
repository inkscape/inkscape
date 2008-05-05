SET(INKSCAPE_DEPENDS
# the commented out below are required but pulled in with gtk+-2.0 & gtkmm-2.4
# with some duplication that we can only remove if lookup each sub-lib individually
#    cairo
#    cairomm-1.0
#    glib-2.0
#    glibmm-2.4
    gtk+-2.0
    gtkmm-2.4
    sigc++-2.0
    lcms
#    libgc1c2
    libpng
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
#        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${${dep_name}_CFLAGS}")
        SET(INKSCAPE_LINK_FLAGS "${INKSCAPE_LINK_FLAGS} ${${dep_name}_LINK_FLAGS}")
    ELSE("${dep}_FOUND")
        message(STATUS "${dep}: NOT FOUND")
    ENDIF("${dep}_FOUND")
ENDFOREACH(dep)

# Include non pkg-config dependencies:
INCLUDE(FindBoost)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${BOOST_INCLUDE_DIR} ")
INCLUDE(FindGC)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${LIBGC_INCLUDE_DIR} ")
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
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${${opt_name}_CFLAGS}")
#        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${${opt_name}_CFLAGS}")
        SET(INKSCAPE_LINK_FLAGS "${INKSCAPE_LINK_FLAGS} ${${opt_name}_LINK_FLAGS}")
    ELSE("${opt}_FOUND")
        message(STATUS "${opt}: NOT FOUND")
    ENDIF("${opt}_FOUND")
ENDFOREACH(opt)
# end Optional Dependencies
message(STATUS "")
message(STATUS "")

SET(CMAKE_MAKE_PROGRAM "${CMAKE_MAKE_PROGRAM} -j2")

INCLUDE(ConfigCompileFlags)
INCLUDE(ConfigChecks)

