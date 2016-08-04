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

# include_directories(${CMAKE_CURRENT_BINARY_DIR}/__/src)
include_directories(${CMAKE_BINARY_DIR}/src)

add_executable(unittest
	unittest.cpp
	doc-per-case-test.cpp
	src/attributes-test.cpp
	src/color-profile-test.cpp
	src/dir-util-test.cpp
)

target_link_libraries(unittest inkscape_base)

add_dependencies(unittest inkscape_version)

set (_optional_unittest_libs )

target_link_libraries(unittest
	gmock_main

	# order from automake
	#sp_LIB

	#inkscape_LIB
	#sp_LIB # annoying, we need both!
	nrtype_LIB # annoying, we need both!

	croco_LIB
	avoid_LIB
	${_optional_unittest_libs}
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


add_test(BaseTest ${EXECUTABLE_OUTPUT_PATH}/unittest)
add_dependencies(check unittest)

#
