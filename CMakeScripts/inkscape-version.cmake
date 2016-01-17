# This is called by cmake as an extermal process from
# ./src/CMakeLists.txt and creates inkscape-version.cpp
#
# These variables are defined by the caller, matching the CMake equivilents.
# - ${INKSCAPE_SOURCE_DIR}
# - ${INKSCAPE_BINARY_DIR}

# We should extract the version from build.xml
# but for now just hard code
set(INKSCAPE_REVISION "unknown")

if(EXISTS ${INKSCAPE_SOURCE_DIR}/.bzr/)
    execute_process(COMMAND
	bzr revno --tree ${INKSCAPE_SOURCE_DIR}
	OUTPUT_VARIABLE INKSCAPE_REVISION
	OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

file(WRITE
    ${INKSCAPE_BINARY_DIR}/src/inkscape-version.cpp.txt
    # unlike autoconf, include config.h
    "#ifdef HAVE_CONFIG_H\n"
    "# include <config.h>\n"
    "#endif\n"
    "\n"
    "namespace Inkscape {\n"
    "    char const *version_string = VERSION \" \" \"${INKSCAPE_REVISION}\";\n"
    "}\n")

# Copy the file to the final header only if the version changes
# and avoid needless rebuilds
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${INKSCAPE_BINARY_DIR}/src/inkscape-version.cpp.txt
    ${INKSCAPE_BINARY_DIR}/src/inkscape-version.cpp)
