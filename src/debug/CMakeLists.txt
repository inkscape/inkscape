SET(debug
demangle.cpp
#demangle.h
#event.h
#event-tracker.h
#gc-heap.h
heap.cpp
#heap.h
log-display-config.cpp
#log-display-config.h
logger.cpp
#logger.h
#simple-event.h
sysv-heap.cpp
#sysv-heap.h
timestamp.cpp
#timestamp.h
)

ADD_LIBRARY(inkdebug STATIC ${debug})
