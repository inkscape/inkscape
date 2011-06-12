set(libcola_SRC
	cola.cpp
	conjugate_gradient.cpp
	connected_components.cpp
	cycle_detector.cpp
	gradient_projection.cpp
	shortest_paths.cpp
	straightener.cpp
)
add_library(cola STATIC ${libcola_SRC})
target_link_libraries(cola ${INKSCAPE_LIBS})