set(xml_SRC
	composite-node-observer.cpp
	croco-node-iface.cpp
	event.cpp
	log-builder.cpp
	node-fns.cpp
	quote.cpp
	repr.cpp
	repr-css.cpp
	repr-io.cpp
	repr-sorting.cpp
	repr-util.cpp
	simple-document.cpp
	simple-node.cpp
	subtree.cpp
	helper-observer.cpp
	rebase-hrefs.cpp
)

add_library(xml_LIB STATIC ${xml_SRC})
target_link_libraries(xml_LIB 2geom_LIB ${INKSCAPE_LIBS})
