
set(nr_SRC
	# in-svg-plane-test.cpp
	nr-object.cpp
	nr-point-fns.cpp
	# nr-point-fns-test.cpp
	nr-rect.cpp
	nr-rect-l.cpp
	# nr-rotate-fns-test.cpp
	#nr-translate-test.cpp
	nr-types.cpp
	# nr-types-test.cpp
	nr-values.cpp
	# testnr.cpp


	# -------
	# Headers
	# in-svg-plane-test.h
	in-svg-plane.h
	nr-convert2geom.h
	nr-coord.h
	nr-dim2.h
	nr-forward.h
	nr-i-coord.h
	nr-macros.h
	nr-object.h
	# nr-point-fns-test.h
	nr-point-fns.h
	nr-point-l.h
	nr-point-ops.h
	nr-point.h
	nr-rect-l.h
	nr-rect-ops.h
	nr-rect.h
	nr-render.h
	# nr-translate-test.h
	# nr-types-test.h
	nr-types.h
	nr-values.h
)

add_inkscape_lib(nr_LIB "${nr_SRC}")
