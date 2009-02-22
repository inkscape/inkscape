SET(bind_SRC
dobinding.cpp
javabind.cpp
)
ADD_LIBRARY(bind STATIC ${bind_SRC})
TARGET_LINK_LIBRARIES(bind
${INKSCAPE_LIBS})