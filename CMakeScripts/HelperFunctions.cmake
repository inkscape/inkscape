# pkg_check_variable() - a function to retrieve pkg-config variables in CMake
#
# source: http://bloerg.net/2015/03/06/pkg-config-variables-in-cmake.html

find_package(PkgConfig REQUIRED)

function(pkg_check_variable _pkg _name)
    string(TOUPPER ${_pkg} _pkg_upper)
    string(TOUPPER ${_name} _name_upper)
    string(REPLACE "-" "_" _pkg_upper ${_pkg_upper})
    string(REPLACE "-" "_" _name_upper ${_name_upper})
    set(_output_name "${_pkg_upper}_${_name_upper}")

    execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=${_name} ${_pkg}
        OUTPUT_VARIABLE _pkg_result
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    set("${_output_name}" "${_pkg_result}" CACHE STRING "pkg-config variable ${_name} of ${_pkg}")
endfunction()

# Join a cmake list of strings with a given glue character/string
# E.g. join(MY_RESULT, ",", "1; 2; 3;") returns "1, 2, 3"
function(join OUTPUT GLUE)
    set(_TMP_RESULT "")
    set(_GLUE "") # effective glue is empty at the beginning
    foreach(arg ${ARGN})
	# Skip empty lines
	if(NOT arg STREQUAL "\n")
	    set(_TMP_RESULT "${_TMP_RESULT}${_GLUE}${arg}")
	    set(_GLUE "${GLUE}")
	endif()
    endforeach()
    set(${OUTPUT} "${_TMP_RESULT}" PARENT_SCOPE)
endfunction()
