SET(ui_SRC
clipboard.cpp
clipboard.h
context-menu.cpp
context-menu.h
icons.cpp
icons.h
previewable.h
previewfillable.h
previewholder.cpp
previewholder.h
stock.cpp
stock.h
stock-items.cpp
stock-items.h
${ui_cache_SRC}
${ui_dialog_SRC}
${ui_view_SRC}
${ui_widget_SRC}
)
# Add New folders in src folder here
SET(uifolders
cache
dialog
view
widget
)

FOREACH(uidirlist ${uifolders})
ADD_SUBDIRECTORY(${uidirlist})
ENDFOREACH(uidirlist)
