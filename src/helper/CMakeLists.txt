include(UseGlibMarshal)

GLIB_MARSHAL(sp_marshal helper/sp-marshal "${CMAKE_CURRENT_BINARY_DIR}/helper")

SET(helper_SRC
action.cpp
gnome-utils.cpp
pixbuf-ops.cpp
png-write.cpp
stock-items.cpp
unit-menu.cpp
units.cpp
units-test.cpp
unit-tracker.cpp
window.cpp
sp-marshal.list
# we generate this file and it's .h counter-part
#sp-marshal.cpp
)
