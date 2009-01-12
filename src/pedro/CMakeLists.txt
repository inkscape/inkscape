SET(pedro_SRC
#empty.cpp
#geckoembed.cpp
pedroconfig.cpp
pedrodom.cpp
#pedrogui.cpp
#pedromain.cpp
pedroutil.cpp
pedroxmpp.cpp
)
ADD_LIBRARY(pedro STATIC ${pedro_SRC})
TARGET_LINK_LIBRARIES(pedro
${INKSCAPE_LIBS})