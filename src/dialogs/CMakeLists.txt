set(dialogs_SRC
	clonetiler.cpp
	dialog-events.cpp
	export.cpp
	find.cpp
	item-properties.cpp
	object-attributes.cpp
	spellcheck.cpp
	text-edit.cpp
	xml-tree.cpp
)
add_library(dialogs STATIC ${dialogs_SRC})
target_link_libraries(dialogs 2geom ${INKSCAPE_LIBS})
