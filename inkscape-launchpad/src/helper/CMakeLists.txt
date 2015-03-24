
include(${CMAKE_SOURCE_DIR}/CMakeScripts/UseGlibMarshal.cmake)

GLIB_MARSHAL(sp_marshal sp-marshal "${CMAKE_CURRENT_BINARY_DIR}/helper")

set(sp_marshal_SRC
	${CMAKE_CURRENT_BINARY_DIR}/sp-marshal.cpp
	${CMAKE_CURRENT_BINARY_DIR}/sp-marshal.h
)

set(helper_SRC
	action.cpp
  action-context.cpp
	geom.cpp
	geom-nodetype.cpp
	geom-pathstroke.cpp
	gnome-utils.cpp
	pixbuf-ops.cpp
	png-write.cpp
	stock-items.cpp
	#units-test.cpp
	window.cpp

	# we generate this file and it's .h counter-part
	${sp_marshal_SRC}


	# -------
	# Headers
	action.h
  action-context.h
	geom-curves.h
	geom-nodetype.h
	geom-pathstroke.h
	geom.h
	gnome-utils.h
	mathfns.h
	pixbuf-ops.h
	png-write.h
	stock-items.h
	window.h
)

set_source_files_properties(sp_marshal_SRC PROPERTIES GENERATED true)

# add_inkscape_lib(helper_LIB "${helper_SRC}")
add_inkscape_source("${helper_SRC}")
