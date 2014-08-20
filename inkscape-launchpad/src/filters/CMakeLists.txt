
set(filters_SRC
	blend.cpp
	colormatrix.cpp
	componenttransfer-funcnode.cpp
	componenttransfer.cpp
	composite.cpp
	convolvematrix.cpp
	diffuselighting.cpp
	displacementmap.cpp
	distantlight.cpp
	flood.cpp
	gaussian-blur.cpp
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


	# -------
	# Headers
	blend.h
	colormatrix.h
	componenttransfer-funcnode.h
	componenttransfer.h
	composite.h
	convolvematrix.h
	diffuselighting.h
	displacementmap.h
	distantlight.h
	flood.h
	gaussian-blur.h
	image.h
	merge.h
	mergenode.h
	morphology.h
	offset.h
	pointlight.h
	specularlighting.h
	spotlight.h
	tile.h
	turbulence.h
)

# add_inkscape_lib(filters_LIB "${filters_SRC}"")
add_inkscape_source("${filters_SRC}")
