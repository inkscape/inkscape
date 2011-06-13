
set(debug_SRC
	demangle.cpp
	heap.cpp
	log-display-config.cpp
	logger.cpp
	sysv-heap.cpp
	timestamp.cpp
	gdk-event-latency-tracker.cpp


	# ------
	# Header
	demangle.h
	event-tracker.h
	event.h
	gc-heap.h
	gdk-event-latency-tracker.h
	heap.h
	log-display-config.h
	logger.h
	simple-event.h
	sysv-heap.h
	timestamp.h
)

# add_inkscape_lib(debug_LIB "${debug_SRC}")
add_inkscape_source("${debug_SRC}")
