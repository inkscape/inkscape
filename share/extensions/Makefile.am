
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

extension_SCRIPTS = $(wildcard $(srcdir)/*.py)

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
	xaml2svg.xsl \
	inkscape.extension.rng

module_DATA = $(wildcard $(srcdir)/*.inx)

EXTRA_DIST = \
	$(extension_SCRIPTS) $(otherstuff_DATA) $(module_DATA)


