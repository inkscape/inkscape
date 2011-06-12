
set(util_SRC
	ege-appear-time-tracker.cpp
	ege-tags.cpp
	expression-evaluator.cpp
	share.cpp
	units.cpp
)

add_library(util_LIB STATIC ${util_SRC})
target_link_libraries(util_LIB 2geom_LIB ${INKSCAPE_LIBS})
