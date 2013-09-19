
set(libdepixelize_SRC
	kopftracer2011.cpp

	# -------
	# Headers
        kopftracer2011.h
        splines.h
)

add_inkscape_lib(depixelize_LIB "${libdepixelize_SRC}")
