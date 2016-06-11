#ifndef SEEN_INKSCAPE_EXTENSION_INTERNAL_FILTER_TEXTURES_H__
#define SEEN_INKSCAPE_EXTENSION_INTERNAL_FILTER_TEXTURES_H__
/* Change the 'TEXTURES' above to be your file name */

/*
 * Copyright (C) 2011 Authors:
 *   Ivan Louette (filters)
 *   Nicolas Dufour (UI) <nicoduf@yahoo.fr>
 *
 * Protrusion filters
 *   Ink blot
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
    \brief    Custom predefined Ink Blot filter.
    
    Inkblot on tissue or rough paper.
    
    Filter's parameters:

    * Turbulence type (enum, default fractalNoise else turbulence) -> turbulence (type)
    * Frequency (0.001->1., default 0.04) -> turbulence (baseFrequency [/100])
    * Complexity (1->5, default 3) -> turbulence (numOctaves)
    * Variation (0->100, default 0) -> turbulence (seed)
    * Horizontal inlay (0.01->30., default 10) -> blur1 (stdDeviation x)
    * Vertical inlay (0.01->30., default 10) -> blur1 (stdDeviation y)
    * Displacement (0.->100., default 50) -> map (scale)
    * Blend (0.01->30., default 5) -> blur2 (stdDeviation)
    * Stroke (enum, default over) -> composite (operator)
    * Arithmetic stroke options
        * k1 (-10.->10., default 1.5)
        * k2 (-10.->10., default -0.25)
        * k3 (-10.->10., default 0.5)
*/
class InkBlot : public Inkscape::Extension::Internal::Filter::Filter {
protected:
	virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
	InkBlot ( ) : Filter() { };
	virtual ~InkBlot ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

public:
	static void init (void) {
		Inkscape::Extension::build_from_mem(
            "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
            "<name>" N_("Ink Blot") "</name>\n"
              "<id>org.inkscape.effect.filter.InkBlot</id>\n"
              "<param name=\"type\" _gui-text=\"" N_("Turbulence type:") "\" type=\"enum\">\n"
                "<_item value=\"fractalNoise\">Fractal noise</_item>\n"
                "<_item value=\"turbulence\">Turbulence</_item>\n"
              "</param>\n"
              "<param name=\"freq\" _gui-text=\"" N_("Frequency:") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.01\" max=\"100.00\">4</param>\n"
              "<param name=\"complexity\" _gui-text=\"" N_("Complexity:") "\" type=\"int\" appearance=\"full\" min=\"1\" max=\"5\">3</param>\n"
              "<param name=\"variation\" _gui-text=\"" N_("Variation:") "\" type=\"int\" appearance=\"full\" min=\"0\" max=\"100\">0</param>\n"
              "<param name=\"hblur\" _gui-text=\"" N_("Horizontal inlay:") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.01\" max=\"30.00\">10</param>\n"
              "<param name=\"vblur\" _gui-text=\"" N_("Vertical inlay:") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.01\" max=\"30.00\">10</param>\n"
              "<param name=\"displacement\" _gui-text=\"" N_("Displacement:") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.\" max=\"100.00\">50</param>\n"
              "<param name=\"blend\" _gui-text=\"" N_("Blend:") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.01\" max=\"30.00\">5</param>\n"
              "<param name=\"stroke\" _gui-text=\"" N_("Stroke:") "\" type=\"enum\">\n"
                "<_item value=\"over\">" N_("Wide") "</_item>\n"
                "<_item value=\"atop\">" N_("Normal") "</_item>\n"
                "<_item value=\"in\">" N_("Narrow") "</_item>\n"
                "<_item value=\"xor\">" N_("Overlapping") "</_item>\n"
                "<_item value=\"out\">" N_("External") "</_item>\n"
                "<_item value=\"arithmetic\">" N_("Custom") "</_item>\n"
              "</param>\n"
              "<_param name=\"customHeader\" type=\"description\" appearance=\"header\">" N_("Custom stroke options") "</_param>\n"
                "<param name=\"k1\" _gui-text=\"" N_("k1:") "\" type=\"float\" indent=\"1\" appearance=\"full\" precision=\"2\" min=\"-10.\" max=\"10.\">1.5</param>\n"
                "<param name=\"k2\" _gui-text=\"" N_("k2:") "\" type=\"float\" indent=\"1\" appearance=\"full\" precision=\"2\" min=\"-10.\" max=\"10.\">-0.25</param>\n"
                "<param name=\"k3\" _gui-text=\"" N_("k3:") "\" type=\"float\" indent=\"1\" appearance=\"full\" precision=\"2\" min=\"-10.\" max=\"10.\">0.5</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                     "<submenu name=\"Textures\"/>\n"
                  "</submenu>\n"
                "</effects-menu>\n"
                "<menu-tip>" N_("Inkblot on tissue or rough paper") "</menu-tip>\n"
              "</effect>\n"
            "</inkscape-extension>\n", new InkBlot());
	};

};

gchar const *
InkBlot::get_filter_text (Inkscape::Extension::Extension * ext)
{
	if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream type;
    std::ostringstream freq;
    std::ostringstream complexity;
    std::ostringstream variation;
    std::ostringstream hblur;
    std::ostringstream vblur;
    std::ostringstream displacement;
    std::ostringstream blend;
    std::ostringstream stroke;
    std::ostringstream custom;

    type << ext->get_param_enum("type");
    freq << ext->get_param_float("freq") / 100;
    complexity << ext->get_param_int("complexity");
    variation << ext->get_param_int("variation");
    hblur << ext->get_param_float("hblur");
    vblur << ext->get_param_float("vblur");
    displacement << ext->get_param_float("displacement");
    blend << ext->get_param_float("blend");

    const gchar *ope = ext->get_param_enum("stroke");
    if (g_ascii_strcasecmp("arithmetic", ope) == 0) {
        custom << "k1=\"" << ext->get_param_float("k1") << "\" k2=\"" << ext->get_param_float("k2") << "\" k3=\"" << ext->get_param_float("k3") << "\"";
    } else {
        custom << "";
    }

    stroke << ext->get_param_enum("stroke");

	_filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" style=\"color-interpolation-filters:sRGB;\" x=\"-0.15\" width=\"1.3\" y=\"-0.15\" height=\"1.3\" inkscape:label=\"Ink Blot\" >\n"
          "<feGaussianBlur in=\"SourceGraphic\" stdDeviation=\"%s %s\" result=\"blur1\" />\n"
          "<feTurbulence type=\"%s\" baseFrequency=\"%s\" numOctaves=\"%s\" seed=\"%s\" result=\"turbulence\" />\n"
          "<feDisplacementMap in=\"blur1\" in2=\"turbulence\" xChannelSelector=\"R\" yChannelSelector=\"G\" scale=\"%s\" result=\"map\" />\n"
          "<feGaussianBlur in=\"map\" stdDeviation=\"%s\" result=\"blur2\" />\n"
          "<feComposite in=\"blur2\" in2=\"map\" %s operator=\"%s\" result=\"composite\" />\n"
        "</filter>\n", hblur.str().c_str(), vblur.str().c_str(), type.str().c_str(),
                       freq.str().c_str(), complexity.str().c_str(), variation.str().c_str(),
                       displacement.str().c_str(), blend.str().c_str(),
                       custom.str().c_str(), stroke.str().c_str() );

	return _filter;

}; /* Ink Blot filter */


}; /* namespace Filter */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */

/* Change the 'TEXTURES' below to be your file name */
#endif /* SEEN_INKSCAPE_EXTENSION_INTERNAL_FILTER_TEXTURES_H__ */
