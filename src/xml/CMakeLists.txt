SET(xml_SRC
composite-node-observer.cpp
croco-node-iface.cpp
event.cpp
log-builder.cpp
node-fns.cpp
quote.cpp
#quote-test.cpp
repr.cpp
#repr-action-test.cpp
repr-css.cpp
repr-io.cpp
repr-sorting.cpp
repr-util.cpp
simple-document.cpp
simple-node.cpp
subtree.cpp
)
ADD_LIBRARY(xml STATIC ${xml_SRC})
TARGET_LINK_LIBRARIES(xml
2geom ${INKSCAPE_LIBS})