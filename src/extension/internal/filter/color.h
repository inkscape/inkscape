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

class Duochrome : public Inkscape::Extension::Internal::Filter::Filter {
protected:
	virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
	Duochrome ( ) : Filter() { };
	virtual ~Duochrome ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

	static void init (void) {
		Inkscape::Extension::build_from_mem(
			"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
				"<name>" N_("Duochrome, custom -EXP-") "</name>\n"
				"<id>org.inkscape.effect.filter.Duochrome</id>\n"
// Using color widgets in tabs makes Inkscape crash...
//	            "<param name=\"tab\" type=\"notebook\">\n"
//                    "<page name=\"Color1\" _gui-text=\"Color 1\">\n"
                        "<param name=\"fluo\" gui-text=\"" N_("Fluorescence") "\" type=\"boolean\">false</param>\n"
                        "<_param name=\"header1\" type=\"groupheader\">Color 1</_param>\n"
				        "<param name=\"color1\" gui-text=\"" N_("Color 1") "\" type=\"color\">1364325887</param>\n"
//                    "</page>\n"
//                    "<page name=\"Color2\" _gui-text=\"Color 2\">\n"
                        "<_param name=\"header2\" type=\"groupheader\">Color 2</_param>\n"
				        "<param name=\"color2\" gui-text=\"" N_("Color 2") "\" type=\"color\">-65281</param>\n"
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
			"</inkscape-extension>\n", new Duochrome());
	};

};

gchar const *
Duochrome::get_filter_text (Inkscape::Extension::Extension * ext)
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
    std::ostringstream fluo;

    guint32 color1 = ext->get_param_color("color1");
    guint32 color2 = ext->get_param_color("color2");
    bool fluorescence = ext->get_param_bool("fluo");
    a1 << (color1 & 0xff) / 255.0F;
    r1 << ((color1 >> 24) & 0xff);
    g1 << ((color1 >> 16) & 0xff);
    b1 << ((color1 >>  8) & 0xff);
    a2 << (color2 & 0xff) / 255.0F;
    r2 << ((color2 >> 24) & 0xff);
    g2 << ((color2 >> 16) & 0xff);
    b2 << ((color2 >>  8) & 0xff);
    if (fluorescence) fluo << "";
    else  fluo << " in=\"result6\"";

	_filter = g_strdup_printf(
		"<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" height=\"1\" width=\"1\" y=\"0\" x=\"0\" inkscape:label=\"Duochrome, custom -EXP-\">\n"
            "<feColorMatrix type=\"luminanceToAlpha\" result=\"fbSourceGraphic\" />\n"
            "<feFlood in=\"fbSourceGraphic\" flood-opacity=\"%s\" flood-color=\"rgb(%s,%s,%s)\" result=\"result1\" />\n"
            "<feComposite in=\"result1\" in2=\"fbSourceGraphic\" operator=\"out\" result=\"result2\" />\n"
            "<feComposite in2=\"SourceGraphic\" k2=\"1\" result=\"fbSourceGraphic\" operator=\"arithmetic\" />\n"
            "<feFlood in=\"fbSourceGraphic\" flood-opacity=\"%s\" flood-color=\"rgb(%s,%s,%s)\" result=\"result2\" />\n"
            "<feMerge result=\"result5\">\n"
                "<feMergeNode in=\"result2\" />\n"
                "<feMergeNode in=\"fbSourceGraphic\" />\n"
            "</feMerge>\n"
            "<feBlend in2=\"fbSourceGraphic\" mode=\"normal\" blend=\"normal\" result=\"result6\" />\n"
            "<feColorMatrix type=\"matrix\" values=\"2 -1 0 0 0 0 2 -1 0 0 -1 0 2 0 0 0 0 0 1 0 \" result=\"result10\" />\n"
            "<feComposite %s in2=\"SourceGraphic\" operator=\"in\" result=\"fbSourceGraphic\" />\n"
        "</filter>\n", a1.str().c_str(), r1.str().c_str(), g1.str().c_str(), b1.str().c_str(), a2.str().c_str(), r2.str().c_str(), g2.str().c_str(), b2.str().c_str(), fluo.str().c_str());

	return _filter;
};
}; /* namespace Filter */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */

/* Change the 'COLOR' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_COLOR_H__ */
