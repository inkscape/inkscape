SET(dialogs_SRC
clonetiler.cpp
dialog-events.cpp
export.cpp
extensions.cpp
fill-style.cpp
find.cpp
guidelinedialog.cpp
iconpreview.cpp
in-dt-coordsys.cpp
item-properties.cpp
layer-properties.cpp
layers-panel.cpp
object-attributes.cpp
rdf.cpp
sp-attribute-widget.cpp
stroke-style.cpp
swatches.cpp
text-edit.cpp
unclump.cpp
xml-tree.cpp
)
ADD_LIBRARY(dialogs STATIC ${dialogs_SRC})
TARGET_LINK_LIBRARIES(dialogs
2geom ${INKSCAPE_LIBS})
