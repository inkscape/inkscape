
include(UseGlibMarshal)

GLIB_MARSHAL(sp_marshal sp-marshal "${CMAKE_CURRENT_BINARY_DIR}/helper")

set(sp_marshal_SRC
	${CMAKE_CURRENT_BINARY_DIR}/sp-marshal.cpp
	${CMAKE_CURRENT_BINARY_DIR}/sp-marshal.h
)

set(helper_SRC
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
	sp-marshal.cpp
	sp-marshal.list
	# we generate this file and it's .h counter-part
	${sp_marshal_SRC}
)

add_library(helper_LIB ${helper_SRC})
