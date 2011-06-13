
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

# add_library(pedro_LIB ${pedro_SRC})
add_inkscape_source("${pedro_SRC}")
