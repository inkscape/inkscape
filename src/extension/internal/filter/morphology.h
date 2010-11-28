#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_MORPHOLOGY_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_MORPHOLOGY_H__
/* Change the 'MORPHOLOGY' above to be your file name */

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

/**
    \brief    Custom predefined Crosssmooth filter.
    
    Smooth the outside of shapes and pictures.

    Filter's parameters:
    * Type (enum, default "Smooth edges") ->
        Smooth edges = composite1 (in="SourceGraphic", in2="blur")
        Smooth all = composite1 (in="blur", in2="blur")
    * Blur (0.01->10., default 5.) -> blur (stdDeviation)
*/

class Crosssmooth : public Inkscape::Extension::Internal::Filter::Filter {
protected:
	virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
	Crosssmooth ( ) : Filter() { };
	virtual ~Crosssmooth ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

	static void init (void) {
		Inkscape::Extension::build_from_mem(
			"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
				"<name>" N_("Cross-smooth, custom -EXP-") "</name>\n"
				"<id>org.inkscape.effect.filter.Crosssmooth</id>\n"
                        "<param name=\"type\" gui-text=\"" N_("Type:") "\" type=\"enum\">\n"
                            "<_item value=\"edges\">Smooth edges</_item>\n"
                            "<_item value=\"all\">Smooth all</_item>\n"
                        "</param>\n"
                        "<param name=\"blur\" gui-text=\"" N_("Blur:") "\" type=\"float\" min=\"0.01\" max=\"10\">5</param>\n"
				"<effect>\n"
					"<object-type>all</object-type>\n"
					"<effects-menu>\n"
						"<submenu name=\"" N_("Filters") "\">\n"
   						"<submenu name=\"" N_("Experimental") "\"/>\n"
			      "</submenu>\n"
					"</effects-menu>\n"
					"<menu-tip>" N_("Smooth edges and angles of shapes") "</menu-tip>\n"
				"</effect>\n"
			"</inkscape-extension>\n", new Crosssmooth());
	};

};

gchar const *
Crosssmooth::get_filter_text (Inkscape::Extension::Extension * ext)
{
	if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream blur;
    std::ostringstream c1in;

    blur << ext->get_param_float("blur");

    const gchar *type = ext->get_param_enum("type");
    if((g_ascii_strcasecmp("all", type) == 0)) {
        c1in << "blur";
    } else {
        c1in << "SourceGraphic";
    }

	_filter = g_strdup_printf(
		"<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" y=\"0\" x=\"0\" inkscape:label=\"Cross-smooth, custom -EXP-\">\n"
        "<feGaussianBlur stdDeviation=\"%s\" result=\"blur\" />\n"
        "<feComposite in=\"%s\" in2=\"blur\" operator=\"atop\" result=\"composite1\" />\n"
        "<feComposite in2=\"composite1\" operator=\"in\" result=\"composite2\" />\n"
        "<feComposite in2=\"composite2\" operator=\"in\" result=\"composite3\" />\n"
        "<feColorMatrix values=\"1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 0 0 0 20 -10 \" result=\"colormatrix\" />\n"
        "</filter>\n", blur.str().c_str(), c1in.str().c_str());

	return _filter;
}; /* Crosssmooth filter */

}; /* namespace Filter */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */

/* Change the 'COLOR' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_MORPHOLOGY_H__ */
