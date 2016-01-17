# A macro to replace slashes and spaces in a string with underscores
macro(SANITIZE_PATH _string_var)
    string(REGEX REPLACE "[\\/ ]+" "_" ${_string_var} ${${_string_var}})
endmacro()


macro(inkscape_source_group
	sources)

    # Group by location on disk
    source_group("Source Files" FILES CMakeLists.txt)

    foreach(_SRC ${sources})
	get_filename_component(_SRC_EXT ${_SRC} EXT)
	if((${_SRC_EXT} MATCHES ".h") OR (${_SRC_EXT} MATCHES ".hpp"))
	    source_group("Header Files" FILES ${_SRC})
	else()
	    source_group("Source Files" FILES ${_SRC})
	endif()
    endforeach()

    unset(_SRC)
    unset(_SRC_EXT)
endmacro()


# only MSVC uses SOURCE_GROUP
macro(add_inkscape_lib
	name
	sources)

    add_library(${name} ${sources})

    # works fine without having the includes
    # listed is helpful for IDE's (QtCreator/MSVC)
    inkscape_source_group("${sources}")

endmacro()


# A macro to append to the global source property
set_property(GLOBAL PROPERTY inkscape_global_SRC "")

macro (add_inkscape_source
	sources)

    foreach(_SRC ${ARGV})
	get_filename_component(_ABS_SRC ${_SRC} ABSOLUTE)
	set_property(GLOBAL APPEND PROPERTY inkscape_global_SRC ${_ABS_SRC})
    endforeach()
    unset(_SRC)
    unset(_ABS_SRC)
endmacro()

# A macro to append to the global source property
macro (add_inkscape_library
	sources)

    foreach(_SRC ${ARGV})
	get_filename_component(_ABS_SRC ${_SRC} ABSOLUTE)
	set_property(GLOBAL APPEND PROPERTY inkscape_global_SRC ${_ABS_SRC})
    endforeach()
    unset(_SRC)
    unset(_ABS_SRC)
endmacro()

macro(INKSCAPE_PKG_CONFIG_FIND PREFIX MODNAME VERSION PATH_NAME PATH_SUFFIXE LIB_NAME)
    if(VERSION)
	pkg_check_modules(_${PREFIX} ${MODNAME}${VERSION})
    else(VERSION)
	pkg_check_modules(_${PREFIX} ${MODNAME})
    endif(VERSION)

    find_path(${PREFIX}_INCLUDE_DIR
	NAMES
	    ${PATH_NAME}
	PATHS
	    ${_${PREFIX}_INCLUDEDIR}
	    /usr/include
	    /usr/local/include
	    /opt/local/include
	    /sw/include
	    $ENV{DEVLIBS_PATH}//include//
	PATH_SUFFIXES
	    ${PATH_SUFFIXE}
	)

    find_library(${PREFIX}_LIBRARY
	NAMES
	    ${LIB_NAME}
	PATHS
	    ${_${PREFIX}_LIBDIR}
	    /usr/lib
	    /usr/local/lib
	    /opt/local/lib
	    /sw/lib
	)

    if (${PREFIX}_LIBRARY)
	set(${PREFIX}_FOUND TRUE)
	set(${PREFIX}_VERSION ${_${PREFIX}_VERSION})
    endif (${PREFIX}_LIBRARY)

    set(${PREFIX}_INCLUDE_DIRS
	${${PREFIX}_INCLUDE_DIR}
	)

    if (${PREFIX}_FOUND)
	set(${PREFIX}_LIBRARIES
	    ${${PREFIX}_LIBRARIES}
	    ${${PREFIX}_LIBRARY}
	    )
    endif (${PREFIX}_FOUND)

    if (${PREFIX}_INCLUDE_DIRS AND ${PREFIX}_LIBRARIES)
	set(${PREFIX}_FOUND TRUE)
    endif (${PREFIX}_INCLUDE_DIRS AND ${PREFIX}_LIBRARIES)

    if (${PREFIX}_FOUND)
	if (NOT ${PREFIX}_FIND_QUIETLY)
	    message(STATUS "Found ${MODNAME}: ${${PREFIX}_LIBRARIES}")
	endif (NOT ${PREFIX}_FIND_QUIETLY)
    else (${PREFIX}_FOUND)
	if (${PREFIX}_FIND_REQUIRED)
	    message(FATAL_ERROR "Could not find ${MODNAME}")
	endif (${PREFIX}_FIND_REQUIRED)
    endif (${PREFIX}_FOUND)

    # show the <PREFIX>_INCLUDE_DIRS and <PREFIX>_LIBRARIES variables only in the advanced view
    mark_as_advanced(${PREFIX}_INCLUDE_DIRS ${PREFIX}_LIBRARIES)
endmacro()
