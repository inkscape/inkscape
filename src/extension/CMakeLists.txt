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
${extension_dxf2svg_SRC}
${extension_implementation_SRC}
${extension_internal_SRC}
${extension_script_SRC}
)

# Add New folders in src folder here
SET(extensionfolders
dxf2svg
implementation
internal
param
script
)

FOREACH(extensiondirlist ${extensionfolders})
ADD_SUBDIRECTORY(${extensiondirlist})
ENDFOREACH(extensiondirlist)

