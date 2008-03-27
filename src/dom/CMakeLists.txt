SET(dom_SRC
cssparser.cpp
domimpl.cpp
domptr.cpp
domstring.cpp
lsimpl.cpp
minidom.cpp
prop-css2.cpp
prop-css.cpp
prop-svg.cpp
smilimpl.cpp
svgimpl.cpp
svgparser.cpp
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
${dom_work_SRC}
)
# Add New folders in src folder here
SET(domfolders
io
odf
util
work
)

FOREACH(domdirlist ${domfolders})
ADD_SUBDIRECTORY(${domdirlist})
ENDFOREACH(domdirlist)

