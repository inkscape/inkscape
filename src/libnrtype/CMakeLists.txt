
set(nrtype_SRC
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

	FontFactory.h
	Layout-TNG-Scanline-Maker.h
	Layout-TNG.h
	RasterFont.h
	TextWrapper.h
	boundary-type.h
	font-glyph.h
	font-instance.h
	font-lister.h
	font-style-to-pos.h
	font-style.h
	nr-type-pos-def.h
	nr-type-primitives.h
	nrtype-forward.h
	one-box.h
	one-glyph.h
	one-para.h
	raster-glyph.h
	raster-position.h
	text-boundary.h
)

add_inkscape_lib(nrtype_LIB "${nrtype_SRC}")
