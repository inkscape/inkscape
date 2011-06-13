
set(libavoid_SRC
	connector.cpp
	geometry.cpp
	geomtypes.cpp
	graph.cpp
	makepath.cpp
	orthogonal.cpp
	router.cpp
	shape.cpp
	timer.cpp
	vertices.cpp
	viscluster.cpp
	visibility.cpp
	vpsc.cpp

	# -------
	# Headers
	assertions.h
	connector.h
	debug.h
	geometry.h
	geomtypes.h
	graph.h
	libavoid.h
	makepath.h
	orthogonal.h
	router.h
	shape.h
	timer.h
	vertices.h
	viscluster.h
	visibility.h
	vpsc.h
)

add_inkscape_lib(avoid_LIB "${libavoid_SRC}")
