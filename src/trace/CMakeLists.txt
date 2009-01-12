ADD_SUBDIRECTORY(potrace)
SET(trace_SRC
filterset.cpp
imagemap.cpp
imagemap-gdk.cpp
quantize.cpp
siox.cpp
trace.cpp
${trace_potrace_SRC}
)
ADD_LIBRARY(trace STATIC ${trace_SRC})
TARGET_LINK_LIBRARIES(trace
2geom ${INKSCAPE_LIBS})