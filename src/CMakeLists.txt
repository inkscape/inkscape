
set(sp_SRC
	sp-anchor.cpp
	# sp-animation.cpp
	sp-clippath.cpp
	sp-conn-end-pair.cpp
	sp-conn-end.cpp
	sp-cursor.cpp
	sp-defs.cpp
	sp-desc.cpp
	sp-ellipse.cpp
	sp-filter-primitive.cpp
	sp-filter-reference.cpp
	sp-filter.cpp
	sp-flowdiv.cpp
	sp-flowregion.cpp
	sp-flowtext.cpp
	sp-font-face.cpp
	sp-font.cpp
	sp-gaussian-blur.cpp
	sp-glyph-kerning.cpp
	sp-glyph.cpp
	sp-gradient-reference.cpp
	sp-gradient.cpp
	sp-guide.cpp
	sp-image.cpp
	sp-item-group.cpp
	sp-item-notify-moveto.cpp
	sp-item-rm-unsatisfied-cns.cpp
	sp-item-transform.cpp
	sp-item-update-cns.cpp
	sp-item.cpp
	sp-line.cpp
	sp-lpe-item.cpp
	sp-mask.cpp
	sp-metadata.cpp
	sp-metrics.cpp
	sp-missing-glyph.cpp
	sp-namedview.cpp
	sp-object-group.cpp
	sp-object-repr.cpp
	sp-object.cpp
	sp-offset.cpp
	sp-paint-server.cpp
	sp-path.cpp
	sp-pattern.cpp
	sp-polygon.cpp
	sp-polyline.cpp
	sp-rect.cpp
	sp-root.cpp
	sp-script.cpp
	sp-shape.cpp
	# sp-skeleton.cpp
	sp-spiral.cpp
	sp-star.cpp
	sp-stop.cpp
	sp-string.cpp
	sp-style-elem.cpp
	sp-switch.cpp
	sp-symbol.cpp
	sp-text.cpp
	sp-title.cpp
	sp-tref-reference.cpp
	sp-tref.cpp
	sp-tspan.cpp
	sp-use-reference.cpp
	sp-use.cpp
	spiral-context.cpp
	splivarot.cpp
)

set(inkscape_SRC
	arc-context.cpp
	attributes.cpp
	axis-manip.cpp
	box3d-context.cpp
	box3d-side.cpp
	box3d.cpp
	color-profile.cpp
	color.cpp
	common-context.cpp
	composite-undo-stack-observer.cpp
	conditions.cpp
	conn-avoid-ref.cpp
	connection-points.cpp
	connector-context.cpp
	console-output-undo-observer.cpp
	context-fns.cpp
	desktop-events.cpp
	desktop-handles.cpp
	desktop-style.cpp
	desktop.cpp
	device-manager.cpp
	dir-util.cpp
	document-subset.cpp
	document-undo.cpp
	document.cpp
	doxygen-main.cpp
	draw-anchor.cpp
	draw-context.cpp
	dropper-context.cpp
	dyna-draw-context.cpp
	ege-adjustment-action.cpp
	ege-color-prof-tracker.cpp
	ege-output-action.cpp
	ege-select-one-action.cpp
	eraser-context.cpp
	event-context.cpp
	event-log.cpp
	extract-uri.cpp
	file.cpp
	filter-chemistry.cpp
	filter-enums.cpp
	fixes.cpp
	flood-context.cpp
	gc-anchored.cpp
	gc-finalized.cpp
	gc.cpp
	gradient-chemistry.cpp
	gradient-context.cpp
	gradient-drag.cpp
	graphlayout.cpp
	guide-snapper.cpp
	help.cpp
	id-clash.cpp
	ige-mac-menu.c
	ink-action.cpp
	ink-comboboxentry-action.cpp
	inkscape-version.cpp
	inkscape.cpp
	inkscape.rc
	# inkview.cpp
	# inkview.rc
	interface.cpp
	knot-holder-entity.cpp
	knot.cpp
	knotholder.cpp
	layer-fns.cpp
	layer-manager.cpp
	line-geometry.cpp
	line-snapper.cpp
	lpe-tool-context.cpp
	main-cmdlineact.cpp
	main.cpp
	marker.cpp
	measure-context.cpp
	media.cpp
	message-context.cpp
	message-stack.cpp
	mod360.cpp
	object-edit.cpp
	object-hierarchy.cpp
	object-snapper.cpp
	path-chemistry.cpp
	pen-context.cpp
	pencil-context.cpp
	persp3d-reference.cpp
	persp3d.cpp
	perspective-line.cpp
	preferences.cpp
	prefix.cpp
	print.cpp
	profile-manager.cpp
	proj_pt.cpp
	rdf.cpp
	rect-context.cpp
	removeoverlap.cpp
	resource-manager.cpp
	rubberband.cpp
	satisfied-guide-cns.cpp
	selcue.cpp
	select-context.cpp
	selection-chemistry.cpp
	selection-describer.cpp
	selection.cpp
	seltrans-handles.cpp
	seltrans.cpp
	shape-editor.cpp
	shortcuts.cpp
	snap-preferences.cpp
	snap.cpp
	snapped-curve.cpp
	snapped-line.cpp
	snapped-point.cpp
	snapper.cpp
	spray-context.cpp
	star-context.cpp
	style.cpp
	svg-view-widget.cpp
	svg-view.cpp
	text-chemistry.cpp
	text-context.cpp
	text-editing.cpp
	tools-switch.cpp
	transf_mat_3x4.cpp
	tweak-context.cpp
	unclump.cpp
	unicoderange.cpp
	uri-references.cpp
	uri.cpp
	vanishing-point.cpp
	verbs.cpp
	version.cpp
	zoom-context.cpp
)

