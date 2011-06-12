set(domfolders
	io
	odf
	util
	#work
)

foreach(domlistsrc ${domfolders})
	add_subdirectory(${domlistsrc})
endforeach()

set(dom_SRC
	cssreader.cpp
	domimpl.cpp
	domptr.cpp
	domstring.cpp
	lsimpl.cpp
	prop-css2.cpp
	prop-css.cpp
	prop-svg.cpp
	smilimpl.cpp
	svgimpl.cpp
	svgreader.cpp
	ucd.cpp
	uri.cpp
	xmlreader.cpp
	xmlwriter.cpp
	xpathimpl.cpp
	xpathparser.cpp
	xpathtoken.cpp
	${dom_io_SRC}
	${dom_odf_SRC}
	${dom_util_SRC}
	#${dom_work_SRC}
)

add_library(dom STATIC ${dom_SRC})
target_link_libraries(dom 2geom ${INKSCAPE_LIBS})