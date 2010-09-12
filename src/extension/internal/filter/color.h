#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_COLOR_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_COLOR_H__
/* Change the 'COLOR' above to be your file name */

/*
 * Copyright (C) 2010 Authors:
 *   Ivan Louette (filters)
 *   Nicolas Dufour (UI) <nicoduf@yahoo.fr>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
/* ^^^ Change the copyright to be you and your e-mail address ^^^ */

#include "filter.h"

#include "extension/internal/clear-n_.h"
#include "extension/system.h"
#include "extension/extension.h"

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Filter {

class Bicolorizer : public Inkscape::Extension::Internal::Filter::Filter {
protected:
	virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
	Bicolorizer ( ) : Filter() { };
	virtual ~Bicolorizer ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

	static void init (void) {
		Inkscape::Extension::build_from_mem(
			"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
				"<name>" N_("Bicolorizer -EXP-") "</name>\n"
				"<id>org.inkscape.effect.filter.bicolorizer</id>\n"
// Using color widgets in tabs makes Inkscape crash...
//	            "<param name=\"tab\" type=\"notebook\">\n"
//                    "<page name=\"Color1\" _gui-text=\"Color 1\">\n"
                        "<_param name=\"header1\" type=\"groupheader\">Color 1</_param>\n"
				        "<param name=\"color1\" gui-text=\"" N_("Color 1") "\" type=\"color\">1946122495</param>\n"
//                    "</page>\n"
//                    "<page name=\"Color2\" _gui-text=\"Color 2\">\n"
                        "<_param name=\"header2\" type=\"groupheader\">Color 2</_param>\n"
				        "<param name=\"color2\" gui-text=\"" N_("Color 2") "\" type=\"color\">367387135</param>\n"
//                    "</page>\n"
//                "</param>\n"
				"<effect>\n"
					"<object-type>all</object-type>\n"
					"<effects-menu>\n"
						"<submenu name=\"" N_("Filters") "\">\n"
   						"<submenu name=\"" N_("Color") "\"/>\n"
			      "</submenu>\n"
					"</effects-menu>\n"
					"<menu-tip>" N_("Change colors to a two colors palette") "</menu-tip>\n"
				"</effect>\n"
			"</inkscape-extension>\n", new Bicolorizer());
	};

};

gchar const *
Bicolorizer::get_filter_text (Inkscape::Extension::Extension * ext)
{
	if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream a1;
    std::ostringstream r1;
    std::ostringstream g1;
    std::ostringstream b1;
    std::ostringstream a2;
    std::ostringstream r2;
    std::ostringstream g2;
    std::ostringstream b2;

    guint32 color1 = ext->get_param_color("color1");
    guint32 color2 = ext->get_param_color("color2");
    a1 << (color1 & 0xff) / 255.0F;
    r1 << ((color1 >> 24) & 0xff);
    g1 << ((color1 >> 16) & 0xff);
    b1 << ((color1 >>  8) & 0xff);
    a2 << (color2 & 0xff) / 255.0F;
    r2 << ((color2 >> 24) & 0xff);
    g2 << ((color2 >> 16) & 0xff);
    b2 << ((color2 >>  8) & 0xff);

	_filter = g_strdup_printf(
		"<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" height=\"1\" width=\"1\" y=\"0\" x=\"0\" inkscape:label=\"Bicolorizer -EXP-\">\n"
            "<feColorMatrix result=\"fbSourceGraphic\" type=\"luminanceToAlpha\" />\n"
            "<feFlood flood-opacity=\"%s\" in=\"fbSourceGraphic\" result=\"result1\" flood-color=\"rgb(%s,%s,%s)\" />\n"
            "<feColorMatrix values=\"0\" result=\"result7\" type=\"hueRotate\" />\n"
            "<feColorMatrix values=\"1\" type=\"saturate\" result=\"result8\" />\n"
            "<feComposite result=\"result2\" operator=\"out\" in=\"result8\" in2=\"fbSourceGraphic\" />\n"
            "<feComposite k2=\"1\" result=\"fbSourceGraphic\" operator=\"arithmetic\" in2=\"SourceGraphic\" />\n"
            "<feFlood flood-opacity=\"%s\" in=\"fbSourceGraphic\" result=\"result2\" flood-color=\"rgb(%s,%s,%s)\" />\n"
            "<feColorMatrix result=\"result3\" values=\"0\" type=\"hueRotate\" />\n"
            "<feColorMatrix values=\"1\" result=\"result9\" type=\"saturate\" />\n"
            "<feMerge result=\"result5\">\n"
                "<feMergeNode in=\"result9\" />\n"
                "<feMergeNode in=\"fbSourceGraphic\" />\n"
            "</feMerge>\n"
            "<feBlend result=\"result6\" mode=\"normal\" blend=\"normal\" in2=\"fbSourceGraphic\" />\n"
            "<feComposite result=\"result6\" operator=\"in\" in2=\"SourceGraphic\" />\n"
        "</filter>\n", a1.str().c_str(), r1.str().c_str(), g1.str().c_str(), b1.str().c_str(), a2.str().c_str(), r2.str().c_str(), g2.str().c_str(), b2.str().c_str());

	return _filter;
};
}; /* namespace Filter */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */

/* Change the 'COLOR' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_COLOR_H__ */
