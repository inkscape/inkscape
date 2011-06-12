add_subdirectory(potrace)

set(trace_SRC
	filterset.cpp
	imagemap.cpp
	imagemap-gdk.cpp
	quantize.cpp
	siox.cpp
	trace.cpp
	${trace_potrace_SRC}
)

ADD_LIBRARY(trace_LIB STATIC ${trace_SRC})
TARGET_LINK_LIBRARIES(trace_LIB 2geom_LIB ${INKSCAPE_LIBS})
