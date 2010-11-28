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

/**
    \brief    Custom predefined Drawing filter.
    
    Convert images to duochrome drawings.

    Filter's parameters:
    * Simplification (0.01->10, default 0.7) -> blur1 (stdDeviation)
    * Lightness (0->50, default 5) -> convolve (kernelMatrix, central value -1000->-1050, default -1005)
    * Smoothness (0.01->10, default 0.7) -> blur2 (stdDeviation)
    * Dilatation (3->100, default 6) -> colormatrix3 (n-1th value)

    * Blur (0.01->10., default 5.) -> blur3 (stdDeviation)
    * Blur spread (3->20, default 6) -> colormatrix5 (n-1th value)
    * Blur erosion (-2->0, default -2) -> colormatrix5 (nth value)

    * Stroke color (guint, default 205,0,0) -> flood2 (flood-opacity, flood-color)
    * Image on stroke (boolean, default false) -> composite1 (in="flood2" true-> in="SourceGraphic")
    * Image on stroke opacity (0.->1., default 1) -> composite3 (k3)
    * Fill color (guint, default 255,203,0) -> flood3 (flood-opacity, flood-color)
    * Image on fill (boolean, default false) -> composite2 (in="flood3" true-> in="SourceGraphic")
    * Image on fill opacity (0.->1., default 1) -> composite3 (k2)
*/

class Drawing : public Inkscape::Extension::Internal::Filter::Filter {
protected:
	virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
	Drawing ( ) : Filter() { };
	virtual ~Drawing ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

	static void init (void) {
		Inkscape::Extension::build_from_mem(
			"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
				"<name>" N_("Drawing, custom -EXP-") "</name>\n"
				"<id>org.inkscape.effect.filter.Drawing</id>\n"
                        "<param name=\"simply\" gui-text=\"" N_("Simplification:") "\" type=\"float\" min=\"0.01\" max=\"10\">0.7</param>\n"
                        "<param name=\"light\" gui-text=\"" N_("Lightness:") "\" type=\"int\" min=\"0\" max=\"50\">5</param>\n"
                        "<param name=\"smooth\" gui-text=\"" N_("Smoothness:") "\" type=\"float\" min=\"0.01\" max=\"10\">0.7</param>\n"
                        "<param name=\"dilat\" gui-text=\"" N_("Dilatation:") "\" type=\"int\" min=\"3\" max=\"100\">6</param>\n"
                        "<param name=\"blur\" gui-text=\"" N_("Blur:") "\" type=\"float\" min=\"0.01\" max=\"10\">5</param>\n"
                        "<param name=\"spread\" gui-text=\"" N_("Spread:") "\" type=\"int\" min=\"3\" max=\"20\">6</param>\n"
                        "<param name=\"erosion\" gui-text=\"" N_("Erosion:") "\" type=\"int\" min=\"-2\" max=\"0\">-2</param>\n"
                        "<_param name=\"fillcolorheader\" type=\"groupheader\">Fill color</_param>\n"
				            "<param name=\"fcolor\" gui-text=\"" N_("Fill color") "\" type=\"color\">-3473153</param>\n"
                            "<param name=\"iof\" gui-text=\"" N_("Image on fill") "\" type=\"boolean\" >false</param>\n"
                            "<param name=\"iofo\" gui-text=\"" N_("Image on fill opacity:") "\" type=\"float\" min=\"0\" max=\"1\">1</param>\n"
                        "<_param name=\"strokecolorheader\" type=\"groupheader\">Stroke color</_param>\n"
				            "<param name=\"scolor\" gui-text=\"" N_("Stroke color") "\" type=\"color\">-855637761</param>\n"
                            "<param name=\"ios\" gui-text=\"" N_("Image on stroke") "\" type=\"boolean\" >false</param>\n"
                            "<param name=\"ioso\" gui-text=\"" N_("Image on stroke opacity:") "\" type=\"float\" min=\"0\" max=\"1\">1</param>\n"
				"<effect>\n"
					"<object-type>all</object-type>\n"
					"<effects-menu>\n"
						"<submenu name=\"" N_("Filters") "\">\n"
   						"<submenu name=\"" N_("Experimental") "\"/>\n"
			      "</submenu>\n"
					"</effects-menu>\n"
					"<menu-tip>" N_("Convert images to duochrome drawings") "</menu-tip>\n"
				"</effect>\n"
			"</inkscape-extension>\n", new Drawing());
	};

};

