include(UseGlibMarshal)

GLIB_MARSHAL(sp_marshal sp-marshal "${CMAKE_CURRENT_BINARY_DIR}/helper")
SET(GlibOutput
${CMAKE_CURRENT_BINARY_DIR}/sp-marshal.cpp
${CMAKE_CURRENT_BINARY_DIR}/sp-marshal.h
)

SET(helper_SRC
action.cpp
geom.cpp
geom-nodetype.cpp
gnome-utils.cpp
pixbuf-ops.cpp
png-write.cpp
stock-items.cpp
unit-menu.cpp
units.cpp
#units-test.cpp
unit-tracker.cpp
window.cpp
sp-marshal.list
# we generate this file and it's .h counter-part
${CMAKE_CURRENT_BINARY_DIR}/sp-marshal.cpp
${CMAKE_CURRENT_BINARY_DIR}/sp-marshal.h
)
ADD_LIBRARY(helper STATIC ${helper_SRC})