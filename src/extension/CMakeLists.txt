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
parambool.cpp
paramcolor.cpp
paramdescription.cpp
paramenum.cpp
parameter.cpp
paramfloat.cpp
paramint.cpp
paramnotebook.cpp
paramradiobutton.cpp
paramstring.cpp
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
script
)

FOREACH(extensiondirlist ${extensionfolders})
ADD_SUBDIRECTORY(${extensiondirlist})
ENDFOREACH(extensiondirlist)

