
filtersdir = $(datadir)/inkscape/filters

filters_DATA =  \
	README \
	filters.svg \
	filters.svg.h

filters.svg.h: filters.svg i18n.py
	$(srcdir)/i18n.py $(srcdir)/filters.svg > $(srcdir)/filters.svg.h

EXTRA_DIST = $(filters_DATA) \
             i18n.py
