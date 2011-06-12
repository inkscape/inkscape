set(extfolders
	#dxf2svg
	implementation
	internal
	internal/bitmap
	internal/filter
	internal/pdfinput
	param
	script
)

foreach(extlistsrc ${extfolders})
  add_subdirectory(${extlistsrc})
endforeach()

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
	#${extension_dxf2svg_SRC}
	${extension_implementation_SRC}
	${extension_internal_bitmap_SRC}
	${extension_internal_filter_SRC}
	${extension_internal_pdfinput_SRC}
	${extension_param_SRC}
	${extension_script_SRC}
)

add_library(extension STATIC ${extension_SRC})
target_link_libraries(extension 2geom ${INKSCAPE_LIBS})
