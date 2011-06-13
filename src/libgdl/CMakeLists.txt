
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
)

if(WIN32)
	list(APPEND libgdl_SRC
		gdl-win32.c
	)
endif()

add_library(gdl_LIB ${libgdl_SRC})
