set(libvpsc_SRC
	block.cpp
	blocks.cpp
	constraint.cpp
	csolve_VPSC.cpp
	generate-constraints.cpp
	remove_rectangle_overlap.cpp
	solve_VPSC.cpp
	variable.cpp
	pairingheap/PairingHeap.cpp
)

add_library(vpsc_LIB STATIC ${libvpsc_SRC})
target_link_libraries(vpsc_LIB ${INKSCAPE_LIBS})
