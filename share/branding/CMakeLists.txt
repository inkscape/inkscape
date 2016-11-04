file(GLOB _FILES "README" "*.svg")
install(FILES ${_FILES} DESTINATION ${SHARE_INSTALL}/inkscape/branding)

file(GLOB _FILES
      "EuphoriaScriptRegularExtended/EuphoriaScript-Regular_extended.otf"
      "EuphoriaScriptRegularExtended/FONTLOG.txt"
      "EuphoriaScriptRegularExtended/METADATA.json"
      "EuphoriaScriptRegularExtended/*.txt"
    )

install(FILES ${_FILES} DESTINATION ${SHARE_INSTALL}/inkscape/branding/EuphoriaScriptRegularExtended)

file(GLOB _FILES
      "LinLibertine/README"
      "LinLibertine/VERSION"
      "LinLibertine/*.oft"
      "LinLibertine/*.txt"
    )

install(FILES ${_FILES} DESTINATION ${SHARE_INSTALL}/inkscape/branding/LinLibertine)
