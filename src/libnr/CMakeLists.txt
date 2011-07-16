
set(nr_SRC
	# in-svg-plane-test.cpp
	nr-object.cpp
	nr-point-fns.cpp
	# nr-point-fns-test.cpp
	nr-rect.cpp
	nr-rect-l.cpp
	# nr-rotate-fns-test.cpp
	#nr-translate-test.cpp
	# nr-types-test.cpp
	nr-values.cpp
	# testnr.cpp


	# -------
	# Headers
	# in-svg-plane-test.h
	in-svg-plane.h
	nr-convert2geom.h
	nr-forward.h
	nr-macros.h
	nr-object.h
	# nr-point-fns-test.h
	nr-point-fns.h
	nr-rect-l.h
	nr-rect.h
	# nr-translate-test.h
	# nr-types-test.h
	nr-values.h
)

add_inkscape_lib(nr_LIB "${nr_SRC}")
