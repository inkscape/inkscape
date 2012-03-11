
set(dialogs_SRC
	clonetiler.cpp
	dialog-events.cpp
	find.cpp

	# -------
	# Headers
	clonetiler.h
	dialog-events.h
	find.h
)

# add_inkscape_lib(dialogs_LIB "${dialogs_SRC}")
add_inkscape_source("${dialogs_SRC}")
