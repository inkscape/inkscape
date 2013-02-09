
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
	# xmlwriter.cpp
	xpathimpl.cpp
	xpathparser.cpp
	xpathtoken.cpp

	io/base64stream.cpp
	io/bufferstream.cpp
	io/domstream.cpp
	io/gzipstream.cpp
	io/stringstream.cpp
	io/uristream.cpp

	util/digest.cpp
	util/ziptool.cpp


	# -------
	# Headers
	css.h
	cssreader.h
	dom.h
	domimpl.h
	domptr.h
	domstring.h
	events.h
	ls.h
	lsimpl.h
	smil.h
	smilimpl.h
	stylesheets.h
	svg.h
	svg2.h
	svgimpl.h
	svgreader.h
	svgtypes.h
	traversal.h
	ucd.h
	uri.h
	views-level3.h
	views.h
	xmlreader.h
	xmlwriter.h
	xpath.h
	xpathimpl.h
	xpathparser.h
	xpathtoken.h

	io/base64stream.h
	io/bufferstream.h
	io/domstream.h
	io/gzipstream.h
	io/stringstream.h
	io/uristream.h

	util/digest.h
	util/ziptool.h
)

add_inkscape_lib(dom_LIB "${dom_SRC}")