if(WIN32)
    list(APPEND inkscape_SRC
		registrytool.cpp
		#deptool.cpp
		winmain.cpp
	)
endif()

# All folders for internal inkscape
set(internalfolders
	#algorithms
	#api
	bind
	debug
	dialogs
	display
	dom
	extension
	filters
	helper
	io
	# jabber_whiteboard
	live_effects
	# pedro
	svg
	trace
	#traits
	ui
	util
	widgets
	xml
	2geom
)

set(libfolders
	# Directories containing lists files that describe building internal libraries
	libavoid
	libcola
	libcroco
	libgdl
	libnr
	libnrtype
	libvpsc
	livarot
)

set(dirs
	${internalfolders}
	${libfolders}
)

foreach(srclistsrc ${dirs})
	add_subdirectory(${srclistsrc})
endforeach()

set(inkscape_SRC
	${inkscape_SRC}
	${GlibOutput}
)

add_library(sp_LIB ${sp_SRC})

# make executable for INKSCAPE
add_executable(inkscape ${inkscape_SRC})

target_link_libraries(inkscape
	# order from automake 
	dom_LIB
	croco_LIB
	avoid_LIB
	gdl_LIB
	cola_LIB
	vpsc_LIB
	livarot_LIB
	2geom_LIB

	# guessing these ones
	ui_LIB
	bind_LIB
	debug_LIB
	dialogs_LIB
	display_LIB
	extension_LIB
	filters_LIB
	helper_LIB
	io_LIB
	nr_LIB
	nrtype_LIB
	sp_LIB
	svg_LIB
	trace_LIB
	util_LIB
	widgets_LIB
	xml_LIB
	live_effects_LIB

	${INKSCAPE_LIBS}
	
	
	# system libs
	-lxslt
	-lgtkspell
	-lgsl
	-lgslcblas
	-lgtkmm-2.4
	-latkmm-1.6
	-lgdkmm-2.4
	-lgiomm-2.4
	-lpangomm-1.4
	-lgtk-x11-2.0
	-lglibmm-2.4
	-lcairomm-1.0
	-lsigc-2.0
	-latk-1.0
	-lgio-2.0
	-lpng
	-lX11
	-lxml2
	-ldl
	-lgomp
	-lpopt
	-laspell
	-lgnomevfs-2
	-lgconf-2
	-lpangoft2-1.0
	-lfontconfig
	-lfreetype
	-lz
	-lMagick++
	-lMagickCore
	-lgc
	-llcms
	-lpoppler-glib
	-lgdk-x11-2.0
	-lpoppler
	-lpangocairo-1.0
	-lgdk_pixbuf-2.0
	-lpng14
	-lm
	-lpango-1.0
	-lcairo
	-lgmodule-2.0
	-lgobject-2.0
	-lgthread-2.0
	-lrt
	-lglib-2.0
	
)

# make executable for INKVIEW
#ADD_EXECUTABLE(inkview inkview.cpp)
#TARGET_LINK_LIBRARIES(inkview
#	2geom_LIB
#	avoid_LIB
#	cola_LIB
#	croco_LIB
#	gdl_LIB
#	nr_LIB
#	nrtype_LIB
#	vpsc_LIB
#	livarot_LIB
#	sp_LIB
#)

