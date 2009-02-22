SET(io_SRC
base64stream.cpp
ftos.cpp
gzipstream.cpp
inkscapestream.cpp
resource.cpp
simple-sax.cpp
#streamtest.cpp
stringstream.cpp
sys.cpp
uristream.cpp
xsltstream.cpp
)
ADD_LIBRARY(io STATIC ${io_SRC})
TARGET_LINK_LIBRARIES(io
2geom ${INKSCAPE_LIBS})