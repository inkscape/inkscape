add_subdirectory(parameter)

set(live_effects_SRC
	bezctx.cpp
	effect.cpp
	lpe-angle_bisector.cpp
	lpe-bendpath.cpp
	lpe-boolops.cpp
	lpe-circle_3pts.cpp
	lpe-circle_with_radius.cpp
	lpe-constructgrid.cpp
	lpe-copy_rotate.cpp
	lpe-curvestitch.cpp
	lpe-dynastroke.cpp
	lpe-envelope.cpp
	lpe-extrude.cpp
	lpe-gears.cpp
	lpe-interpolate.cpp
	lpe-knot.cpp
	lpe-lattice.cpp
	lpe-line_segment.cpp
	lpe-mirror_symmetry.cpp
	lpe-offset.cpp
	lpe-parallel.cpp
	lpe-path_length.cpp
	lpe-patternalongpath.cpp
	lpe-perp_bisector.cpp
	lpe-perspective_path.cpp
	lpe-powerstroke.cpp
	lpe-recursiveskeleton.cpp
	lpe-rough-hatches.cpp
	lpe-ruler.cpp
	# lpe-skeleton.cpp
	lpe-sketch.cpp
	lpe-spiro.cpp
	lpe-tangent_to_curve.cpp
	lpe-test-doEffect-stack.cpp
	lpe-text_label.cpp
	lpe-vonkoch.cpp
	lpegroupbbox.cpp
	lpeobject-reference.cpp
	lpeobject.cpp
	spiro.cpp
	${live_effects_parameter_SRC}
)

add_library(live_effects STATIC ${live_effects_SRC})
target_link_libraries(live_effects 2geom ${INKSCAPE_LIBS})
