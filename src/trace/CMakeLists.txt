
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
)

# add_library(trace_LIB ${trace_SRC})
add_inkscape_source("${trace_SRC}")
