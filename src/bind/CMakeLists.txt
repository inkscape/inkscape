set(bind_SRC
	dobinding.cpp
	javabind.cpp
)
add_library(bind STATIC ${bind_SRC})
target_link_libraries(bind ${INKSCAPE_LIBS})
