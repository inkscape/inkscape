
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
)

add_library(avoid_LIB ${libavoid_SRC})
