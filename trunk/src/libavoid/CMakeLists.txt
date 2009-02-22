SET(libavoid_SRC
connector.cpp
geometry.cpp
graph.cpp
makepath.cpp
polyutil.cpp
region.cpp
router.cpp
shape.cpp
static.cpp
timer.cpp
vertices.cpp
visibility.cpp
)
ADD_LIBRARY(avoid STATIC ${libavoid_SRC})
TARGET_LINK_LIBRARIES(avoid
${INKSCAPE_LIBS})