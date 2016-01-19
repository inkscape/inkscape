# - This is a module to Generate files using Glib-Marshal
# Both the header and source files (.h and .cpp)
# Copyright 2008 - Joshua L. Blocher
#
# And it defines the following variables:
#  GLIB_MARSHAL_PREFIX - The name of the files
#  GLIB_MARSHAL_FILE - File to Generate from (.list) and to generate to (.h and .cpp)
#  GLIB_MARSHAL_OUTPUT_HEADER - Result of Generation
#  GLIB_MARSHAL_OUTPUT_CPP - Result of Generation
#  GLIB_MARSHAL_OUTPUT_LOCATION - Where we are putting the Output

find_program(GLIB_MARSHAL_EXECUTABLE NAMES glib-genmarshal PATHS /usr/local/bin )

macro(GLIB_MARSHAL GLIB_MARSHAL_PREFIX GLIB_MARSHAL_FILE GLIB_MARSHAL_OUTPUT_LOCATION)
    if(GLIB_MARSHAL_EXECUTABLE)
        set(GLIB_MARSHAL_OUTPUT_EXTRA_LINE "#include \"${GLIB_MARSHAL_FILE}.h\" \n" )

        message(STATUS "Generating header and sourcefiles from ${GLIB_MARSHAL_FILE}.list (Glib-Marshal)")
        execute_process(COMMAND ${GLIB_MARSHAL_EXECUTABLE} --prefix=${GLIB_MARSHAL_PREFIX} --header ${CMAKE_CURRENT_SOURCE_DIR}/${GLIB_MARSHAL_FILE}.list
            OUTPUT_VARIABLE GLIB_MARSHAL_OUTPUT_HEADER )
        execute_process(COMMAND ${GLIB_MARSHAL_EXECUTABLE} --prefix=${GLIB_MARSHAL_PREFIX} --body ${CMAKE_CURRENT_SOURCE_DIR}/${GLIB_MARSHAL_FILE}.list
            OUTPUT_VARIABLE GLIB_MARSHAL_OUTPUT_CPP )

        # check whether the generated file is the same as the existing one
	if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/${GLIB_MARSHAL_FILE}.h)
	    file(READ ${CMAKE_CURRENT_BINARY_DIR}/${GLIB_MARSHAL_FILE}.h GLIB_MARSHAL_HEADER_OLD)
	else()
	    set(GLIB_MARSHAL_HEADER_OLD "")
	endif()
        if(NOT GLIB_MARSHAL_HEADER_OLD STREQUAL GLIB_MARSHAL_OUTPUT_HEADER)
            message(STATUS "${GLIB_MARSHAL_FILE}.h changed; overwriting")
            file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${GLIB_MARSHAL_FILE}.h "${GLIB_MARSHAL_OUTPUT_HEADER}")
            file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${GLIB_MARSHAL_FILE}.cpp "${GLIB_MARSHAL_OUTPUT_EXTRA_LINE}")
            file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/${GLIB_MARSHAL_FILE}.cpp "${GLIB_MARSHAL_OUTPUT_CPP}")
        else()
            message(STATUS "${GLIB_MARSHAL_FILE}.h unchanged")
        endif()
    endif()
endmacro()

