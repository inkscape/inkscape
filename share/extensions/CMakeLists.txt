# Install the set of non-executable data files
file(GLOB _FILES
      "README"
      "fontfix.conf"
      "inkweb.js"
      "jessyInk.js"
      "jessyInk_core_mouseHandler_noclick.js"
      "jessyInk_core_mouseHandler_zoomControl.js"
      "aisvg.xslt"
      "colors.xml"
      "jessyInk_video.svg"
      "seamless_pattern.svg"
      "svg2fxg.xsl"
      "svg2xaml.xsl"
      "xaml2svg.xsl"
      "inkscape.extension.rng"
      "*.inx"
    )

install(FILES ${_FILES} DESTINATION ${INKSCAPE_SHARE_INSTALL}/extensions)

# Install the executable scripts
file(GLOB _SCRIPTS
      "*.py"
      "*.pl"
      "*.sh"
      "*.rb"
    )
# These files don't need the +x bit
set(_SCRIPTS_NOEXEC
    "hersheydata.py"
    "hpgl_decoder.py"
    "hpgl_encoder.py"
    "simplepath.py"
    "simplestyle.py"
    "simpletransform.py"
)
list(REMOVE_ITEM _SCRIPTS ${_SCRIPTS_NOEXEC})
install(PROGRAMS ${_SCRIPTS} DESTINATION ${INKSCAPE_SHARE_INSTALL}/extensions)
install(FILES ${_SCRIPTS_NOEXEC} DESTINATION ${INKSCAPE_SHARE_INSTALL}/extensions)

file(GLOB _FILES "alphabet_soup/*.svg")
install(FILES ${_FILES} DESTINATION ${INKSCAPE_SHARE_INSTALL}/extensions/alphabet_soup)

file(GLOB _FILES "Barcode/*.py")
install(FILES ${_FILES} DESTINATION ${INKSCAPE_SHARE_INSTALL}/extensions/Barcode)

file(GLOB _FILES "Poly3DObjects/*.obj")
install(FILES ${_FILES} DESTINATION ${INKSCAPE_SHARE_INSTALL}/extensions/Poly3DObjects)

# file(GLOB _FILES
#       "test/*.svg"
#       "test/*.sh"
#       "test/*.py"
#       "test/*.js"
#       "test/run-all-extension-tests"
#     )
# install(FILES ${_FILES} DESTINATION ${INKSCAPE_SHARE_INSTALL}/extensions/test)

file(GLOB _FILES "ink2canvas/*.py")
install(FILES ${_FILES} DESTINATION ${INKSCAPE_SHARE_INSTALL}/extensions/ink2canvas)

file(GLOB _FILES "xaml2svg/*.xsl")
install(FILES ${_FILES} DESTINATION ${INKSCAPE_SHARE_INSTALL}/extensions/xaml2svg)
