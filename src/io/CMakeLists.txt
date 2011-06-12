set(io_SRC
	base64stream.cpp
	ftos.cpp
	gzipstream.cpp
	inkjar.cpp
	inkscapestream.cpp
	resource.cpp
	simple-sax.cpp
	stringstream.cpp
	sys.cpp
	uristream.cpp
	xsltstream.cpp
)

add_library(io_LIB STATIC ${io_SRC})
target_link_libraries(io_LIB 2geom_LIB ${INKSCAPE_LIBS})
