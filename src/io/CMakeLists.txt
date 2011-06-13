
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


	# -------
	# Headers
	base64stream.h
	ftos.h
	gzipstream.h
	inkjar.h
	inkscapestream.h
	resource.h
	simple-sax.h
	stringstream.h
	sys.h
	uristream.h
	xsltstream.h
)

# add_inkscape_lib(io_LIB "${io_SRC}")
add_inkscape_source("${io_SRC}")
