SET(libcola_SRC
cola.cpp
cola.h
conjugate_gradient.cpp
conjugate_gradient.h
connected_components.cpp
cycle_detector.cpp
cycle_detector.h
defs.h
gradient_projection.cpp
gradient_projection.h
shortest_paths.cpp
shortest_paths.h
straightener.cpp
straightener.h
)
ADD_LIBRARY(cola STATIC ${libcola_SRC})
