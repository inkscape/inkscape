if (${HAVE_POTRACE})

set(trace_SRC
	filterset.cpp
	imagemap.cpp
	imagemap-gdk.cpp
	quantize.cpp
	siox.cpp
	trace.cpp

	potrace/inkscape-potrace.cpp

	# -------
	# Headers
	filterset.h
	imagemap-gdk.h
	imagemap.h
	pool.h
	quantize.h
	siox.h
	trace.h

	potrace/bitmap.h
	potrace/inkscape-potrace.h
)

# add_inkscape_lib(trace_LIB "${trace_SRC}")
add_inkscape_source("${trace_SRC}")

endif()
