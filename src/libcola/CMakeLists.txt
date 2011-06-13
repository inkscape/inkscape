
set(libcola_SRC
	cola.cpp
	conjugate_gradient.cpp
	connected_components.cpp
	# cycle_detector.cpp
	gradient_projection.cpp
	shortest_paths.cpp
	straightener.cpp


	# -------
	# Headers
	cola.h
	conjugate_gradient.h
	# cycle_detector.h
	defs.h
	gradient_projection.h
	shortest_paths.h
	straightener.h
)

add_inkscape_lib(cola_LIB "${libcola_SRC}")
