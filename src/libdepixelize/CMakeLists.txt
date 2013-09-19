
set(libdepixelize_SRC
	kopftracer2011.cpp

	# -------
	# Headers
	kopftracer2011.h
	splines.h

	priv/branchless.h
	priv/colorspace.h
	priv/homogeneoussplines.h
	priv/iterator.h
	priv/pixelgraph.h
	priv/point.h
	priv/simplifiedvoronoi.h
	priv/splines.h
)

add_inkscape_lib(depixelize_LIB "${libdepixelize_SRC}")
