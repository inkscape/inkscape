
set(libdepixelize_SRC
	kopftracer2011.cpp

	# -------
	# Headers
	kopftracer2011.h
	splines.h

	priv/branchless.h
	priv/colorspace.h
	priv/curvature.h
	priv/homogeneoussplines.h
	priv/integral.h
	priv/iterator.h
	priv/optimization-kopf2011.h
	priv/pixelgraph.h
	priv/point.h
	priv/simplifiedvoronoi.h
	priv/splines-kopf2011.h
)

add_inkscape_lib(depixelize_LIB "${libdepixelize_SRC}")
