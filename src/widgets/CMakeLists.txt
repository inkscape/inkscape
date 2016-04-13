
set(widgets_SRC
	button.cpp
	  arc-toolbar.cpp
	  box3d-toolbar.cpp
	  calligraphy-toolbar.cpp
	  connector-toolbar.cpp
	  dropper-toolbar.cpp
	  eraser-toolbar.cpp
	  lpe-toolbar.cpp
	  measure-toolbar.cpp
	  mesh-toolbar.cpp
	  node-toolbar.cpp
	  pencil-toolbar.cpp
	  rect-toolbar.cpp
	  spiral-toolbar.cpp
	  spray-toolbar.cpp
	  star-toolbar.cpp
	  text-toolbar.cpp
	  tweak-toolbar.cpp
	  zoom-toolbar.cpp		
	dash-selector.cpp
	desktop-widget.cpp
	eek-preview.cpp
	ege-paint-def.cpp
        ege-adjustment-action.cpp
        ege-output-action.cpp
        ege-select-one-action.cpp
	fill-style.cpp
	font-selector.cpp
	gradient-image.cpp
	gradient-selector.cpp
	gradient-toolbar.cpp
	gradient-vector.cpp
	icon.cpp
	ink-action.cpp
	ink-comboboxentry-action.cpp
	paint-selector.cpp
	ruler.cpp
	select-toolbar.cpp
	sp-attribute-widget.cpp
	sp-color-selector.cpp
	sp-widget.cpp
	sp-xmlview-attr-list.cpp
	sp-xmlview-content.cpp
	sp-xmlview-tree.cpp
	spinbutton-events.cpp
	spw-utilities.cpp
	stroke-marker-selector.cpp
	stroke-style.cpp
	swatch-selector.cpp
	toolbox.cpp


	# -------
	# Headers
	button.h
	  arc-toolbar.h
	  box3d-toolbar.h
	  calligraphy-toolbar.h
	  connector-toolbar.h
	  dropper-toolbar.h
	  eraser-toolbar.h
	  lpe-toolbar.h
	  measure-toolbar.h
	  mesh-toolbar.h
	  node-toolbar.h
	  pencil-toolbar.h
	  rect-toolbar.h
	  spiral-toolbar.h
	  spray-toolbar.h
	  star-toolbar.h
	  text-toolbar.h
	  tweak-toolbar.h
	  zoom-toolbar.h	
	dash-selector.h
	desktop-widget.h
	eek-preview.h
	ege-paint-def.h
        ege-adjustment-action.h
        ege-output-action.h
        ege-select-one-action.h
	fill-n-stroke-factory.h
	fill-style.h
	font-selector.h
	gradient-image.h
	gradient-selector.h
	gradient-toolbar.h
	gradient-vector.h
	icon.h
        ink-action.h
        ink-comboboxentry-action.h
	paint-selector.h
	ruler.h
	select-toolbar.h
	sp-attribute-widget.h
	sp-color-selector.h
	sp-widget.h
	sp-xmlview-attr-list.h
	sp-xmlview-content.h
	sp-xmlview-tree.h
	spinbutton-events.h
	spw-utilities.h
	stroke-marker-selector.h
	stroke-style.h
	swatch-selector.h
	toolbox.h
	widget-sizes.h
)

if(${WITH_GTK3_EXPERIMENTAL})
    set(image_menu_item_SRC image-menu-item.h image-menu-item.c)
    add_inkscape_source("${image_menu_item_SRC}")
endif()

# add_inkscape_lib(widgets_LIB "${widgets_SRC}")
add_inkscape_source("${widgets_SRC}")

set ( widgets_paintbucket_SRC
	  paintbucket-toolbar.cpp
	  paintbucket-toolbar.h
)

if ("${HAVE_POTRACE}")
	add_inkscape_source("${widgets_paintbucket_SRC}")
endif()



