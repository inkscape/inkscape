#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_EXPERIMENTAL_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_EXPERIMENTAL_H__
/* Change the 'EXPERIMENTAL' above to be your file name */

/*
 * Copyright (C) 2010 Authors:
 *   Ivan Louette (filters)
 *   Nicolas Dufour (UI) <nicoduf@yahoo.fr>
 *
 * Experimental filters (no assigned menu)
 *   Chromolitho
 *   Drawing
 *   Posterize
 *   Test filter (should no be used...)
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
    \brief    Custom predefined Chromolitho filter.
    
    Chromo effect with customizable edge drawing and graininess

    Filter's parameters:
    * Drawing (boolean, default checked) -> Checked = blend1 (in="convolve1"), unchecked = blend1 (in="composite1")
    * Transparent (boolean, default unchecked) -> Checked = colormatrix5 (in="colormatrix4"), Unchecked = colormatrix5 (in="component1")
    * Invert (boolean, default false) -> component1 (tableValues) [adds a trailing 0]
    * Dented (boolean, default false) -> component1 (tableValues) [adds intermediate 0s]
    * Expand white (0->5, default 1) -> component1 (tableValues) [0="0 1", 5="0 1 1 1 1 1 1"]
    * Lightness (0->10, default 0) -> composite1 (k1)
    * Saturation (0.->1., default 1.) -> colormatrix3 (values)
    * Noise reduction (1->1000, default 20) -> convolve (kernelMatrix, central value -1001->-2000, default -1020)
    * Drawing blend (enum, default Normal) -> blend1 (mode)
    * Smoothness (0.01->10, default 1) -> blur1 (stdDeviation)
    * Grain (boolean, default unchecked) -> Checked = blend2 (in="colormatrix2"), Unchecked = blend2 (in="blur1")
        * Grain x frequency (0.->100, default 100) -> turbulence1 (baseFrequency, first value)
        * Grain y frequency (0.->100, default 100) -> turbulence1 (baseFrequency, second value)
        * Grain complexity (1->5, default 1) -> turbulence1 (numOctaves)
        * Grain variation (0->1000, default 0) -> turbulence1 (seed)
        * Grain expansion (1.->50., default 1.) -> colormatrix1 (n-1 value)
        * Grain erosion (0.->40., default 0.) -> colormatrix1 (nth value) [inverted]
        * Grain color (boolean, default true) -> colormatrix2 (values)
        * Grain blend (enum, default Normal) -> blend2 (mode)
*/
class Chromolitho : public Inkscape::Extension::Internal::Filter::Filter {
protected:
	virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
	Chromolitho ( ) : Filter() { };
	virtual ~Chromolitho ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

