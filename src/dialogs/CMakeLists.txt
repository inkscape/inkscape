
set(dialogs_SRC
	clonetiler.cpp
	dialog-events.cpp
	export.cpp
	find.cpp
	spellcheck.cpp
	text-edit.cpp
	xml-tree.cpp


	# -------
	# Headers
	clonetiler.h
	dialog-events.h
	export.h
	find.h
	spellcheck.h
	text-edit.h
	xml-tree.h
)

# add_inkscape_lib(dialogs_LIB "${dialogs_SRC}")
add_inkscape_source("${dialogs_SRC}")
