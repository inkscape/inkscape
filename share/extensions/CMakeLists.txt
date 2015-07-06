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
      "*.py"
      "*.pl"
      "*.sh"
      "*.rb"
      "*.inx"
    )
install(FILES ${_FILES} DESTINATION ${SHARE_INSTALL}/inkscape/extensions)

file(GLOB _FILES "alphabet_soup/*.svg")
install(FILES ${_FILES} DESTINATION ${SHARE_INSTALL}/inkscape/extensions/alphabet_soup)

file(GLOB _FILES "Barcode/*.py")
install(FILES ${_FILES} DESTINATION ${SHARE_INSTALL}/inkscape/extensions/Barcode)

file(GLOB _FILES "Poly3DObjects/*.obj")
install(FILES ${_FILES} DESTINATION ${SHARE_INSTALL}/inkscape/extensions/Poly3DObjects)

# file(GLOB _FILES
#       "test/*.svg"
#       "test/*.sh"
#       "test/*.py"
#       "test/*.js"
#       "test/run-all-extension-tests"
#     )
# install(FILES ${_FILES} DESTINATION ${SHARE_INSTALL}/inkscape/extensions/test)

file(GLOB _FILES "ink2canvas/*.py")
install(FILES ${_FILES} DESTINATION ${SHARE_INSTALL}/inkscape/extensions/ink2canvas)

file(GLOB _FILES "scour/*.py")
install(FILES ${_FILES} DESTINATION ${SHARE_INSTALL}/inkscape/extensions/scour)

file(GLOB _FILES "xaml2svg/*.xsl")
install(FILES ${_FILES} DESTINATION ${SHARE_INSTALL}/inkscape/extensions/xaml2svg)