	static void init (void) {
		Inkscape::Extension::build_from_mem(
			"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
				"<name>" N_("Chromolitho, custom -EXP-") "</name>\n"
				"<id>org.inkscape.effect.filter.Chromolitho</id>\n"
	            "<param name=\"tab\" type=\"notebook\">\n"
                    "<page name=\"optionstab\" _gui-text=\"Options\">\n"
                        "<param name=\"drawing\" gui-text=\"" N_("Drawing mode") "\" type=\"boolean\" >true</param>\n"
                        "<param name=\"dblend\" gui-text=\"" N_("Drawing blend:") "\" type=\"enum\">\n"
                            "<_item value=\"normal\">Normal</_item>\n"
                            "<_item value=\"multiply\">Multiply</_item>\n"
                            "<_item value=\"screen\">Screen</_item>\n"
                            "<_item value=\"lighten\">Lighten</_item>\n"
                            "<_item value=\"darken\">Darken</_item>\n"
                        "</param>\n"
                        "<param name=\"transparent\" gui-text=\"" N_("Transparent") "\" type=\"boolean\" >false</param>\n"
                        "<param name=\"dented\" gui-text=\"" N_("Dented") "\" type=\"boolean\" >false</param>\n"
                        "<param name=\"inverted\" gui-text=\"" N_("Inverted") "\" type=\"boolean\" >false</param>\n"
                        "<param name=\"light\" gui-text=\"" N_("Lightness:") "\" type=\"int\" min=\"0\" max=\"10\">0</param>\n"
                        "<param name=\"saturation\" gui-text=\"" N_("Saturation:") "\" type=\"float\" min=\"0\" max=\"1\">1</param>\n"
                        "<param name=\"noise\" gui-text=\"" N_("Noise reduction:") "\" type=\"int\" min=\"1\" max=\"1000\">20</param>\n"
                        "<param name=\"smooth\" gui-text=\"" N_("Smoothness:") "\" type=\"float\" min=\"0.01\" max=\"10\">1</param>\n"
                    "</page>\n"
                    "<page name=\"graintab\" _gui-text=\"Grain\">\n"
                        "<param name=\"grain\" gui-text=\"" N_("Grain mode") "\" type=\"boolean\" >true</param>\n"
                        "<param name=\"grainxf\" gui-text=\"" N_("X frequency:") "\" type=\"float\" min=\"0\" max=\"100\">100</param>\n"
                        "<param name=\"grainyf\" gui-text=\"" N_("Y frequency:") "\" type=\"float\" min=\"0\" max=\"100\">100</param>\n"
                        "<param name=\"grainc\" gui-text=\"" N_("Complexity:") "\" type=\"int\" min=\"1\" max=\"5\">1</param>\n"
                        "<param name=\"grainv\" gui-text=\"" N_("Variation:") "\" type=\"int\" min=\"0\" max=\"1000\">0</param>\n"
                        "<param name=\"grainexp\" gui-text=\"" N_("Expansion:") "\" type=\"float\" min=\"1\" max=\"50\">1</param>\n"
                        "<param name=\"grainero\" gui-text=\"" N_("Erosion:") "\" type=\"float\" min=\"0\" max=\"40\">0</param>\n"
                        "<param name=\"graincol\" gui-text=\"" N_("Color") "\" type=\"boolean\" >true</param>\n"
                        "<param name=\"gblend\" gui-text=\"" N_("Grain blend:") "\" type=\"enum\">\n"
                            "<_item value=\"normal\">Normal</_item>\n"
                            "<_item value=\"multiply\">Multiply</_item>\n"
                            "<_item value=\"screen\">Screen</_item>\n"
                            "<_item value=\"lighten\">Lighten</_item>\n"
                            "<_item value=\"darken\">Darken</_item>\n"
                        "</param>\n"
                    "</page>\n"
                "</param>\n"
				"<effect>\n"
					"<object-type>all</object-type>\n"
					"<effects-menu>\n"
						"<submenu name=\"" N_("Filters") "\">\n"
   						"<submenu name=\"" N_("Experimental") "\"/>\n"
			      "</submenu>\n"
					"</effects-menu>\n"
					"<menu-tip>" N_("Chromo effect with customizable edge drawing and graininess") "</menu-tip>\n"
				"</effect>\n"
			"</inkscape-extension>\n", new Chromolitho());
	};
};

