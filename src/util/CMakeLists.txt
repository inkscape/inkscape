
set(util_SRC
	ege-appear-time-tracker.cpp
	ege-tags.cpp
	expression-evaluator.cpp
	share.cpp
	units.cpp
)

# add_library(util_LIB ${util_SRC})
add_inkscape_source("${util_SRC}")
