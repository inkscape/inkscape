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
message("revision is " ${INKSCAPE_REVISION})

configure_file(${INKSCAPE_SOURCE_DIR}/src/inkscape-version.cpp.in ${INKSCAPE_BINARY_DIR}/src/inkscape-version.cpp)
