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
				"<name>" N_("Poster and painting, custom -EXP-") "</name>\n"
				"<id>org.inkscape.effect.filter.Posterize</id>\n"
                "<param name=\"type\" gui-text=\"" N_("Effect type:") "\" type=\"enum\">\n"
                    "<_item value=\"normal\">Normal</_item>\n"
                    "<_item value=\"dented\">Dented</_item>\n"
                "</param>\n"
                "<param name=\"table\" gui-text=\"" N_("Transfer type:") "\" type=\"enum\">\n"
                    "<_item value=\"discrete\">Poster</_item>\n"
                    "<_item value=\"table\">Painting</_item>\n"
                "</param>\n"
                "<param name=\"levels\" gui-text=\"" N_("Levels:") "\" type=\"int\" min=\"1\" max=\"15\">5</param>\n"
                "<param name=\"blend\" gui-text=\"" N_("Blend mode:") "\" type=\"enum\">\n"
                    "<_item value=\"lighten\">Ligthen</_item>\n"
                    "<_item value=\"normal\">Normal</_item>\n"
                    "<_item value=\"darken\">Darken</_item>\n"
                "</param>\n"
                "<param name=\"blur1\" gui-text=\"" N_("Primary blur:") "\" type=\"float\" min=\"0.0\" max=\"100.0\">4.0</param>\n"
                "<param name=\"blur2\" gui-text=\"" N_("Secondary blur:") "\" type=\"float\" min=\"0.0\" max=\"100.0\">0.5</param>\n"
                "<param name=\"presaturation\" gui-text=\"" N_("Pre-saturation:") "\" type=\"float\" min=\"0.00\" max=\"1.00\">1.00</param>\n"
                "<param name=\"postsaturation\" gui-text=\"" N_("Post-saturation:") "\" type=\"float\" min=\"0.00\" max=\"1.00\">1.00</param>\n"
                "<param name=\"antialiasing\" gui-text=\"" N_("Simulate antialiasing") "\" type=\"boolean\">false</param>\n"
				"<effect>\n"
					"<object-type>all</object-type>\n"
					"<effects-menu>\n"
						"<submenu name=\"" N_("Filters") "\">\n"
   						"<submenu name=\"" N_("Experimental") "\"/>\n"
			      "</submenu>\n"
					"</effects-menu>\n"
					"<menu-tip>" N_("Poster and painting effects") "</menu-tip>\n"
				"</effect>\n"
			"</inkscape-extension>\n", new Posterize());
	};

};

gchar const *
Posterize::get_filter_text (Inkscape::Extension::Extension * ext)
{
	if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream table;
    std::ostringstream blendmode;
    std::ostringstream blur1;
    std::ostringstream blur2;
    std::ostringstream presat;
    std::ostringstream postsat;
    std::ostringstream transf;
    std::ostringstream antialias;
    
    table << ext->get_param_enum("table");
    blendmode << ext->get_param_enum("blend");
    blur1 << ext->get_param_float("blur1") + 0.01;
    blur2 << ext->get_param_float("blur2") + 0.01;
    presat << ext->get_param_float("presaturation");
    postsat << ext->get_param_float("postsaturation");


    // TransfertComponenet table values are calculated based on the poster type.
    transf << "0";
    int levels = ext->get_param_int("levels") + 1;
    const gchar *effecttype =  ext->get_param_enum("type");
    float val = 0.0;
    for ( int step = 1 ; step <= levels ; step++ ) {
        val = (float) step / levels;
        transf << " " << val;
        if((g_ascii_strcasecmp("dented", effecttype) == 0)) {
            transf << " " << (val - ((float) 1 / (3 * levels))) << " " << (val + ((float) 1 / (2 * levels)));
        }
    }
    transf << " 1";
    
    if (ext->get_param_bool("antialiasing"))
        antialias << "0.5";
    else
        antialias << "0.01";

    
	_filter = g_strdup_printf(
		"<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" height=\"1\" width=\"1\" y=\"0\" x=\"0\" inkscape:label=\"Poster and painting, custom -EXP-\">\n"
            "<feComposite result=\"Composite1\" operator=\"arithmetic\" k2=\"1\" />\n"
            "<feGaussianBlur stdDeviation=\"%s\" result=\"Gaussian1\" />\n"
            "<feGaussianBlur stdDeviation=\"%s\" in=\"Composite1\" />\n"
            "<feBlend in2=\"Gaussian1\" mode=\"%s\" />\n"
            "<feColorMatrix type=\"saturate\" values=\"%s\" />\n"
            "<feComponentTransfer>\n"
                "<feFuncR type=\"%s\" tableValues=\"%s\" />\n"
                "<feFuncG type=\"%s\" tableValues=\"%s\" />\n"
                "<feFuncB type=\"%s\" tableValues=\"%s\" />\n"
            "</feComponentTransfer>\n"
            "<feColorMatrix type=\"saturate\" values=\"%s\" />\n"
            "<feGaussianBlur stdDeviation=\"%s\" />\n"
            "<feComposite in2=\"SourceGraphic\" operator=\"atop\" />\n"
        "</filter>\n", blur1.str().c_str(), blur2.str().c_str(), blendmode.str().c_str(), presat.str().c_str(), table.str().c_str(), transf.str().c_str(), table.str().c_str(), transf.str().c_str(), table.str().c_str(), transf.str().c_str(), postsat.str().c_str(), antialias.str().c_str());

	return _filter;
};

