
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

	# -------
	# Headers
	blend-fns.h
	blend.h
	colormatrix-fns.h
	colormatrix.h
	componenttransfer-fns.h
	componenttransfer-funcnode.h
	componenttransfer.h
	composite-fns.h
	composite.h
	convolvematrix-fns.h
	convolvematrix.h
	diffuselighting-fns.h
	diffuselighting.h
	displacementmap-fns.h
	displacementmap.h
	distantlight.h
	flood-fns.h
	flood.h
	image-fns.h
	image.h
	merge-fns.h
	merge.h
	mergenode.h
	morphology-fns.h
	morphology.h
	offset-fns.h
	offset.h
	pointlight.h
	specularlighting-fns.h
	specularlighting.h
	spotlight.h
	tile-fns.h
	tile.h
	turbulence-fns.h
	turbulence.h
)

# add_inkscape_lib(filters_LIB "${filters_SRC}"")
add_inkscape_source("${filters_SRC}")
