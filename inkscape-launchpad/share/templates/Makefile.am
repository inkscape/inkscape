
templatesdir = $(datadir)/inkscape/templates

templates_DATA = \
	README \
	CD_label_120x120.svg \
	default.svg \
	default.be.svg \
	default.ca.svg \
	default.cs.svg \
	default.de.svg \
	default.en_US.svg \
	default.eo.svg \
	default.eu.svg \
	default.es.svg \
	default.fi.svg \
	default.fr.svg \
	default.hu.svg \
	default.is.svg \
	default.it.svg \
	default.ja.svg \
	default.lt.svg \
	default.nl.svg \
	default.pl.svg \
	default.pt_BR.svg \
	default.sk.svg \
	default_pt.svg \
	default_px.svg \
	no_layers.svg \
	LaTeX_Beamer.svg \
	Typography_Canvas.svg \
	templates.h

templates_i18n = \
	CD_label_120x120.svg \
	default.svg \
	default.be.svg \
	default.ca.svg \
	default.cs.svg \
	default.de.svg \
	default.en_US.svg \
	default.eo.svg \
	default.eu.svg \
	default.es.svg \
	default.fi.svg \
	default.fr.svg \
	default.hu.svg \
	default.is.svg \
	default.it.svg \
	default.ja.svg \
	default.lt.svg \
	default.nl.svg \
	default.pl.svg \
	default.pt_BR.svg \
	default.sk.svg \
	default_pt.svg \
	default_px.svg \
	no_layers.svg \
	LaTeX_Beamer.svg \
	Typography_Canvas.svg

templates.h: i18n.py $(templates_i18n)
	$(srcdir)/i18n.py $(foreach i,$(templates_i18n),$(srcdir)/$(i)) > $(srcdir)/templates.h

EXTRA_DIST = $(templates_DATA) \
			 i18n.py
			 
