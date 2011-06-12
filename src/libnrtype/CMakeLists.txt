set(libnrtype_SRC
	FontFactory.cpp
	FontInstance.cpp
	font-lister.cpp
	font-style-to-pos.cpp
	Layout-TNG.cpp
	Layout-TNG-Compute.cpp
	Layout-TNG-Input.cpp
	Layout-TNG-OutIter.cpp
	Layout-TNG-Output.cpp
	Layout-TNG-Scanline-Makers.cpp
	nr-type-pos-def.cpp
	nr-type-primitives.cpp
	RasterFont.cpp
	TextWrapper.cpp
)
add_library(nrtype_LIB STATIC ${libnrtype_SRC})
target_link_libraries(nrtype_LIB nr_LIB ${INKSCAPE_LIBS})
