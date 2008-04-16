SET(INKSCAPE_SRC)

# A macro to replace slashes and spaces in a string with underscores
MACRO(SANITIZE_PATH _string_var)
  STRING(REGEX REPLACE "[\\/ ]+" "_" ${_string_var} ${${_string_var}})
ENDMACRO(SANITIZE_PATH _string_var)

# A macro to prepend a given string onto the beginning of each string in a list
MACRO(PREPEND _list _str)
  SET(_temp_list ${${_list}})
  SET(${_list})
  FOREACH(x ${_temp_list})
    SET(${_list} ${${_list}} ${_str}${x})
  ENDFOREACH(x)
ENDMACRO(PREPEND _list _str)

# All folder with CMakeLists.txt files (excluding internal libraries)
SET(srcfolders
algorithms
api
bind
debug
display
dom
dom/io
dom/odf
dom/util
#dom/work
extension
#extension/dxf2svg
extension/implementation
extension/internal
extension/param
graphlayout
helper
inkjar
io
jabber_whiteboard
jabber_whiteboard/dialog
live_effects
live_effects/parameter
#pedro
removeoverlap
svg
trace
trace/potrace
traits
ui/cache
ui
ui/dialog
ui/view
ui/widget
#utest
util
widgets
xml
)

# 1) include each list file
# 2) grab the variable inside 
# 3) prepend the full relative path to each file in list
# 4) append the list to INKSCAPE_SRC
FOREACH(dirlist ${srcfolders})
  SET(_temp_dirlist "${dirlist}")
  MESSAGE(STATUS "Processing ${CMAKE_CURRENT_SOURCE_DIR}/${dirlist}/CMakeLists.txt")
  INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/${dirlist}/CMakeLists.txt)
  SANITIZE_PATH(dirlist)
  PREPEND(${dirlist}_SRC "${_temp_dirlist}/")
  SET(INKSCAPE_SRC ${INKSCAPE_SRC} ${${dirlist}_SRC})
ENDFOREACH(dirlist)


# All directories containing lists files that describe building internal libraries
SET(libfolders
2geom
application
dialogs
libavoid
libcola
libcroco
libgdl
libnr
libnrtype
libvpsc
)

FOREACH(dirlist ${libfolders})
  ADD_SUBDIRECTORY(${dirlist})
ENDFOREACH(dirlist)

IF(WIN32)
SET(ONLY_WIN
registrytool.cpp
#deptool.cpp
)
ELSEIF(WIN32)
SET(ONLY_WIN)
ENDIF(WIN32)

