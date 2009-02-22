SET(application_SRC
editor.cpp
application.cpp
app-prototype.cpp
)
ADD_LIBRARY(application STATIC ${application_SRC})
TARGET_LINK_LIBRARIES(application
2geom ${INKSCAPE_LIBS})