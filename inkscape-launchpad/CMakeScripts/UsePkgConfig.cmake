# - pkg-config module for CMake
#
# Defines the following macros:
#
#  PKGCONFIG_FOUND(package found)
#  PKGCONFIG(package includedir libdir linkflags cflags)
#  PKGCONFIG_VERSION(package version)
#  PKGCONFIG_DEFINITION(package definition)

# Calling PKGCONFIG_FOUND will fill into the argument the value of the package search's result
# e.g. PKGCONFIG_FOUND(libart-2.0 LIBART_FOUND)
#
# Calling PKGCONFIG_VERSION will fill the desired version into the argument,
# e.g. PKGCONFIG_VERSION(libart-2.0 LIBART_VERSION)
# Calling PKGCONFIG will fill the desired information into the 4 given arguments,
# e.g. PKGCONFIG(libart-2.0 LIBART_INCLUDE_DIR LIBART_LINK_DIR LIBART_LINK_FLAGS LIBART_CFLAGS)
# if pkg-config was NOT found or the specified software package doesn't exist, the
# variable will be empty when the function returns, otherwise they will contain the respective information
#
# Calling PKGCONFIG_VERSION will fill the desired version into the argument,
# e.g. PKGCONFIG_VERSION(libart-2.0 LIBART_VERSION)
#
# Calling PKGCONFIG_DEFINITION will fill the definition (e.g -D_REENTRANT) into the argument,
# e.g. PKGCONFIG_DEFINITION(libart-2.0 LIBART_DEFINITION)

find_program(PKGCONFIG_EXECUTABLE NAMES pkg-config PATHS /usr/local/bin )

macro(STRIP_NEWLINES _string_var)
    string(REGEX REPLACE "[\n\r]+" "" ${_string_var} ${${_string_var}})
endmacro(STRIP_NEWLINES _string_var)

macro(PKGCONFIG_FOUND _package _found)
    # reset the variable at the beginning
    set(${_found})

    # if pkg-config has been found
    if(PKGCONFIG_EXECUTABLE)
	exec_program(${PKGCONFIG_EXECUTABLE} ARGS ${_package} --print-errors --exists RETURN_VALUE _return_VALUE OUTPUT_VARIABLE _pkgconfigDevNull )
	if(${_pkgconfigDevNull})
	    message(STATUS "${_pkgconfigDevNull}")
	endif(${_pkgconfigDevNull})
    
	if(NOT _return_VALUE)
	    set(${_found} "TRUE")
	endif(NOT _return_VALUE)
    endif(PKGCONFIG_EXECUTABLE)
endmacro(PKGCONFIG_FOUND _found)

macro(PKGCONFIG _package _include_DIR _link_DIR _link_FLAGS _cflags)
    # reset the variables at the beginning
    set(${_include_DIR})
    set(${_link_DIR})
    set(${_link_FLAGS})
    set(${_cflags})

    # if pkg-config has been found
    if(PKGCONFIG_EXECUTABLE)
	exec_program(${PKGCONFIG_EXECUTABLE} ARGS ${_package} --exists RETURN_VALUE _return_VALUE OUTPUT_VARIABLE _pkgconfigDevNull )

    # and if the package of interest also exists for pkg-config, then get the information
    if(NOT _return_VALUE)
      exec_program(${PKGCONFIG_EXECUTABLE} ARGS ${_package} --variable=includedir OUTPUT_VARIABLE ${_include_DIR} )
      exec_program(${PKGCONFIG_EXECUTABLE} ARGS ${_package} --variable=libdir OUTPUT_VARIABLE ${_link_DIR} )
      exec_program(${PKGCONFIG_EXECUTABLE} ARGS ${_package} --libs OUTPUT_VARIABLE ${_link_FLAGS} )
      exec_program(${PKGCONFIG_EXECUTABLE} ARGS ${_package} --cflags OUTPUT_VARIABLE ${_cflags} )
      strip_newlines(${_cflags})
    endif(NOT _return_VALUE)
  endif(PKGCONFIG_EXECUTABLE)
endmacro(PKGCONFIG _include_DIR _link_DIR _link_FLAGS _cflags)

macro(PKGCONFIG_VERSION _package _version)
    # reset the variables at the beginning
    set(${_version})

    # if pkg-config has been found
    if(PKGCONFIG_EXECUTABLE)
	exec_program(${PKGCONFIG_EXECUTABLE} ARGS --print-errors ${_package} --exists RETURN_VALUE _return_VALUE OUTPUT_VARIABLE _pkgconfigDevNull )

	# and if the package of interest also exists for pkg-config, then get the information
	if(NOT _return_VALUE)
	    exec_program(${PKGCONFIG_EXECUTABLE} ARGS --print-errors ${_package} --modversion OUTPUT_VARIABLE ${_version} )
	endif(NOT _return_VALUE)
    endif(PKGCONFIG_EXECUTABLE)
endmacro(PKGCONFIG_VERSION _package _version)

mark_as_advanced(PKGCONFIG_EXECUTABLE)

macro(PKGCONFIG_DEFINITION _package _definition)
    # reset the variables at the beginning
    set(${_definition})

    # if pkg-config has been found
    if(PKGCONFIG_EXECUTABLE)
	exec_program(${PKGCONFIG_EXECUTABLE} ARGS --print-errors ${_package} --exists RETURN_VALUE _return_VALUE OUTPUT_VARIABLE _pkgconfigDevNull )

	# and if the package of interest also exists for pkg-config, then get the information
	if(NOT _return_VALUE)
	    exec_program(${PKGCONFIG_EXECUTABLE} ARGS --print-errors ${_package} --cflags-only-other OUTPUT_VARIABLE ${_definition} )
	endif(NOT _return_VALUE)
    endif(PKGCONFIG_EXECUTABLE)
endmacro(PKGCONFIG_DEFINITION _package _definition)

mark_as_advanced(PKGCONFIG_EXECUTABLE)
