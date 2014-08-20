
set(libvpsc_SRC
	block.cpp
	blocks.cpp
	constraint.cpp
	generate-constraints.cpp
	remove_rectangle_overlap.cpp
	solve_VPSC.cpp
	variable.cpp
	pairingheap/PairingHeap.cpp


	# -------
	# Headers
	block.h
	blocks.h
	constraint.h
	generate-constraints.h
	pairingheap/PairingHeap.h
	pairingheap/dsexceptions.h
	placement_SolveVPSC.h
	remove_rectangle_overlap.h
	solve_VPSC.h
	variable.h
)

add_inkscape_lib(vpsc_LIB "${libvpsc_SRC}")
