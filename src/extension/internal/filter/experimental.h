#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_EXPERIMENTAL_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_EXPERIMENTAL_H__
/* Change the 'EXPERIMENTAL' above to be your file name */

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

class Posterize : public Inkscape::Extension::Internal::Filter::Filter {
protected:
	virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
	Posterize ( ) : Filter() { };
	virtual ~Posterize ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

	static void init (void) {
		Inkscape::Extension::build_from_mem(
			"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
				"<name>" N_("Posterize, custom -EXP-") "</name>\n"
				"<id>org.inkscape.effect.filter.Posterize</id>\n"
                "<param name=\"type\" gui-text=\"" N_("Type:") "\" type=\"enum\">\n"
                    "<_item value=\"normal\">Normal</_item>\n"
                    "<_item value=\"contrasted\">Contrasted</_item>\n"
                "</param>\n"
                "<param name=\"level\" gui-text=\"" N_("Level:") "\" type=\"int\" min=\"1\" max=\"10\">3</param>\n"
				"<effect>\n"
					"<object-type>all</object-type>\n"
					"<effects-menu>\n"
						"<submenu name=\"" N_("Filters") "\">\n"
   						"<submenu name=\"" N_("Experimental") "\"/>\n"
			      "</submenu>\n"
					"</effects-menu>\n"
					"<menu-tip>" N_("Change colors to a two colors palette") "</menu-tip>\n"
				"</effect>\n"
			"</inkscape-extension>\n", new Posterize());
	};

};

gchar const *
Posterize::get_filter_text (Inkscape::Extension::Extension * ext)
{
	if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream transf;

    int level = ext->get_param_int("level") + 1;
    const gchar *type = ext->get_param_enum("type");
    float val = 0.0;
    transf << "0";
    for ( int step = 1 ; step <= level ; step++ ) {
        val = (float) step / level;
        transf << " " << val;
        if((g_ascii_strcasecmp("contrasted", type) == 0)) {
            transf << " " << (val - ((float) 1 / (3 * level))) << " " << (val + ((float) 1 / (2 * level)));
        }
    }
    transf << " 1";

	_filter = g_strdup_printf(
		"<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" height=\"1\" width=\"1\" y=\"0\" x=\"0\" inkscape:label=\"Posterize, custom -EXP-\">\n"
            "<feGaussianBlur stdDeviation=\"1.5\" result=\"result3\" />\n"
            "<feGaussianBlur stdDeviation=\"1.5\" in=\"SourceGraphic\" result=\"result4\" />\n"
            "<feBlend in2=\"result3\" blend=\"normal\" mode=\"lighten\" />\n"
            "<feComponentTransfer result=\"result1\">\n"
                "<feFuncR type=\"discrete\" tableValues=\"%s\" />\n"
                "<feFuncG type=\"discrete\" tableValues=\"%s\" />\n"
                "<feFuncB type=\"discrete\" tableValues=\"%s\" />\n"
            "</feComponentTransfer>\n"
            "<feColorMatrix type=\"saturate\" values=\"1\" />\n"
            "<feGaussianBlur stdDeviation=\"0.01\" />\n"
            "<feComposite in2=\"SourceGraphic\" operator=\"atop\" result=\"result2\" />\n"
        "</filter>\n", transf.str().c_str(), transf.str().c_str(), transf.str().c_str());

	return _filter;
};
}; /* namespace Filter */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */

/* Change the 'COLOR' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_EXPERIMENTAL_H__ */
