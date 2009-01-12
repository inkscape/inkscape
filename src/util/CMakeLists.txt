SET(util_SRC
share.cpp
units.cpp
)
ADD_LIBRARY(util STATIC ${util_SRC})
TARGET_LINK_LIBRARIES(util
2geom ${INKSCAPE_LIBS})