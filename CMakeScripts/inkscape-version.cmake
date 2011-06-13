# This is called by cmake as an extermal process from
# ./src/CMakeLists.txt and creates inkscape-version.cpp
#
# These variables are defined by the caller, matching the CMake equivilents.
# - ${INKSCAPE_SOURCE_DIR}
# - ${INKSCAPE_BINARY_DIR}

# We should extract the version from build.xml
# but for now just hard code
set(INKSCAPE_VERSION "0.48+devel")
set(INKSCAPE_REVISION "unknown")

if(EXISTS ${INKSCAPE_SOURCE_DIR}/.bzr/)
	execute_process(COMMAND
			bzr revno ${INKSCAPE_SOURCE_DIR}
			OUTPUT_VARIABLE INKSCAPE_REVISION
			OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

file(WRITE
		${INKSCAPE_BINARY_DIR}/src/inkscape-version.cpp.txt
		"namespace Inkscape {\n"
		"    char const *version_string = \"${INKSCAPE_VERSION} ${INKSCAPE_REVISION}\";\n"
		"}\n")

# Copy the file to the final header only if the version changes
# and avoid needless rebuilds
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        ${INKSCAPE_BINARY_DIR}/src/inkscape-version.cpp.txt
						${INKSCAPE_BINARY_DIR}/src/inkscape-version.cpp)
