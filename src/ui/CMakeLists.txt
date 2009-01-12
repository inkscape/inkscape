SET(uifolders
cache
dialog
view
widget
)
FOREACH(uilistsrc ${uifolders})
  ADD_SUBDIRECTORY(${uilistsrc})
ENDFOREACH(uilistsrc)

SET(ui_SRC
clipboard.cpp
context-menu.cpp
previewholder.cpp
stock.cpp
stock-items.cpp
${ui_cache_SRC}
${ui_dialog_SRC}
${ui_view_SRC}
${ui_widget_SRC}
)
ADD_LIBRARY(ui STATIC ${ui_SRC})
TARGET_LINK_LIBRARIES(ui
2geom ${INKSCAPE_LIBS})