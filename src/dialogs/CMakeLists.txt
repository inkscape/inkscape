
set(dialogs_SRC
	dialog-events.cpp

	# -------
	# Headers
	dialog-events.h
)

# add_inkscape_lib(dialogs_LIB "${dialogs_SRC}")
add_inkscape_source("${dialogs_SRC}")
