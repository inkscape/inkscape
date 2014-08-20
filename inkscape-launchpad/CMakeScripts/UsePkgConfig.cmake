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

FIND_PROGRAM(PKGCONFIG_EXECUTABLE NAMES pkg-config PATHS /usr/local/bin )

MACRO(STRIP_NEWLINES _string_var)
  STRING(REGEX REPLACE "[\n\r]+" "" ${_string_var} ${${_string_var}})
ENDMACRO(STRIP_NEWLINES _string_var)

MACRO(PKGCONFIG_FOUND _package _found)
  # reset the variable at the beginning
  SET(${_found})

# if pkg-config has been found
  IF(PKGCONFIG_EXECUTABLE)

    EXEC_PROGRAM(${PKGCONFIG_EXECUTABLE} ARGS ${_package} --print-errors --exists RETURN_VALUE _return_VALUE OUTPUT_VARIABLE _pkgconfigDevNull )
    
    IF(${_pkgconfigDevNull})
      MESSAGE(STATUS "${_pkgconfigDevNull}")
    ENDIF(${_pkgconfigDevNull})
    
    IF(NOT _return_VALUE)
      SET(${_found} "TRUE")
    ENDIF(NOT _return_VALUE)
  ENDIF(PKGCONFIG_EXECUTABLE)

ENDMACRO(PKGCONFIG_FOUND _found)

MACRO(PKGCONFIG _package _include_DIR _link_DIR _link_FLAGS _cflags)
# reset the variables at the beginning
  SET(${_include_DIR})
  SET(${_link_DIR})
  SET(${_link_FLAGS})
  SET(${_cflags})

  # if pkg-config has been found
  IF(PKGCONFIG_EXECUTABLE)

    EXEC_PROGRAM(${PKGCONFIG_EXECUTABLE} ARGS ${_package} --exists RETURN_VALUE _return_VALUE OUTPUT_VARIABLE _pkgconfigDevNull )

    # and if the package of interest also exists for pkg-config, then get the information
    IF(NOT _return_VALUE)

      EXEC_PROGRAM(${PKGCONFIG_EXECUTABLE} ARGS ${_package} --variable=includedir OUTPUT_VARIABLE ${_include_DIR} )

      EXEC_PROGRAM(${PKGCONFIG_EXECUTABLE} ARGS ${_package} --variable=libdir OUTPUT_VARIABLE ${_link_DIR} )

      EXEC_PROGRAM(${PKGCONFIG_EXECUTABLE} ARGS ${_package} --libs OUTPUT_VARIABLE ${_link_FLAGS} )

      EXEC_PROGRAM(${PKGCONFIG_EXECUTABLE} ARGS ${_package} --cflags OUTPUT_VARIABLE ${_cflags} )

      STRIP_NEWLINES(${_cflags})

    ENDIF(NOT _return_VALUE)

  ENDIF(PKGCONFIG_EXECUTABLE)

ENDMACRO(PKGCONFIG _include_DIR _link_DIR _link_FLAGS _cflags)

MACRO(PKGCONFIG_VERSION _package _version)
# reset the variables at the beginning
  SET(${_version})

# if pkg-config has been found
  IF(PKGCONFIG_EXECUTABLE)
    EXEC_PROGRAM(${PKGCONFIG_EXECUTABLE} ARGS --print-errors ${_package} --exists RETURN_VALUE _return_VALUE OUTPUT_VARIABLE _pkgconfigDevNull )

# and if the package of interest also exists for pkg-config, then get the information
    IF(NOT _return_VALUE)
      EXEC_PROGRAM(${PKGCONFIG_EXECUTABLE} ARGS --print-errors ${_package} --modversion OUTPUT_VARIABLE ${_version} )
    ENDIF(NOT _return_VALUE)

  ENDIF(PKGCONFIG_EXECUTABLE)

ENDMACRO(PKGCONFIG_VERSION _package _version)

MARK_AS_ADVANCED(PKGCONFIG_EXECUTABLE)

MACRO(PKGCONFIG_DEFINITION _package _definition)
# reset the variables at the beginning
  SET(${_definition})

# if pkg-config has been found
  IF(PKGCONFIG_EXECUTABLE)
    EXEC_PROGRAM(${PKGCONFIG_EXECUTABLE} ARGS --print-errors ${_package} --exists RETURN_VALUE _return_VALUE OUTPUT_VARIABLE _pkgconfigDevNull )

# and if the package of interest also exists for pkg-config, then get the information
    IF(NOT _return_VALUE)
      EXEC_PROGRAM(${PKGCONFIG_EXECUTABLE} ARGS --print-errors ${_package} --cflags-only-other OUTPUT_VARIABLE ${_definition} )
    ENDIF(NOT _return_VALUE)

  ENDIF(PKGCONFIG_EXECUTABLE)

ENDMACRO(PKGCONFIG_DEFINITION _package _definition)

MARK_AS_ADVANCED(PKGCONFIG_EXECUTABLE)
