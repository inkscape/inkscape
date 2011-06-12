set(bind_SRC
	dobinding.cpp
	javabind.cpp
)
add_library(bind_LIB STATIC ${bind_SRC})
target_link_libraries(bind_LIB ${INKSCAPE_LIBS})
