SET(libvpsc_SRC
block.cpp
blocks.cpp
constraint.cpp
csolve_VPSC.cpp
generate-constraints.cpp
remove_rectangle_overlap.cpp
solve_VPSC.cpp
variable.cpp
)
ADD_LIBRARY(vpsc STATIC ${libvpsc_SRC})
TARGET_LINK_LIBRARIES(vpsc
${INKSCAPE_LIBS})