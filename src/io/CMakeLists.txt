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
add_library(io STATIC ${io_SRC})
target_link_libraries(io 2geom ${INKSCAPE_LIBS})