SET(INKSCAPE_SRC
${INKSCAPE_SRC}
arc-context.cpp
attributes.cpp
axis-manip.cpp
box3d-context.cpp
box3d.cpp
box3d-side.cpp
color.cpp
color-profile.cpp
composite-undo-stack-observer.cpp
conditions.cpp
conn-avoid-ref.cpp
connector-context.cpp
console-output-undo-observer.cpp
context-fns.cpp
desktop-affine.cpp
desktop.cpp
desktop-events.cpp
desktop-handles.cpp
desktop-style.cpp
dir-util.cpp
document.cpp
document-subset.cpp
document-undo.cpp
doxygen-main.cpp
draw-anchor.cpp
draw-context.cpp
dropper-context.cpp
dyna-draw-context.cpp
ege-adjustment-action.cpp
ege-color-prof-tracker.cpp
ege-output-action.cpp
ege-select-one-action.cpp
event-context.cpp
event-log.cpp
extract-uri.cpp
file.cpp
filter-chemistry.cpp
filter-enums.cpp
fixes.cpp
flood-context.cpp
fontsize-expansion.cpp
gc-anchored.cpp
gc.cpp
gc-finalized.cpp
gradient-chemistry.cpp
gradient-context.cpp
gradient-drag.cpp
guide-snapper.cpp
help.cpp
ink-action.cpp
inkscape.cpp
inkscape-stock.cpp
##inkview is another executable file
##needs to be separated as a second target
#inkview.cpp
interface.cpp
knot.cpp
knotholder.cpp
layer-fns.cpp
layer-manager.cpp
line-geometry.cpp
line-snapper.cpp
main-cmdlineact.cpp
main.cpp
marker.cpp
media.cpp
message-context.cpp
message-stack.cpp
mod360.cpp
node-context.cpp
nodepath.cpp
object-edit.cpp
object-hierarchy.cpp
object-snapper.cpp
path-chemistry.cpp
pencil-context.cpp
pen-context.cpp
persp3d.cpp
persp3d-reference.cpp
perspective-line.cpp
preferences.cpp
prefix.cpp
prefs-utils.cpp
print.cpp
profile-manager.cpp
proj_pt.cpp
rect-context.cpp
rubberband.cpp
satisfied-guide-cns.cpp
selcue.cpp
select-context.cpp
selection-chemistry.cpp
selection.cpp
selection-describer.cpp
seltrans.cpp
seltrans-handles.cpp
shape-editor.cpp
shortcuts.cpp
snap.cpp
snapped-line.cpp
snapped-point.cpp
snapper.cpp
sp-anchor.cpp
sp-animation.cpp
sp-clippath.cpp
sp-conn-end.cpp
sp-conn-end-pair.cpp
sp-cursor.cpp
sp-defs.cpp
sp-ellipse.cpp
sp-feblend.cpp
sp-fecolormatrix.cpp
sp-fecomponenttransfer.cpp
sp-fecomposite.cpp
sp-feconvolvematrix.cpp
sp-fediffuselighting.cpp
sp-fedisplacementmap.cpp
sp-fedistantlight.cpp
sp-feflood.cpp
sp-feimage.cpp
sp-femerge.cpp
sp-femergenode.cpp
sp-femorphology.cpp
sp-feoffset.cpp
sp-fepointlight.cpp
sp-fespecularlighting.cpp
sp-fespotlight.cpp
sp-fetile.cpp
sp-feturbulence.cpp
sp-filter.cpp
sp-filter-primitive.cpp
sp-filter-reference.cpp
sp-flowdiv.cpp
sp-flowregion.cpp
sp-flowtext.cpp
sp-gaussian-blur.cpp
sp-gradient.cpp
sp-gradient-reference.cpp
sp-guide.cpp
sp-image.cpp
spiral-context.cpp
sp-item.cpp
sp-item-group.cpp
sp-item-notify-moveto.cpp
sp-item-rm-unsatisfied-cns.cpp
sp-item-transform.cpp
sp-item-update-cns.cpp
sp-line.cpp
splivarot.cpp
sp-mask.cpp
sp-metadata.cpp
sp-metrics.cpp
sp-namedview.cpp
sp-object.cpp
sp-object-group.cpp
sp-object-repr.cpp
sp-offset.cpp
sp-paint-server.cpp
sp-path.cpp
sp-pattern.cpp
sp-polygon.cpp
sp-polyline.cpp
sp-rect.cpp
sp-root.cpp
sp-shape.cpp
sp-skeleton.cpp
sp-spiral.cpp
sp-star.cpp
sp-string.cpp
sp-style-elem.cpp
sp-switch.cpp
sp-symbol.cpp
sp-text.cpp
sp-tref.cpp
sp-tref-reference.cpp
sp-tspan.cpp
sp-use.cpp
sp-use-reference.cpp
star-context.cpp
streams-gzip.cpp
streams-handles.cpp
streams-jar.cpp
streams-zlib.cpp
style.cpp
#style-test.cpp
svg-view.cpp
svg-view-widget.cpp
text-chemistry.cpp
text-context.cpp
text-editing.cpp
tools-switch.cpp
transf_mat_3x4.cpp
tweak-context.cpp
uri.cpp
uri-references.cpp
vanishing-point.cpp
verbs.cpp
version.cpp
winmain.cpp
zoom-context.cpp
${ONLY_WIN}
)

# make executable for INKSCAPE
ADD_EXECUTABLE(inkscape ${INKSCAPE_SRC})
TARGET_LINK_LIBRARIES(inkscape
	"${INKSCAPE_LINK_FLAGS}" 2geom avoid cola croco gdl nr nrtype vpsc
)
#INSTALL(TARGETS INKSCAPE
#  RUNTIME DESTINATION bin
#  LIBRARY DESTINATION lib
#  ARCHIVE DESTINATION lib
#)

#FILE(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
#INSTALL(FILES ${files} DESTINATION include/INKSCAPE/INKSCAPE)

#CONFIGURE_FILE( ${CMAKE_SOURCE_DIR}/INKSCAPE.pc.in
#                ${CMAKE_BINARY_DIR}/INKSCAPE.pc @ONLY IMMEDIATE )
#INSTALL(FILES "${CMAKE_BINARY_DIR}/INKSCAPE.pc" DESTINATION lib/pkgconfig)
