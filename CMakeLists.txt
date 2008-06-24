PROJECT(inkscape)

SET(INKSCAPE_VERSION 0.46+devel)
SET(PROJECT_NAME inkscape)
CMAKE_MINIMUM_REQUIRED(VERSION 2.4.6)
SET(CMAKE_INCLUDE_CURRENT_DIR TRUE)

LIST(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeScripts")
INCLUDE(ConfigPaths)
INCLUDE(DefineDependsandFlags)
INCLUDE(HelperMacros)
# Set to true if compiler/linker should enable profiling:
#SET(ENABLE_PROFILING TRUE)

#make dist target
SET(INKSCAPE_DIST_PREFIX "${PROJECT_NAME}-${INKSCAPE_VERSION}")
ADD_CUSTOM_TARGET(dist svn export --force -q "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/${INKSCAPE_DIST_PREFIX}" 
		COMMAND tar -czf "${CMAKE_BINARY_DIR}/${INKSCAPE_DIST_PREFIX}.tar.gz" -C "${CMAKE_BINARY_DIR}" --exclude=".hidden" ${INKSCAPE_DIST_PREFIX}
		WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

#make unistall target
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

SET(folders
src
cxxtest
share
doc
#doc-pak
po
)

FOREACH(dirlist ${folders})
ADD_SUBDIRECTORY(${dirlist})
ENDFOREACH(dirlist)
