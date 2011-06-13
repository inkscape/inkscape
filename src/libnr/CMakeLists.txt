
set(nr_SRC
	# in-svg-plane-test.cpp
	nr-blit.cpp
	nr-compose.cpp
	nr-compose-transform.cpp
	nr-gradient.cpp
	nr-matrix.cpp
	nr-matrix-div.cpp
	nr-matrix-fns.cpp
	nr-matrix-rotate-ops.cpp
	nr-object.cpp
	nr-pixblock.cpp
	nr-pixblock-line.cpp
	nr-pixblock-pattern.cpp
	nr-pixblock-pixel.cpp
	nr-point-fns.cpp
	# nr-point-fns-test.cpp
	nr-rect.cpp
	nr-rect-l.cpp
	nr-rotate-fns.cpp
	# nr-rotate-fns-test.cpp
	nr-rotate-matrix-ops.cpp
	nr-scale-matrix-ops.cpp
	nr-scale-translate-ops.cpp
	nr-translate-matrix-ops.cpp
	nr-translate-rotate-ops.cpp
	nr-translate-scale-ops.cpp
	#nr-translate-test.cpp
	nr-types.cpp
	# nr-types-test.cpp
	nr-values.cpp
	# testnr.cpp

	# -------
	# Headers
	# in-svg-plane-test.h
	in-svg-plane.h
	nr-blit.h
	nr-compose-reference.h
	nr-compose-test.h
	nr-compose-transform.h
	nr-compose.h
	nr-convert2geom.h
	nr-convex-hull-ops.h
	nr-convex-hull.h
	nr-coord.h
	nr-dim2.h
	nr-forward.h
	nr-gradient.h
	nr-i-coord.h
	nr-macros.h
	nr-matrix-div.h
	nr-matrix-fns.h
	nr-matrix-ops.h
	nr-matrix-rotate-ops.h
	nr-matrix-scale-ops.h
	nr-matrix-test.h
	nr-matrix-translate-ops.h
	nr-matrix.h
	nr-maybe.h
	nr-object.h
	nr-path-code.h
	nr-pixblock-line.h
	nr-pixblock-pattern.h
	nr-pixblock-pixel.h
	nr-pixblock.h
	nr-pixops.h
	# nr-point-fns-test.h
	nr-point-fns.h
	nr-point-l.h
	nr-point-matrix-ops.h
	nr-point-ops.h
	nr-point.h
	nr-rect-l.h
	nr-rect-ops.h
	nr-rect.h
	nr-render.h
	nr-rotate-fns-test.h
	nr-rotate-fns.h
	nr-rotate-matrix-ops.h
	nr-rotate-ops.h
	nr-rotate-test.h
	nr-rotate.h
	nr-scale-matrix-ops.h
	nr-scale-ops.h
	nr-scale-test.h
	nr-scale-translate-ops.h
	nr-scale.h
	nr-translate-matrix-ops.h
	nr-translate-ops.h
	nr-translate-rotate-ops.h
	nr-translate-scale-ops.h
	# nr-translate-test.h
	nr-translate.h
	# nr-types-test.h
	nr-types.h
	nr-values.h
)

add_inkscape_lib(nr_LIB "${nr_SRC}")
