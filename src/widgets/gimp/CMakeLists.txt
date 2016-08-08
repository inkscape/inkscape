set(gimpwidgets_SRC
	gimpcolorwheel.c
	gimpspinscale.c
	ruler.cpp

	# -------
	# Headers
	gimpcolorwheel.h
	gimpspinscale.h
	ruler.h
)

add_inkscape_source("${gimpwidgets_SRC}")
