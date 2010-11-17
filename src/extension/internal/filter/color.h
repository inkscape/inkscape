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
                        "<param name=\"fluo\" gui-text=\"" N_("Fluorescence level:") "\" type=\"float\" min=\"0\" max=\"2\">0</param>\n"
                        "<param name=\"swap\" gui-text=\"" N_("Swap:") "\" type=\"enum\">\n"
                            "<_item value=\"none\">No swap</_item>\n"
                            "<_item value=\"full\">Color and alpha</_item>\n"
                            "<_item value=\"color\">Color only</_item>\n"
                            "<_item value=\"alpha\">Alpha only</_item>\n"
                        "</param>\n"
                        "<_param name=\"header1\" type=\"groupheader\">Color 1</_param>\n"
				        "<param name=\"color1\" gui-text=\"" N_("Color 1") "\" type=\"color\">1364325887</param>\n"
                        "<_param name=\"header2\" type=\"groupheader\">Color 2</_param>\n"
				        "<param name=\"color2\" gui-text=\"" N_("Color 2") "\" type=\"color\">-65281</param>\n"
				"<effect>\n"
					"<object-type>all</object-type>\n"
					"<effects-menu>\n"
						"<submenu name=\"" N_("Filters") "\">\n"
   						"<submenu name=\"" N_("Experimental") "\"/>\n"
			      "</submenu>\n"
					"</effects-menu>\n"
					"<menu-tip>" N_("Convert luminance values to a duochrome palette") "</menu-tip>\n"
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
    std::ostringstream swap1;
    std::ostringstream swap2;
    guint32 color1 = ext->get_param_color("color1");
    guint32 color2 = ext->get_param_color("color2");
    float fluorescence = ext->get_param_float("fluo");
    const gchar *swaptype = ext->get_param_enum("swap");

    r1 << ((color1 >> 24) & 0xff);
    g1 << ((color1 >> 16) & 0xff);
    b1 << ((color1 >>  8) & 0xff);
    r2 << ((color2 >> 24) & 0xff);
    g2 << ((color2 >> 16) & 0xff);
    b2 << ((color2 >>  8) & 0xff);
    fluo << fluorescence;

    if((g_ascii_strcasecmp("full", swaptype) == 0)) {
        swap1 << "in";
        swap2 << "out";
        a1 << (color1 & 0xff) / 255.0F;
        a2 << (color2 & 0xff) / 255.0F;
    } else if((g_ascii_strcasecmp("color", swaptype) == 0)) {
        swap1 << "in";
        swap2 << "out";
        a1 << (color2 & 0xff) / 255.0F;
        a2 << (color1 & 0xff) / 255.0F;
    } else if((g_ascii_strcasecmp("alpha", swaptype) == 0)) {
        swap1 << "out";
        swap2 << "in";
        a1 << (color2 & 0xff) / 255.0F;
        a2 << (color1 & 0xff) / 255.0F;
    } else {
        swap1 << "out";
        swap2 << "in";
        a1 << (color1 & 0xff) / 255.0F;
        a2 << (color2 & 0xff) / 255.0F;
    }

	_filter = g_strdup_printf(
		"<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" height=\"1\" width=\"1\" y=\"0\" x=\"0\" inkscape:label=\"Duochrome, custom -EXP-\">\n"
            "<feColorMatrix type=\"luminanceToAlpha\" result=\"colormatrix1\" />\n"
            "<feFlood flood-opacity=\"%s\" flood-color=\"rgb(%s,%s,%s)\" result=\"flood1\" />\n"
            "<feComposite in2=\"colormatrix1\" operator=\"%s\" result=\"composite1\" />\n"
            "<feFlood in=\"colormatrix1\" flood-opacity=\"%s\" flood-color=\"rgb(%s,%s,%s)\" result=\"flood2\" />\n"
            "<feComposite in2=\"colormatrix1\" result=\"composite2\" operator=\"%s\" />\n"
            "<feComposite in=\"composite2\" in2=\"composite1\" k2=\"1\"  k3=\"1\" operator=\"arithmetic\" result=\"composite3\" />\n"
            "<feColorMatrix in=\"composite3\" type=\"matrix\" values=\"2 -1 0 0 0 0 2 -1 0 0 -1 0 2 0 0 0 0 0 1 0 \" result=\"colormatrix2\" />\n"
            "<feComposite in=\"colormatrix2\" in2=\"composite3\" operator=\"arithmetic\" k2=\"%s\" result=\"composite4\" />\n"
            "<feBlend in=\"composite4\" in2=\"composite3\" blend=\"normal\" mode=\"normal\" result=\"blend\" />\n"
            "<feComposite in2=\"SourceGraphic\" operator=\"in\" />\n"
        "</filter>\n", a1.str().c_str(), r1.str().c_str(), g1.str().c_str(), b1.str().c_str(), swap1.str().c_str(), a2.str().c_str(), r2.str().c_str(), g2.str().c_str(), b2.str().c_str(), swap2.str().c_str(), fluo.str().c_str());

	return _filter;
};
}; /* namespace Filter */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */

/* Change the 'COLOR' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_COLOR_H__ */
