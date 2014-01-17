
symbolsdir = $(datadir)/inkscape/symbols

# Adding srcdir allows out-of-src builds to work
symbols_DATA = \
    README \
    $(wildcard $(srcdir)/*.svg) \
    symbols.h

symbols_i18n = $(wildcard $(srcdir)/*.svg)

symbols.h: i18n.py $(symbols_i18n)
	$(srcdir)/i18n.py $(symbols_i18n) > $(srcdir)/symbols.h

EXTRA_DIST = $(symbols_DATA) \
             i18n.py

