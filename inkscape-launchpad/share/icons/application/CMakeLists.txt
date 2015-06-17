set(PIXMAP_SIZES "16x16" "22x22" "24x24" "32x32" "48x48" "256x256")
set(THEME hicolor)
foreach(pixmap_size ${PIXMAP_SIZES})
  FILE(GLOB PIXMAP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/${pixmap_size}/*.png ${CMAKE_CURRENT_SOURCE_DIR}/${pixmap_size}/*.svg)
  install(FILES ${PIXMAP_FILES} DESTINATION ${SHARE_INSTALL}/icons/${THEME}/${pixmap_size}/apps)
endforeach(pixmap_size)

# I hope that this is actually run after installing the files.
install(CODE "execute_process(COMMAND gtk-update-icon-cache -f -t ${CMAKE_INSTALL_PREFIX}/${SHARE_INSTALL}/icons/${THEME})")