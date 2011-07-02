
set(widgets_SRC
	button.cpp
	dash-selector.cpp
	desktop-widget.cpp
	eek-preview.cpp
	ege-paint-def.cpp
	fill-style.cpp
	font-selector.cpp
	gradient-image.cpp
	gradient-selector.cpp
	gradient-toolbar.cpp
	gradient-vector.cpp
	icon.cpp
	paint-selector.cpp
	ruler.cpp
	select-toolbar.cpp
	shrink-wrap-button.cpp
	sp-attribute-widget.cpp
	sp-color-gtkselector.cpp
	sp-color-icc-selector.cpp
	sp-color-notebook.cpp
	sp-color-scales.cpp
	sp-color-selector.cpp
	sp-color-slider.cpp
	sp-color-wheel-selector.cpp
	sp-widget.cpp
	sp-xmlview-attr-list.cpp
	sp-xmlview-content.cpp
	sp-xmlview-tree.cpp
	spinbutton-events.cpp
	spw-utilities.cpp
	stroke-style.cpp
	swatch-selector.cpp
	toolbox.cpp


	# -------
	# Headers
	button.h
	dash-selector.h
	desktop-widget.h
	eek-preview.h
	ege-paint-def.h
	fill-n-stroke-factory.h
	fill-style.h
	font-selector.h
	gradient-image.h
	gradient-selector.h
	gradient-toolbar.h
	gradient-vector.h
	icon.h
	paint-selector.h
	ruler.h
	select-toolbar.h
	shrink-wrap-button.h
	sp-attribute-widget.h
	sp-color-gtkselector.h
	sp-color-icc-selector.h
	sp-color-notebook.h
	sp-color-scales.h
	sp-color-selector.h
	sp-color-slider.h
	sp-color-wheel-selector.h
	sp-widget.h
	sp-xmlview-attr-list.h
	sp-xmlview-content.h
	sp-xmlview-tree.h
	spinbutton-events.h
	spw-utilities.h
	stroke-style.h
	swatch-selector.h
	toolbox.h
	widget-sizes.h
)

# add_inkscape_lib(widgets_LIB "${widgets_SRC}")
add_inkscape_source("${widgets_SRC}")
