SET(graphlayout_SRC
graphlayout.cpp
)
ADD_LIBRARY(graphlayout STATIC ${graphlayout_SRC})
TARGET_LINK_LIBRARIES(graphlayout
2geom ${INKSCAPE_LIBS})