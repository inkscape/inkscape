
set(2geom_SRC
	affine.cpp
	basic-intersection.cpp
	bezier.cpp
	bezier-clipping.cpp
	bezier-curve.cpp
	bezier-utils.cpp
	cairo-path-sink.cpp
	circle.cpp
	# conic_section_clipper_impl.cpp
	# conicsec.cpp
	convex-hull.cpp
	coord.cpp
	crossing.cpp
	curve.cpp
	d2-sbasis.cpp
	ellipse.cpp
	elliptical-arc.cpp
	elliptical-arc-from-sbasis.cpp
	geom.cpp
	intersection-graph.cpp
	line.cpp
	nearest-time.cpp
	numeric/matrix.cpp
	path-intersection.cpp
	path-sink.cpp
	path.cpp
	pathvector.cpp
	piecewise.cpp
	point.cpp
	polynomial.cpp
	rect.cpp
	# recursive-bezier-intersection.cpp
	sbasis-2d.cpp
	sbasis-geometric.cpp
	sbasis-math.cpp
	sbasis-poly.cpp
	sbasis-roots.cpp
	sbasis-to-bezier.cpp
	sbasis.cpp
	solve-bezier.cpp
	solve-bezier-one-d.cpp
	solve-bezier-parametric.cpp
	svg-path-parser.cpp
	svg-path-writer.cpp
	sweep-bounds.cpp
	toposweep.cpp
	transforms.cpp
	utils.cpp


	# -------
	2geom.h
	# Headers
	affine.h
	angle.h
	basic-intersection.h
	bezier-curve.h
	bezier-to-sbasis.h
	bezier-utils.h
	bezier.h
	choose.h
	circle.h
	concepts.h
	conic_section_clipper.h
	conic_section_clipper_cr.h
	conic_section_clipper_impl.h
	conicsec.h
	coord.h
	crossing.h
	curve.h
	curves.h
	d2.h
	ellipse.h
	elliptical-arc.h
	exception.h
	forward.h
	generic-interval.h
	generic-rect.h
	geom.h
	int-interval.h
	int-point.h
	int-rect.h
	interval.h
	line.h
	linear.h
	math-utils.h
	ord.h
	path-intersection.h
	path-sink.h
	path.h
	pathvector.h
	piecewise.h
	point.h
	polynomial.h
	ray.h
	rect.h
	sbasis-2d.h
	sbasis-curve.h
	sbasis-geometric.h
	sbasis-math.h
	sbasis-poly.h
	sbasis-to-bezier.h
	sbasis.h
	solver.h
	svg-path-parser.h
	svg-path-writer.h
	sweeper.h
	sweep-bounds.h
	toposweep.h
	transforms.h
	utils.h

	numeric/fitting-model.h
	numeric/fitting-tool.h
	numeric/linear_system.h
	numeric/matrix.h
	numeric/symmetric-matrix-fs-operation.h
	numeric/symmetric-matrix-fs-trace.h
	numeric/symmetric-matrix-fs.h
	numeric/vector.h
)

# make lib for 2geom_LIB
add_inkscape_lib(2geom_LIB "${2geom_SRC}")
