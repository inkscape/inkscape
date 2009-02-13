themedir = $(pkgdatadir)/icons/hicolor
size = scalable
context = places

iconsdir = $(themedir)/$(size)/$(context)

# To update this list, run:
# find . -name '*.svg' -and -not -name '*.svn*' -printf '\t%P \\\n' | sort | zenity --text-info
# and paste the contents of the dialog box here
#
# To vacuum defs in all documents, simply execute
# inkscape --vacuum-defs *

icons_DATA = \
	dialog-align-and-distribute.svg \
	dialog-fill-and-stroke.svg \
	dialog-icon-preview.svg \
	dialog-input-devices.svg \
	dialog-layers.svg \
	dialog-memory.svg \
	dialog-messages.svg \
	dialog-object-properties.svg \
	dialog-rows-and-columns.svg \
	dialog-scripts.svg \
	dialog-text-and-font.svg \
	dialog-tile-clones.svg \
	dialog-xml-editor.svg \
	grid-axonometric.svg \
	grid-rectangular.svg \
	guides.svg

EXTRA_DIST = $(icons_DATA)