gchar const *
Drawing::get_filter_text (Inkscape::Extension::Extension * ext)
{
	if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream simply;
    std::ostringstream light;
    std::ostringstream smooth;
    std::ostringstream dilat;
    std::ostringstream blur;
    std::ostringstream spread;
    std::ostringstream erosion;
    std::ostringstream strokea;
    std::ostringstream stroker;
    std::ostringstream strokeg;
    std::ostringstream strokeb;
    std::ostringstream ios;
    std::ostringstream ioso;
    std::ostringstream filla;
    std::ostringstream fillr;
    std::ostringstream fillg;
    std::ostringstream fillb;
    std::ostringstream iof;
    std::ostringstream iofo;

    simply << ext->get_param_float("simply");
    light << (-1000 - ext->get_param_int("light"));
    smooth << ext->get_param_float("smooth");
    dilat << ext->get_param_int("dilat");

    blur << ext->get_param_float("blur");
    spread << ext->get_param_int("spread");
    erosion << ext->get_param_int("erosion");

    guint32 fcolor = ext->get_param_color("fcolor");
    fillr << ((fcolor >> 24) & 0xff);
    fillg << ((fcolor >> 16) & 0xff);
    fillb << ((fcolor >>  8) & 0xff);
    filla << (fcolor & 0xff) / 255.0F;
    if (ext->get_param_bool("iof"))
        iof << "SourceGraphic";
    else
        iof << "flood3";
    iofo << ext->get_param_float("iofo");

    guint32 scolor = ext->get_param_color("scolor");
    stroker << ((scolor >> 24) & 0xff);
    strokeg << ((scolor >> 16) & 0xff);
    strokeb << ((scolor >>  8) & 0xff);
    strokea << (scolor & 0xff) / 255.0F;
    if (ext->get_param_bool("ios"))
        ios << "SourceGraphic";
    else
        ios << "flood2";
    ioso << ext->get_param_float("ioso");

	_filter = g_strdup_printf(
		"<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" height=\"1\" width=\"1\" y=\"0\" x=\"0\" inkscape:label=\"Cross-smooth, custom -EXP-\">\n"
        "<feGaussianBlur in=\"SourceGraphic\" stdDeviation=\"%s\" result=\"blur1\" />\n"
        "<feConvolveMatrix in=\"blur1\" order=\"3 3\" kernelMatrix=\"0 250 0 250 %s 250 0 250 0 \" divisor=\"1\" targetX=\"1\" targetY=\"1\" preserveAlpha=\"true\" bias=\"0\" stdDeviation=\"1\" result=\"convolve\" />\n"
        "<feColorMatrix values=\"0 -100 0 0 1 0 -100 0 0 1 0 -100 0 0 1 0 0 0 1 0 \" result=\"colormatrix1\" />\n"
        "<feColorMatrix in=\"colormatrix1\" values=\"0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -0.2125 -0.7154 -0.0721 1 0 \" result=\"colormatrix2\" />\n"
        "<feGaussianBlur stdDeviation=\"%s\" result=\"blur2\" />\n"
        "<feColorMatrix values=\"1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 0 0 0 %s -2 \" result=\"colormatrix3\" />\n"
        "<feFlood flood-color=\"rgb(255,255,255)\" result=\"flood1\" />\n"
        "<feBlend in2=\"colormatrix3\" blend=\"normal\" mode=\"multiply\" result=\"blend1\" />\n"
        "<feComponentTransfer in=\"blend1\" result=\"component1\">\n"
            "<feFuncR tableValues=\"0 1 1\" type=\"discrete\" />\n"
            "<feFuncG tableValues=\"0 1 1\" type=\"discrete\" />\n"
            "<feFuncB tableValues=\"0 1 1\" type=\"discrete\" />\n"
        "</feComponentTransfer>\n"
        "<feGaussianBlur stdDeviation=\"%s\" result=\"blur3\" />\n"
        "<feColorMatrix in=\"blur3\" values=\"0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -0.2125 -0.7154 -0.0721 1 0 \" result=\"colormatrix4\" />\n"
        "<feColorMatrix stdDeviation=\"3\" values=\"1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 0 0 0 %s %s \" result=\"colormatrix5\" />\n"
        "<feColorMatrix in=\"colormatrix5\" type=\"saturate\" values=\"1\" result=\"colormatrix6\" />\n"
        "<feFlood flood-opacity=\"%s\" flood-color=\"rgb(%s,%s,%s)\" stdDeviation=\"3\" result=\"flood2\" />\n"
        "<feComposite in=\"%s\" in2=\"colormatrix6\" operator=\"in\" result=\"composite1\" />\n"
        "<feFlood flood-opacity=\"%s\" in=\"colormatrix6\" flood-color=\"rgb(%s,%s,%s)\" result=\"flood3\" />\n"
        "<feComposite in=\"%s\" in2=\"colormatrix6\" operator=\"out\" result=\"composite2\" />\n"
        "<feComposite in2=\"composite1\" operator=\"arithmetic\" k2=\"%s\" k3=\"%s\" result=\"composite3\" />\n"
        "<feComposite in2=\"SourceGraphic\" operator=\"in\" />\n"
        "</filter>\n", simply.str().c_str(), light.str().c_str(), smooth.str().c_str(), dilat.str().c_str(), blur.str().c_str(), spread.str().c_str(), erosion.str().c_str(), strokea.str().c_str(), stroker.str().c_str(), strokeg.str().c_str(), strokeb.str().c_str(), ios.str().c_str(), filla.str().c_str(), fillr.str().c_str(), fillg.str().c_str(), fillb.str().c_str(), iof.str().c_str(), iofo.str().c_str(), ioso.str().c_str());

	return _filter;
}; /* Drawing filter */

/**
    \brief    Custom predefined Posterize filter.
    
    Poster and painting effects.

    Filter's parameters:
    * Type (enum, default "Normal") ->
        Normal = feComponentTransfer
        Dented = Normal + intermediate values
    * Blur (0.01->10., default 5.) -> blur3 (stdDeviation)

*/
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
                    "<_item value=\"lighten\">Lighten</_item>\n"
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


    // TransfertComponent table values are calculated based on the poster type.
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
            "<feComposite in2=\"SourceGraphic\" operator=\"in\" />\n"
        "</filter>\n", blur1.str().c_str(), blur2.str().c_str(), blendmode.str().c_str(), presat.str().c_str(), table.str().c_str(), transf.str().c_str(), table.str().c_str(), transf.str().c_str(), table.str().c_str(), transf.str().c_str(), postsat.str().c_str(), antialias.str().c_str());

	return _filter;
}; /* Posterize filter */


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
