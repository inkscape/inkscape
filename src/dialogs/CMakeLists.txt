
set(dialogs_SRC
	dialog-events.cpp
	find.cpp

	# -------
	# Headers
	dialog-events.h
	find.h
)

# add_inkscape_lib(dialogs_LIB "${dialogs_SRC}")
add_inkscape_source("${dialogs_SRC}")
