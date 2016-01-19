
SUBDIRS = \
	alphabet_soup \
	Barcode \
	ink2canvas \
	Poly3DObjects \
	test \
	xaml2svg

extensiondir = $(datadir)/inkscape/extensions

otherstuffdir = $(datadir)/inkscape/extensions

moduledir = $(datadir)/inkscape/extensions

extension_SCRIPTS = \
	$(wildcard $(srcdir)/*.py) \
	$(wildcard $(srcdir)/*.pl) \
	$(wildcard $(srcdir)/*.sh) \
	$(wildcard $(srcdir)/*.rb)

otherstuff_DATA = \
	fontfix.conf \
	inkweb.js \
	jessyInk.js \
	jessyInk_core_mouseHandler_noclick.js \
	jessyInk_core_mouseHandler_zoomControl.js \
	aisvg.xslt \
	colors.xml \
	jessyInk_video.svg \
	seamless_pattern.svg \
	svg2fxg.xsl \
	svg2xaml.xsl \
	xaml2svg.xsl \
	inkscape.extension.rng

module_DATA = $(wildcard $(srcdir)/*.inx)

EXTRA_DIST = \
	$(extension_SCRIPTS) $(otherstuff_DATA) $(module_DATA)


