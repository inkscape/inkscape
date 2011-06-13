
set(debug_SRC
	demangle.cpp
	heap.cpp
	log-display-config.cpp
	logger.cpp
	sysv-heap.cpp
	timestamp.cpp
	gdk-event-latency-tracker.cpp
)

add_library(debug_LIB ${debug_SRC})
