
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


	# -------
	# Headers
	attribute-record.h
	comment-node.h
	composite-node-observer.h
	croco-node-iface.h
	document.h
	element-node.h
	event-fns.h
	event.h
	helper-observer.h
	invalid-operation-exception.h
	log-builder.h
	node-event-vector.h
	node-fns.h
	node-iterators.h
	node-observer.h
	node.h
	pi-node.h
	quote-test.h
	quote.h
	rebase-hrefs-test.h
	rebase-hrefs.h
	repr-action-test.h
	repr-sorting.h
	repr.h
	simple-document.h
	simple-node.h
	sp-css-attr.h
	subtree.h
	text-node.h
)

# add_inkscape_lib(xml_LIB "${xml_SRC}")
add_inkscape_source("${xml_SRC}")
