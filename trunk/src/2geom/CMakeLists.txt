SET(2GEOM_SRC

svg-path.h
svg-path.cpp
svg-path-parser.h
svg-path-parser.cpp

ord.h

#nearestpoint.cpp
nearest-point.cpp
nearest-point.h

bezier-curve.h
circle.cpp
circle.h
curve.h
curves.h
curve-helpers.cpp
ellipse.cpp
ellipse.h
elliptical-arc.cpp
elliptical-arc.h
hvlinesegment.h
sbasis-curve.h
path.cpp
path.h
path-intersection.cpp
path-intersection.h
pathvector.cpp
pathvector.h

forward.h

shape.cpp
shape.h
region.cpp
region.h
crossing.h
crossing.cpp
sweep.cpp
sweep.h

poly.cpp
poly.h
poly-dk-solve.cpp
poly-dk-solve.h
poly-laguerre-solve.cpp
poly-laguerre-solve.h

quadtree.cpp
quadtree.h

matrix.cpp
matrix.h
transforms.cpp
transforms.h

point.h
point.cpp
point-l.h

coord.h

d2.h
d2-sbasis.h
d2-sbasis.cpp
rect.h

piecewise.h
piecewise.cpp

sbasis.cpp
sbasis.h
sbasis-2d.h
sbasis-2d.cpp
sbasis-geometric.cpp
sbasis-geometric.h
sbasis-math.h
sbasis-math.cpp
sbasis-poly.cpp
sbasis-poly.h
#chebyshev.cpp # requires gsl, not useful, I think
#chebyshev.h
sbasis-roots.cpp
sbasis-to-bezier.cpp
sbasis-to-bezier.h

bezier-to-sbasis.h

basic-intersection.h
basic-intersection.cpp

geom.cpp
geom.h

#utils.cpp
utils.h
exception.h
angle.h

bezier-utils.cpp
bezier-utils.h
choose.h
circulator.h
conjugate_gradient.cpp
conjugate_gradient.h
convex-cover.cpp
convex-cover.h
solve-bezier-one-d.cpp
solve-bezier-parametric.cpp
solver.h
sturm.h
svg-elliptical-arc.cpp
svg-elliptical-arc.h

#arc-length.cpp
#arc-length.h

numeric/matrix.cpp
)

# make lib for 2geom
ADD_LIBRARY(2geom STATIC ${2GEOM_SRC})
#TARGET_LINK_LIBRARIES(2geom blas gsl)
TARGET_LINK_LIBRARIES(2geom ${INKSCAPE_LIBS})
