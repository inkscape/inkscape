IF(WIN32)
SET(GDL_WIN
gdl-win32.c
)
ENDIF(WIN32)

SET(libgdl_SRC
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
gdl-tools.hlibgdlmarshal.c
libgdltypebuiltins.c
${GDL_WIN}
)
ADD_LIBRARY(gdl STATIC ${libgdl_SRC})
