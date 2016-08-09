# -----------------------------------------------------------------------------
#
# -----------------------------------------------------------------------------
add_subdirectory(${GMOCK_DIR} ${CMAKE_BINARY_DIR}/gmock)

include_directories(SYSTEM ${GMOCK_DIR}/gtest/include
                           ${GMOCK_DIR}/include
                           ${CMAKE_SOURCE_DIR}/gtest/gtest/include)

set_source_files_properties(
		${CMAKE_BINARY_DIR}/src/inkscape-version.cpp
		PROPERTIES GENERATED TRUE)

include_directories(${CMAKE_BINARY_DIR}/src)

set(TEST_SOURCES
	attributes-test
	color-profile-test
	dir-util-test
	sp-object-test
	object-set-test)

set(TEST_LIBS
	gmock_main
	inkscape_base
	nrtype_LIB
	croco_LIB
	avoid_LIB
	cola_LIB
	vpsc_LIB
	livarot_LIB
	uemf_LIB
	2geom_LIB
	depixelize_LIB
	util_LIB
	gc_LIB
	${INKSCAPE_LIBS}
)

foreach(source ${TEST_SOURCES})
	add_executable(${source} src/${source}.cpp unittest.cpp doc-per-case-test.cpp)
	target_link_libraries(${source} ${TEST_LIBS})
	add_test(NAME ${source} COMMAND ${EXECUTABLE_OUTPUT_PATH}/${source})
	add_dependencies(${source} inkscape_version)
	add_dependencies(check ${source})
endforeach()