class TestFilter : public Inkscape::Extension::Internal::Filter::Filter {
protected:
	virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
	TestFilter ( ) : Filter() { };
	virtual ~TestFilter ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

	static void init (void) {
		Inkscape::Extension::build_from_mem(
			"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
				"<name>" N_("Test Filter -EXP-") "</name>\n"
				"<id>org.inkscape.effect.filter.TestFilter</id>\n"
                "<_param name=\"header1\" type=\"groupheader\">Test filter</_param>\n"
				"<effect>\n"
					"<object-type>all</object-type>\n"
					"<effects-menu>\n"
						"<submenu name=\"" N_("Filters") "\">\n"
   						"<submenu name=\"" N_("Experimental") "\"/>\n"
			      "</submenu>\n"
					"</effects-menu>\n"
					"<menu-tip>" N_("Change colors to a two colors palette") "</menu-tip>\n"
				"</effect>\n"
			"</inkscape-extension>\n", new TestFilter());
	};

};

gchar const *
TestFilter::get_filter_text (Inkscape::Extension::Extension * ext)
{
	if (_filter != NULL) g_free((void *)_filter);
    
	_filter = g_strdup_printf(
		"<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" height=\"1\" width=\"1\" y=\"0\" x=\"0\" inkscape:label=\"Test Filter -EXP-\">\n"
            "<feComposite result=\"Composite1\" operator=\"arithmetic\" k2=\"1\" />\n"
            "<feGaussianBlur stdDeviation=\"4\" result=\"Gaussian1\" />\n"
            "<feGaussianBlur stdDeviation=\"0.5\" in=\"Composite1\" />\n"
            "<feBlend in2=\"Gaussian1\" mode=\"normal\" />\n"
            "<feColorMatrix type=\"saturate\" values=\"1\" />\n"
            "<feComponentTransfer>\n"
                "<feFuncR type=\"discrete\" tableValues=\"0 0.25 0.5 0.75 1 1\" />\n"
                "<feFuncG type=\"discrete\" tableValues=\"0 0.25 0.5 0.75 1 1\" />\n"
                "<feFuncB type=\"discrete\" tableValues=\"0 0.25 0.5 0.75 1 1\" />\n"
            "</feComponentTransfer>\n"
            "<feColorMatrix type=\"saturate\" values=\"1\" />\n"
            "<feGaussianBlur stdDeviation=\"0.05\" />\n"
            "<feComposite in2=\"SourceGraphic\" operator=\"atop\" />\n"
        "</filter>\n");

	return _filter;
};
}; /* namespace Filter */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */

/* Change the 'COLOR' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_EXPERIMENTAL_H__ */
