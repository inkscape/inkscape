
set(debug_SRC
	demangle.cpp
	heap.cpp
	log-display-config.cpp
	logger.cpp
	sysv-heap.cpp
	timestamp.cpp
	gdk-event-latency-tracker.cpp
)

add_library(debug STATIC ${debug_SRC})
target_link_libraries(debug 2geom ${INKSCAPE_LIBS})
