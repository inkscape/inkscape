
set(extension_SRC
	db.cpp
	dependency.cpp
	effect.cpp
	error-file.cpp
	execution-env.cpp
	extension.cpp
	init.cpp
	input.cpp
	output.cpp
	patheffect.cpp
	prefdialog.cpp
	print.cpp
	system.cpp
	timer.cpp

	implementation/implementation.cpp
	implementation/xslt.cpp
	implementation/script.cpp

	param/bool.cpp
	param/color.cpp
	param/description.cpp
	param/enum.cpp
	param/float.cpp
	param/int.cpp
	param/notebook.cpp
	param/parameter.cpp
	param/radiobutton.cpp
	param/string.cpp
	
	internal/bluredge.cpp
	internal/cairo-png-out.cpp
	internal/cairo-ps-out.cpp
	# internal/cairo-render-context.cpp  # XXX MUST GET THIS WORKING
	# internal/cairo-renderer.cpp  # XXX MUST GET THIS WORKING
	internal/cairo-renderer-pdf-out.cpp
	internal/emf-win32-inout.cpp
	internal/emf-win32-print.cpp
	internal/gdkpixbuf-input.cpp
	internal/gimpgrad.cpp
	internal/grid.cpp
	internal/latex-pstricks.cpp
	# internal/latex-pstricks-out.cpp  # XXX MUST GET THIS WORKING
	internal/odf.cpp
	internal/latex-text-renderer.cpp
	internal/pdf-input-cairo.cpp
	internal/pov-out.cpp
	internal/javafx-out.cpp
	internal/svg.cpp
	internal/svgz.cpp
	internal/wpg-input.cpp

	internal/bitmap/adaptiveThreshold.cpp
	internal/bitmap/addNoise.cpp
	internal/bitmap/blur.cpp
	internal/bitmap/channel.cpp
	internal/bitmap/charcoal.cpp
	internal/bitmap/colorize.cpp
	internal/bitmap/contrast.cpp
	internal/bitmap/cycleColormap.cpp
	internal/bitmap/despeckle.cpp
	internal/bitmap/edge.cpp
	internal/bitmap/emboss.cpp
	internal/bitmap/enhance.cpp
	internal/bitmap/equalize.cpp
	internal/bitmap/gaussianBlur.cpp
	internal/bitmap/imagemagick.cpp
	internal/bitmap/implode.cpp
	internal/bitmap/level.cpp
	internal/bitmap/levelChannel.cpp
	internal/bitmap/medianFilter.cpp
	internal/bitmap/modulate.cpp
	internal/bitmap/negate.cpp
	internal/bitmap/normalize.cpp
	internal/bitmap/oilPaint.cpp
	internal/bitmap/opacity.cpp
	internal/bitmap/raise.cpp
	internal/bitmap/reduceNoise.cpp
	internal/bitmap/sample.cpp
	internal/bitmap/shade.cpp
	internal/bitmap/sharpen.cpp
	internal/bitmap/solarize.cpp
	internal/bitmap/spread.cpp
	internal/bitmap/swirl.cpp
	internal/bitmap/threshold.cpp
	internal/bitmap/unsharpmask.cpp
	internal/bitmap/wave.cpp

	internal/filter/filter-all.cpp
	internal/filter/filter-file.cpp
	internal/filter/filter.cpp

	internal/pdfinput/pdf-input.cpp
	internal/pdfinput/pdf-parser.cpp
	internal/pdfinput/svg-builder.cpp

	script/InkscapeScript.cpp

	# dxf2svg/aci2rgb.cpp
	# dxf2svg/entities2elements.cpp
	# dxf2svg/tables2svg_info.cpp
	# dxf2svg/blocks.cpp
	# dxf2svg/entities.cpp
	# dxf2svg/tables.cpp
	# dxf2svg/dxf2svg.cpp
	# dxf2svg/read_dxf.cpp
	# dxf2svg/test_dxf.cpp
)

if(WIN32)
	list(APPEND extension_SRC
		win32.cpp
	)
endif()

add_library(extension_LIB ${extension_SRC})
