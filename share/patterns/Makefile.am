
patternsdir = $(datadir)/inkscape/patterns

patterns_DATA = \
	README \
	patterns.svg \
	patterns.svg.h

patterns.svg.h: patterns.svg i18n.py
	$(srcdir)/i18n.py $(srcdir)/patterns.svg > $(srcdir)/patterns.svg.h

EXTRA_DIST = $(patterns_DATA) i18n.py
