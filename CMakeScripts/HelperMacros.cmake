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
