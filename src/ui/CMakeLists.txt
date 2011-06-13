
set(ui_SRC
	clipboard.cpp
	context-menu.cpp
	previewholder.cpp
	uxmanager.cpp

	cache/svg_preview_cache.cpp

	tool/control-point-selection.cpp
	tool/control-point.cpp
	tool/curve-drag-point.cpp
	tool/event-utils.cpp
	tool/manipulator.cpp
	tool/modifier-tracker.cpp
	tool/multi-path-manipulator.cpp
	tool/node-tool.cpp
	tool/node.cpp
	tool/path-manipulator.cpp
	tool/selectable-control-point.cpp
	tool/selector.cpp
	tool/transform-handle-set.cpp

	dialog/aboutbox.cpp
	dialog/align-and-distribute.cpp
	dialog/calligraphic-profile-rename.cpp
	dialog/color-item.cpp
	dialog/debug.cpp
	dialog/desktop-tracker.cpp
	dialog/dialog-manager.cpp
	dialog/dialog.cpp
	dialog/dock-behavior.cpp
	dialog/document-metadata.cpp
	dialog/document-properties.cpp
	dialog/extension-editor.cpp
	dialog/extensions.cpp
	dialog/filedialog.cpp
	dialog/filedialogimpl-gtkmm.cpp
	dialog/fill-and-stroke.cpp
	dialog/filter-effects-dialog.cpp
	dialog/find.cpp
	dialog/floating-behavior.cpp
	dialog/glyphs.cpp
	dialog/guides.cpp
	dialog/icon-preview.cpp
	dialog/inkscape-preferences.cpp
	dialog/input.cpp
	dialog/layer-properties.cpp
	dialog/layers.cpp
	dialog/livepatheffect-editor.cpp
	dialog/memory.cpp
	dialog/messages.cpp
	dialog/ocaldialogs.cpp
	dialog/print-colors-preview-dialog.cpp
	dialog/print.cpp
	dialog/scriptdialog.cpp
	# dialog/session-player.cpp
	dialog/svg-fonts-dialog.cpp
	dialog/swatches.cpp
	dialog/tile.cpp
	dialog/tracedialog.cpp
	dialog/transformation.cpp
	dialog/undo-history.cpp
	# dialog/whiteboard-connect.cpp
	# dialog/whiteboard-sharewithchat.cpp
	# dialog/whiteboard-sharewithuser.cpp

	widget/button.cpp
	widget/color-picker.cpp
	widget/color-preview.cpp
	widget/combo-text.cpp
	widget/dock-item.cpp
	widget/dock.cpp
	widget/entity-entry.cpp
	widget/entry.cpp
	widget/filter-effect-chooser.cpp
	widget/handlebox.cpp
	widget/icon-widget.cpp
	widget/imageicon.cpp
	widget/imagetoggler.cpp
	widget/labelled.cpp
	widget/layer-selector.cpp
	widget/licensor.cpp
	widget/notebook-page.cpp
	widget/object-composite-settings.cpp
	widget/page-sizer.cpp
	widget/panel.cpp
	widget/point.cpp
	widget/preferences-widget.cpp
	widget/random.cpp
	widget/registered-widget.cpp
	widget/registry.cpp
	widget/rendering-options.cpp
	widget/rotateable.cpp
	widget/ruler.cpp
	widget/scalar-unit.cpp
	widget/scalar.cpp
	widget/selected-style.cpp
	widget/spin-slider.cpp
	widget/spinbutton.cpp
	widget/style-subject.cpp
	widget/style-swatch.cpp
	widget/svg-canvas.cpp
	widget/text.cpp
	widget/tolerance-slider.cpp
	widget/toolbox.cpp
	widget/unit-menu.cpp
	widget/zoom-status.cpp

	view/view.cpp
	view/view-widget.cpp
)

if(WIN32)
	list(APPEND ui_SRC
		dialog/filedialogimpl-win32.cpp
	)
endif()

include_directories(
	"${CMAKE_SOURCE_DIR}/src"
	"${CMAKE_SOURCE_DIR}"
	"${CMAKE_SOURCE_DIR}/bind/javainc"
	"${CMAKE_SOURCE_DIR}/bind/javainc/linux"
	"${CMAKE_SOURCE_DIR}/extension/dbus"
)

add_library(ui_LIB ${ui_SRC})
