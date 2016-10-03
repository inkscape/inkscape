# - Try to find the YAML library
# Once done this will define
#
#  YAML_FOUND - system has yaml
#  YAML_INCLUDE_DIR - the yaml include directory
#  YAML_LIBRARIES - the libraries needed to use yaml

# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (YAML_INCLUDE_DIR AND YAML_LIBRARIES)
    # in cache already
    SET(YAML_FOUND TRUE)
else (YAML_INCLUDE_DIR AND YAML_LIBRARIES)
    IF (NOT WIN32)
		FIND_PACKAGE(PkgConfig)
        IF (PKG_CONFIG_FOUND)
            # use pkg-config to get the directories and then use these values
            # in the FIND_PATH() and FIND_LIBRARY() calls
            pkg_check_modules(_YAML_PC QUIET yaml-1)
        ENDIF (PKG_CONFIG_FOUND)
    ENDIF (NOT WIN32)

    FIND_PATH(YAML_INCLUDE_DIR yaml.h
        /usr/include
        /usr/local/include
    )

    FIND_LIBRARY(YAML_LIBRARIES NAMES yaml
        PATHS)

    if (YAML_INCLUDE_DIR AND YAML_LIBRARIES)
        set(YAML_FOUND TRUE)
    endif (YAML_INCLUDE_DIR AND YAML_LIBRARIES)


    if (YAML_FOUND)
        if (NOT YAML_FIND_QUIETLY)
            message(STATUS "Found YAML: ${YAML_LIBRARIES}")
        endif (NOT YAML_FIND_QUIETLY)
    else (YAML_FOUND)
        if (YAML_FIND_REQUIRED)
            message(FATAL_ERROR "Could NOT find YAML")
        endif (YAML_FIND_REQUIRED)
    endif (YAML_FOUND)

    MARK_AS_ADVANCED(YAML_INCLUDE_DIR YAML_LIBRARIES)

endif (YAML_INCLUDE_DIR AND YAML_LIBRARIES)
