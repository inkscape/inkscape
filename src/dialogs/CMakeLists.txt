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
add_library(dialogs_LIB STATIC ${dialogs_SRC})
target_link_libraries(dialogs_LIB 2geom_LIB ${INKSCAPE_LIBS})
