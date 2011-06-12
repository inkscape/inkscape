
set(util_SRC
	ege-appear-time-tracker.cpp
	ege-tags.cpp
	expression-evaluator.cpp
	share.cpp
	units.cpp
)

add_library(util STATIC ${util_SRC})
target_link_libraries(util 2geom ${INKSCAPE_LIBS})
