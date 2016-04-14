# This is called by cmake as an extermal process from
# ./po/CMakeLists.txt and creates inkscape.desktop
#
# These variables are defined by the caller, matching the CMake equivilents.
# - ${INKSCAPE_SOURCE_DIR}
# - ${INKSCAPE_BINARY_DIR}
message("building inkscape.desktop")
set(INKSCAPE_MIMETYPE "image/svg+xml;image/svg+xml-compressed;application/vnd.corel-draw;application/pdf;application/postscript;image/x-eps;application/illustrator;image/cgm;image/x-wmf;application/x-xccx;application/x-xcgm;application/x-xcdt;application/x-xsk1;application/x-xcmx;image/x-xcdr;application/visio;application/x-visio;application/vnd.visio;application/visio.drawing;application/vsd;application/x-vsd;image/x-vsd;")
configure_file(${INKSCAPE_BINARY_DIR}/inkscape.desktop.template.in ${INKSCAPE_BINARY_DIR}/inkscape.desktop)
