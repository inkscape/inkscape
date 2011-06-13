
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

# add_library(io_LIB ${io_SRC})
add_inkscape_source("${io_SRC}")
