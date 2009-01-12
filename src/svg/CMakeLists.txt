SET(svg_SRC
css-ostringstream.cpp
#ftos.cpp
itos.cpp
path-string.cpp
round.cpp
sp-svg.def
stringstream.cpp
strip-trailing-zeros.cpp
svg-affine.cpp
svg-color.cpp
svg-length.cpp
svg-path.cpp
#test-stubs.cpp
)
ADD_LIBRARY(svg STATIC ${svg_SRC})
TARGET_LINK_LIBRARIES(svg
2geom ${INKSCAPE_LIBS})