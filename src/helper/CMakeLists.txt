include(UseGlibMarshal)

GLIB_MARSHAL(sp_marshal sp-marshal "${CMAKE_CURRENT_BINARY_DIR}/helper")

set(GlibOutput
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
	${CMAKE_CURRENT_BINARY_DIR}/sp-marshal.cpp
	${CMAKE_CURRENT_BINARY_DIR}/sp-marshal.h
)

add_library(helper STATIC ${helper_SRC})
target_link_libraries(helper 2geom ${INKSCAPE_LIBS})