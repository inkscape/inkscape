SET(trace_SRC
filterset.cpp
filterset.h
imagemap.cpp
imagemap.h
imagemap-gdk.cpp
imagemap-gdk.h
pool.h
quantize.cpp
quantize.h
siox.cpp
siox.h
trace.cpp
trace.h
${trace_potrace_SRC}
)
# Add New folders in src folder here
SET(tracefolders
potrace
)

FOREACH(tracedirlist ${tracefolders})
ADD_SUBDIRECTORY(${tracedirlist})
ENDFOREACH(tracedirlist)