gchar const *
Chromolitho::get_filter_text (Inkscape::Extension::Extension * ext)
{
	if (_filter != NULL) g_free((void *)_filter);
    
    std::ostringstream b1in;
    std::ostringstream b2in;
    std::ostringstream col3in;
    std::ostringstream transf;
    std::ostringstream light;
    std::ostringstream saturation;
    std::ostringstream noise;
    std::ostringstream dblend;
    std::ostringstream smooth;
    std::ostringstream grain;
    std::ostringstream grainxf;
    std::ostringstream grainyf;
    std::ostringstream grainc;
    std::ostringstream grainv;
    std::ostringstream gblend;
    std::ostringstream grainexp;
    std::ostringstream grainero;
    std::ostringstream graincol;

    if (ext->get_param_bool("drawing"))
        b1in << "convolve1";
    else
        b1in << "composite1";

    if (ext->get_param_bool("transparent"))
        col3in << "colormatrix4";
    else
        col3in << "component1";
    light << ext->get_param_int("light");
    saturation << ext->get_param_float("saturation");
    noise << (-1000 - ext->get_param_int("noise"));
    dblend << ext->get_param_enum("dblend");
    smooth << ext->get_param_float("smooth");

    if (ext->get_param_bool("dented")) {
        transf << "0 1 0 1";
    } else {
        transf << "0 1 1";
    }
    if (ext->get_param_bool("inverted"))
        transf << " 0";

    if (ext->get_param_bool("grain"))
        b2in << "colormatrix2";
    else
        b2in << "blur1";
    grainxf << (ext->get_param_float("grainxf") / 100);
    grainyf << (ext->get_param_float("grainyf") / 100);
    grainc << ext->get_param_int("grainc");
    grainv << ext->get_param_int("grainv");
    gblend << ext->get_param_enum("gblend");
    grainexp << ext->get_param_float("grainexp");
    grainero << (-ext->get_param_float("grainero"));
    if (ext->get_param_bool("graincol"))
        graincol << "1";
    else
        graincol << "0";

	_filter = g_strdup_printf(
		"<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" height=\"1\" width=\"1\" y=\"0\" x=\"0\" inkscape:label=\"Chromolitho, custom -EXP-\">\n"

        "<feComposite stdDeviation=\"1\" in=\"SourceGraphic\" in2=\"SourceGraphic\" operator=\"arithmetic\" k1=\"%s\" k2=\"1\" result=\"composite1\" />\n"
        "<feConvolveMatrix in=\"composite1\" kernelMatrix=\"0 250 0 250 %s 250 0 250 0 \" order=\"3 3\" stdDeviation=\"1\" result=\"convolve1\" />\n"
        "<feBlend in=\"%s\" in2=\"composite1\" mode=\"%s\" blend=\"normal\" stdDeviation=\"1\" result=\"blend1\" />\n"
        "<feGaussianBlur in=\"blend1\" stdDeviation=\"%s\" result=\"blur1\" />\n"
        "<feTurbulence baseFrequency=\"%s %s\" numOctaves=\"%s\" seed=\"%s\" type=\"fractalNoise\" result=\"turbulence1\" />\n"
        "<feColorMatrix values=\"1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 0 0 0 %s %s \" result=\"colormatrix1\" />\n"
        "<feColorMatrix type=\"saturate\" stdDeviation=\"3\" values=\"%s\" result=\"colormatrix2\" />\n"
        "<feBlend in=\"%s\" in2=\"blur1\" stdDeviation=\"1\" blend=\"normal\" mode=\"%s\" result=\"blend2\" />\n"
        "<feColorMatrix in=\"blend2\" type=\"saturate\" values=\"%s\" result=\"colormatrix3\" />\n"
        "<feComponentTransfer in=\"colormatrix3\" stdDeviation=\"2\" result=\"component1\">\n"
            "<feFuncR type=\"discrete\" tableValues=\"%s\" />\n"
            "<feFuncG type=\"discrete\" tableValues=\"%s\" />\n"
            "<feFuncB type=\"discrete\" tableValues=\"%s\" />\n"
        "</feComponentTransfer>\n"
        "<feColorMatrix values=\"1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 -0.2125 -0.7154 -0.0721 1 0 \" result=\"colormatrix4\" />\n"
        "<feColorMatrix in=\"%s\" values=\"1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 0 0 0 15 0 \" result=\"colormatrix5\" />\n"
        "<feComposite in2=\"SourceGraphic\" operator=\"in\" result=\"composite2\" />\n"
        "</filter>\n", light.str().c_str(), noise.str().c_str(), b1in.str().c_str(), dblend.str().c_str(), smooth.str().c_str(), grainxf.str().c_str(), grainyf.str().c_str(), grainc.str().c_str(), grainv.str().c_str(), grainexp.str().c_str(), grainero.str().c_str(), graincol.str().c_str(), b2in.str().c_str(), gblend.str().c_str(), saturation.str().c_str(), transf.str().c_str(), transf.str().c_str(), transf.str().c_str(), col3in.str().c_str());

	return _filter;
}; /* Chromolitho filter */

