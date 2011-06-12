
set(ui_dialog_SRC
	aboutbox.cpp
	align-and-distribute.cpp
	calligraphic-profile-rename.cpp
	color-item.cpp
	debug.cpp
	desktop-tracker.cpp
	dialog-manager.cpp
	dialog.cpp
	dock-behavior.cpp
	document-metadata.cpp
	document-properties.cpp
	extension-editor.cpp
	extensions.cpp
	filedialog.cpp
	filedialogimpl-gtkmm.cpp
	fill-and-stroke.cpp
	filter-effects-dialog.cpp
	find.cpp
	floating-behavior.cpp
	glyphs.cpp
	guides.cpp
	icon-preview.cpp
	inkscape-preferences.cpp
	input.cpp
	layer-properties.cpp
	layers.cpp
	livepatheffect-editor.cpp
	memory.cpp
	messages.cpp
	ocaldialogs.cpp
	print-colors-preview-dialog.cpp
	print.cpp
	scriptdialog.cpp
	# session-player.cpp
	svg-fonts-dialog.cpp
	swatches.cpp
	tile.cpp
	tracedialog.cpp
	transformation.cpp
	undo-history.cpp
	# whiteboard-connect.cpp
	# whiteboard-sharewithchat.cpp
	# whiteboard-sharewithuser.cpp
)

if(WIN32)
	list(APPEND ui_dialog_SRC
		filedialogimpl-win32.cpp
	)
endif()
