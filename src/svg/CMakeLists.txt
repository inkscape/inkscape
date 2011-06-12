set(svg_SRC
	css-ostringstream.cpp
	#ftos.cpp
	itos.cpp
	path-string.cpp
	round.cpp
	sp-svg.def
	stringstream.cpp
	strip-trailing-zeros.cpp
	svg-affine.cpp
	svg-color.cpp
	svg-length.cpp
	svg-path.cpp
	#test-stubs.cpp
)

add_library(svg_LIB STATIC ${svg_SRC})
target_link_libraries(svg_LIB 2geom_LIB ${INKSCAPE_LIBS})