/**
    \brief    Custom predefined Drawing filter.
    
    Convert images to duochrome drawings.

    Filter's parameters:
    * Simplification (0.01->10, default 0.7) -> blur1 (stdDeviation)
    * Lightness (0->50, default 5) -> convolve (kernelMatrix, central value -1000->-1050, default -1005)
    * Smoothness (0.01->10, default 0.7) -> blur2 (stdDeviation)
    * Dilatation (3->100, default 6) -> colormatrix3 (n-1th value)

    * Blur (0.01->10., default 1.) -> blur3 (stdDeviation)
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
	            "<param name=\"tab\" type=\"notebook\">\n"
                    "<page name=\"optionstab\" _gui-text=\"Options\">\n"
                        "<param name=\"simply\" gui-text=\"" N_("Simplification:") "\" type=\"float\" min=\"0.01\" max=\"10\">0.7</param>\n"
                        "<param name=\"light\" gui-text=\"" N_("Lightness:") "\" type=\"int\" min=\"0\" max=\"50\">5</param>\n"
                        "<param name=\"smooth\" gui-text=\"" N_("Smoothness:") "\" type=\"float\" min=\"0.01\" max=\"10\">0.7</param>\n"
                        "<param name=\"dilat\" gui-text=\"" N_("Dilatation:") "\" type=\"int\" min=\"3\" max=\"100\">6</param>\n"
                        "<_param name=\"blurheader\" type=\"groupheader\">Blur</_param>\n"
                            "<param name=\"blur\" gui-text=\"" N_("Level:") "\" type=\"float\" min=\"0.01\" max=\"10\">1</param>\n"
                            "<param name=\"spread\" gui-text=\"" N_("Spread:") "\" type=\"int\" min=\"3\" max=\"20\">6</param>\n"
                            "<param name=\"erosion\" gui-text=\"" N_("Erosion:") "\" type=\"int\" min=\"-2\" max=\"0\">-2</param>\n"
                    "</page>\n"
                    "<page name=\"co11tab\" _gui-text=\"Fill color\">\n"
			            "<param name=\"fcolor\" gui-text=\"" N_("Fill color") "\" type=\"color\">-3473153</param>\n"
                        "<param name=\"iof\" gui-text=\"" N_("Image on fill") "\" type=\"boolean\" >false</param>\n"
                        "<param name=\"iofo\" gui-text=\"" N_("Image on fill opacity:") "\" type=\"float\" min=\"0\" max=\"1\">1</param>\n"
                    "</page>\n"
                    "<page name=\"co12tab\" _gui-text=\"Stroke color\">\n"
			            "<param name=\"scolor\" gui-text=\"" N_("Stroke color") "\" type=\"color\">-855637761</param>\n"
                        "<param name=\"ios\" gui-text=\"" N_("Image on stroke") "\" type=\"boolean\" >false</param>\n"
                        "<param name=\"ioso\" gui-text=\"" N_("Image on stroke opacity:") "\" type=\"float\" min=\"0\" max=\"1\">1</param>\n"
                    "</page>\n"
                "</param>\n"
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
		"<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" height=\"1\" width=\"1\" y=\"0\" x=\"0\" inkscape:label=\"Drawing, custom -EXP-\">\n"
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

    Filter's parameters (not finished yet):
    * Effect type (enum, default "Normal") ->
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
}; /* Test filter */

}; /* namespace Filter */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */

/* Change the 'COLOR' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_EXPERIMENTAL_H__ */
