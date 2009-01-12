SET(extfolders
#dxf2svg
implementation
internal
internal/bitmap
internal/filter
internal/pdfinput
param
script
)

FOREACH(extlistsrc ${extfolders})
  ADD_SUBDIRECTORY(${extlistsrc})
ENDFOREACH(extlistsrc)

SET(extension_SRC
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
ADD_LIBRARY(extension STATIC ${extension_SRC})
TARGET_LINK_LIBRARIES(extension
2geom ${INKSCAPE_LIBS})