
SUBDIRS = \
	alphabet_soup \
	Barcode \
	Poly3DObjects \
	test \
	ink2canvas \
	xaml2svg

extensiondir = $(datadir)/inkscape/extensions

otherstuffdir = $(datadir)/inkscape/extensions

moduledir = $(datadir)/inkscape/extensions

# If you remove an extension from here, either add it to EXTRA_DIST
# below, or remove all mention of it from po/inkscape.pot as well.
module_DATA = $(wildcard $(extensiondir)/*.py)

otherstuff_DATA = \
	fontfix.conf \
	inkweb.js \
	jessyInk.js \
	jessyInk_core_mouseHandler_noclick.js \
	jessyInk_core_mouseHandler_zoomControl.js \
	aisvg.xslt \
	colors.xml \
	jessyInk_video.svg \
	svg2fxg.xsl \
	svg2xaml.xsl \
	xaml2svg.xsl

module_DATA = $(wildcard $(moduledir)/*.inx)

EXTRA_DIST = \
	$(extension_SCRIPTS) $(otherstuff_DATA) $(module_DATA)


