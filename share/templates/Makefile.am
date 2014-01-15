
templatesdir = $(datadir)/inkscape/templates

templates_DATA = \
	README \
	A4_landscape.svg \
	A4.svg \
	black_opaque.svg \
	white_opaque.svg \
	business_card_85x54mm.svg \
	business_card_90x50mm.svg \
	CD_cover_300dpi.svg \
	CD_label_120x120.svg \
	DVD_cover_regular_300dpi.svg \
	DVD_cover_slim_300dpi.svg \
	DVD_cover_superslim_300dpi.svg \
	DVD_cover_ultraslim_300dpi.svg \
	default.svg \
	default.be.svg \
	default.ca.svg \
	default.cs.svg \
	default.de.svg \
	default.eo.svg \
	default.eu.svg \
	default.es.svg \
	default.fi.svg \
	default.fr.svg \
	default.hu.svg \
	default.it.svg \
	default.ja.svg \
	default.lt.svg \
	default.nl.svg \
	default.pl.svg \
	default.pt_BR.svg \
	default.sk.svg \
	default_mm.svg \
	default_pt.svg \
	desktop_1024x768.svg \
	desktop_1600x1200.svg \
	desktop_640x480.svg \
	desktop_800x600.svg \
	fontforge_glyph.svg \
	icon_16x16.svg \
	icon_32x32.svg \
	icon_48x48.svg \
	icon_64x64.svg \
	Letter_landscape.svg \
	Letter.svg \
	no_borders.svg \
	no_layers.svg \
	video_HDTV_1920x1080.svg \
	video_NTSC_720x486.svg \
	video_PAL_720x576.svg \
	web_banner_468x60.svg \
	web_banner_728x90.svg \
	LaTeX_Beamer.svg \
	Typography_Canvas.svg \
	templates.h

templates_i18n = \
	A4_landscape.svg \
	A4.svg \
	black_opaque.svg \
	white_opaque.svg \
	business_card_85x54mm.svg \
	business_card_90x50mm.svg \
	CD_cover_300dpi.svg \
	CD_label_120x120.svg \
	DVD_cover_regular_300dpi.svg \
	DVD_cover_slim_300dpi.svg \
	DVD_cover_superslim_300dpi.svg \
	DVD_cover_ultraslim_300dpi.svg \
	default.svg \
	default.be.svg \
	default.ca.svg \
	default.cs.svg \
	default.de.svg \
	default.eo.svg \
	default.eu.svg \
	default.es.svg \
	default.fi.svg \
	default.fr.svg \
	default.hu.svg \
	default.it.svg \
	default.ja.svg \
	default.lt.svg \
	default.nl.svg \
	default.pl.svg \
	default.pt_BR.svg \
	default.sk.svg \
	default_mm.svg \
	default_pt.svg \
	desktop_1024x768.svg \
	desktop_1600x1200.svg \
	desktop_640x480.svg \
	desktop_800x600.svg \
	fontforge_glyph.svg \
	icon_16x16.svg \
	icon_32x32.svg \
	icon_48x48.svg \
	icon_64x64.svg \
	Letter_landscape.svg \
	Letter.svg \
	no_borders.svg \
	no_layers.svg \
	video_HDTV_1920x1080.svg \
	video_NTSC_720x486.svg \
	video_PAL_720x576.svg \
	web_banner_468x60.svg \
	web_banner_728x90.svg \
	LaTeX_Beamer.svg \
	Typography_Canvas.svg

templates.h: i18n.py $(templates_i18n)
	$(srcdir)/i18n.py $(foreach i,$(templates_i18n),$(srcdir)/$(i)) > $(srcdir)/templates.h

EXTRA_DIST = $(templates_DATA) \
			 i18n.py
			 
