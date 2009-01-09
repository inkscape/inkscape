SET(debug_SRC
demangle.cpp
heap.cpp
log-display-config.cpp
logger.cpp
sysv-heap.cpp
timestamp.cpp
gdk-event-latency-tracker.cpp
)
ADD_LIBRARY(debug STATIC ${debug_SRC})