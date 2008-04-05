include(UseGlibMarshal)

SET(Marshal
sp-marshal.list
)

GLIB_MARSHAL(sp_marshal ${Marshal} ${CMAKE_CURRENT_BINARY_DIR})

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
sp-marshal.cpp
)
