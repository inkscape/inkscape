
set(libgdl_SRC
	gdl-dock.c
	gdl-dock-bar.c
	gdl-dock-item.c
	gdl-dock-item-grip.c
	gdl-dock-master.c
	gdl-dock-notebook.c
	gdl-dock-object.c
	gdl-dock-paned.c
	gdl-dock-placeholder.c
	gdl-dock-tablabel.c
	gdl-i18n.c
	gdl-stock.c
	gdl-switcher.c
	gdl-tools.h
	libgdlmarshal.c
	libgdltypebuiltins.c


	# -------
	# Headers
	gdl-dock-bar.h
	gdl-dock-item-grip.h
	gdl-dock-item.h
	gdl-dock-master.h
	gdl-dock-notebook.h
	gdl-dock-object.h
	gdl-dock-paned.h
	gdl-dock-placeholder.h
	gdl-dock-tablabel.h
	gdl-dock.h
	gdl-i18n.h
	gdl-stock-icons.h
	gdl-stock.h
	gdl-switcher.h
	libgdl.h
	libgdlmarshal.h
	libgdltypebuiltins.h
)

if(WIN32)
	list(APPEND libgdl_SRC
		gdl-win32.c
		gdl-win32.h
	)
endif()

add_inkscape_lib(gdl_LIB "${libgdl_SRC}")
