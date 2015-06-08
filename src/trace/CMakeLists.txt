
set(trace_SRC
	filterset.cpp
	imagemap.cpp
	imagemap-gdk.cpp
	quantize.cpp
	siox.cpp
	trace.cpp

	potrace/curve.cpp
	potrace/decompose.cpp
	potrace/greymap.cpp
	potrace/inkscape-potrace.cpp
	potrace/potracelib.cpp
	potrace/render.cpp
	potrace/trace.cpp


	# -------
	# Headers
	filterset.h
	imagemap-gdk.h
	imagemap.h
	pool.h
	quantize.h
	siox.h
	trace.h

	potrace/auxiliary.h
	potrace/bitmap.h
	potrace/bitops.h
	potrace/curve.h
	potrace/decompose.h
	potrace/greymap.h
	potrace/inkscape-potrace.h
	potrace/lists.h
	potrace/potracelib.h
	potrace/progress.h
	potrace/render.h
	potrace/trace.h
)

# add_inkscape_lib(trace_LIB "${trace_SRC}")
add_inkscape_source("${trace_SRC}")
