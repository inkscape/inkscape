
set(io_SRC
	base64stream.cpp
	bufferstream.cpp
	gzipstream.cpp
	inkjar.cpp
	inkscapestream.cpp
	resource.cpp
	stringstream.cpp
	sys.cpp
	uristream.cpp
	xsltstream.cpp

	# -------
	# Headers
	base64stream.h
	bufferstream.h
	gzipstream.h
	inkjar.h
	inkscapestream.h
	resource.h
	stringstream.h
	sys.h
	uristream.h
	xsltstream.h
)

# add_inkscape_lib(io_LIB "${io_SRC}")
add_inkscape_source("${io_SRC}")
