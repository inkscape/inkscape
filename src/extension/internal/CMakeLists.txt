SET(extension_internal_SRC
bluredge.cpp
cairo-pdf-out.cpp
cairo-png-out.cpp
cairo-ps-out.cpp
cairo-render-context.cpp
cairo-renderer.cpp
cairo-renderer-pdf-out.cpp
emf-win32-inout.cpp
emf-win32-print.cpp
eps-out.cpp
gdkpixbuf-input.cpp
gimpgrad.cpp
grid.cpp
latex-pstricks.cpp
latex-pstricks-out.cpp
odf.cpp
pdf-cairo.cpp
pdf-input-cairo.cpp
pov-out.cpp
ps.cpp
ps-out.cpp
svg.cpp
svgz.cpp
win32.cpp
wpg-input.cpp
${extension_internal_bitmap_SRC}
${extension_internal_pdfinput_SRC}
)

# Add New folders in src folder here
SET(extensioninternalfolders
bitmap
pdfinput
)

FOREACH(extensionINTdirlist ${extensioninternalfolders})
ADD_SUBDIRECTORY(${extensionINTdirlist})
ENDFOREACH(extensionINTdirlist)

