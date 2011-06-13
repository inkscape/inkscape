
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
	# io/httpclient.cpp
	io/socket.cpp
	io/stringstream.cpp
	io/uristream.cpp

	odf/odfdocument.cpp
	#odf/SvgOdg.cpp

	util/digest.cpp
	util/thread.cpp
	util/ziptool.cpp

	# # Dont use any of them.
	# work/svg2.cpp
	# work/testdom.cpp
	# work/testhttp.cpp
	# work/testjs.cpp
	# work/testodf.cpp
	# work/testsvg.cpp
	# work/testuri.cpp
	# work/testxpath.cpp
	# work/testzip.cpp
	# work/xpathtests.cpp

)

add_library(dom_LIB ${dom_SRC})
