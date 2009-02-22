SET(inkjar_SRC
jar.cpp
)
ADD_LIBRARY(inkjar STATIC ${inkjar_SRC})
TARGET_LINK_LIBRARIES(inkjar
2geom ${INKSCAPE_LIBS})