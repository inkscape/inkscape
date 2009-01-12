SET(removeoverlap_SRC
removeoverlap.cpp
)
ADD_LIBRARY(removeoverlap STATIC ${removeoverlap_SRC})
TARGET_LINK_LIBRARIES(removeoverlap
2geom ${INKSCAPE_LIBS})