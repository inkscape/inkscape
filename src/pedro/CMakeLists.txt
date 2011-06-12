set(pedro_SRC
	#empty.cpp
	#geckoembed.cpp
	pedroconfig.cpp
	pedrodom.cpp
	#pedrogui.cpp
	#pedromain.cpp
	pedroutil.cpp
	pedroxmpp.cpp
)
add_library(pedro STATIC ${pedro_SRC})
target_link_libraries(pedro ${INKSCAPE_LIBS})