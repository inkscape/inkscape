set(filters_SRC
	blend.cpp
	colormatrix.cpp
	componenttransfer.cpp
	componenttransfer-funcnode.cpp
	composite.cpp
	convolvematrix.cpp
	diffuselighting.cpp
	displacementmap.cpp
	distantlight.cpp
	flood.cpp
	image.cpp
	merge.cpp
	mergenode.cpp
	morphology.cpp
	offset.cpp
	pointlight.cpp
	specularlighting.cpp
	spotlight.cpp
	tile.cpp
	turbulence.cpp
)
add_library(filters STATIC ${filters_SRC})
target_link_libraries(filters 2geom ${INKSCAPE_LIBS})
