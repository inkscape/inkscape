
set(livarot_SRC
	AlphaLigne.cpp
	AVL.cpp
	BitLigne.cpp
	float-line.cpp
	int-line.cpp
	PathConversion.cpp
	Path.cpp
	PathCutting.cpp
	path-description.cpp
	PathOutline.cpp
	PathSimplify.cpp
	PathStroke.cpp
	Shape.cpp
	ShapeDraw.cpp
	ShapeMisc.cpp
	ShapeRaster.cpp
	ShapeSweep.cpp
	sweep-event.cpp
	sweep-tree.cpp
	sweep-tree-list.cpp


	# -------
	# Headers
	AVL.h
	AlphaLigne.h
	BitLigne.h
	Livarot.h
	LivarotDefs.h
	Path.h
	Shape.h
	float-line.h
	int-line.h
	path-description.h
	sweep-event-queue.h
	sweep-event.h
	sweep-tree-list.h
	sweep-tree.h
)

add_inkscape_lib(livarot_LIB "${livarot_SRC}")
