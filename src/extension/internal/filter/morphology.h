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
    * Type (enum, default "Smooth all") ->
        Smooth all = composite (in="colormatrix", in2="colormatrix")
        Smooth edges = composite (in="SourceGraphic", in2="colormatrix")
    * Blur (0.01->10., default 5.) -> blur (stdDeviation)
    * Spreading (1->100, default 20) -> colormatrix (value n-1)
    * Erosion (0->-100, default -15) -> colormatrix (value n)
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
                            "<_item value=\"all\">Smooth all</_item>\n"
                            "<_item value=\"edges\">Smooth edges</_item>\n"
                        "</param>\n"
                        "<param name=\"blur\" gui-text=\"" N_("Blur:") "\" type=\"float\" min=\"0.01\" max=\"10\">5</param>\n"
                        "<param name=\"spreading\" gui-text=\"" N_("Spreading:") "\" type=\"int\" min=\"1\" max=\"100\">20</param>\n"
                        "<param name=\"erosion\" gui-text=\"" N_("Erosion:") "\" type=\"int\" min=\"-100\" max=\"0\">-15</param>\n"
				"<effect>\n"
					"<object-type>all</object-type>\n"
					"<effects-menu>\n"
						"<submenu name=\"" N_("Filters") "\">\n"
   						"<submenu name=\"" N_("Experimental") "\"/>\n"
			      "</submenu>\n"
					"</effects-menu>\n"
					"<menu-tip>" N_("Smooth the outside of shapes and pictures without altering their contents") "</menu-tip>\n"
				"</effect>\n"
			"</inkscape-extension>\n", new Crosssmooth());
	};

};

gchar const *
Crosssmooth::get_filter_text (Inkscape::Extension::Extension * ext)
{
	if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream blur;
    std::ostringstream spreading;
    std::ostringstream erosion;
    std::ostringstream cin;

    blur << ext->get_param_float("blur");
    spreading << ext->get_param_int("spreading");
    erosion << ext->get_param_int("erosion");

    const gchar *type = ext->get_param_enum("type");
    if((g_ascii_strcasecmp("all", type) == 0)) {
        cin << "colormatrix";
    } else {
        cin << "SourceGraphic";
    }
    
	_filter = g_strdup_printf(
		"<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" height=\"1\" width=\"1\" y=\"0\" x=\"0\" inkscape:label=\"Cross-smooth, custom -EXP-\">\n"
        "<feGaussianBlur stdDeviation=\"%s\" result=\"blur\" />\n"
        "<feColorMatrix in=\"blur\" values=\"1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 0 0 0 %s %s \" result=\"colormatrix\" />\n"
        "<feComposite in=\"%s\" in2=\"colormatrix\" operator=\"in\" />\n"
        "</filter>\n", blur.str().c_str(), spreading.str().c_str(), erosion.str().c_str(), cin.str().c_str());

	return _filter;
}; /* Crosssmooth filter */

}; /* namespace Filter */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */

/* Change the 'COLOR' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_MORPHOLOGY_H__ */
