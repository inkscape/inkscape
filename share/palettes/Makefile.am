
palettesdir = $(datadir)/inkscape/palettes

palettes_DATA = \
    README \
    Blues.gpl \
    echo-palette.gpl \
    Gold.gpl \
    Greens.gpl \
    Gray.gpl \
    Hilite.gpl \
    inkscape.gpl \
    LaTeX-Beamer.gpl \
    Khaki.gpl \
    MATLAB-Jet-72.gpl \
    Reds.gpl \
    Royal.gpl \
    svg.gpl \
    Tango-Palette.gpl \
    Topographic.gpl \
    Ubuntu.gpl \
    webhex.gpl \
    websafe22.gpl \
    windowsXP.gpl \
    palettes.h

palettes_i18n = \
    echo-palette.gpl \
    inkscape.gpl \
    LaTeX-Beamer.gpl \
    svg.gpl \
    Tango-Palette.gpl \
    Ubuntu.gpl

palettes.h: i18n.py $(palettes_i18n)
	$(srcdir)/i18n.py $(palettes_i18n) > $(srcdir)/palettes.h

EXTRA_DIST = $(palettes_DATA) \
             i18n.py
             
