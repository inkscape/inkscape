#ifndef SEEN_INKSCAPE_EXTENSION_INTERNAL_FILTER_DISTORT_H__
#define SEEN_INKSCAPE_EXTENSION_INTERNAL_FILTER_DISTORT_H__
/* Change the 'DISTORT' above to be your file name */

/*
 * Copyright (C) 2011 Authors:
 *   Ivan Louette (filters)
 *   Nicolas Dufour (UI) <nicoduf@yahoo.fr>
 *
 * Distort filters
 *   Felt Feather
 *   Roughen
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
    \brief    Custom predefined FeltFeather filter.
    
    Blur and displace edges of shapes and pictures
    
    Filter's parameters:
    * Type (enum, default "In") ->
        in = map (in="composite3")
        out = map (in="blur")
    * Horizontal blur (0.01->30., default 15) -> blur (stdDeviation)
    * Vertical blur (0.01->30., default 15) -> blur (stdDeviation)
    * Dilatation (n-1th value, 0.->100., default 1) -> colormatrix (matrix)
    * Erosion (nth value, 0.->100., default 0) -> colormatrix (matrix)
    * Stroke (enum, default "Normal") ->
        Normal = composite4 (operator="atop")
        Wide = composite4 (operator="over")
        Narrow = composite4 (operator="in")
        No fill = composite4 (operator="xor")
    * Roughness (group)  
    * Turbulence type (enum, default fractalNoise else turbulence) -> turbulence (type)
    * Horizontal frequency (0.001->1., default 0.05) -> turbulence (baseFrequency [/100])
    * Vertical frequency (0.001->1., default 0.05) -> turbulence (baseFrequency [/100])
    * Complexity (1->5, default 3) -> turbulence (numOctaves)
    * Variation (0->100, default 0) -> turbulence (seed)
    * Intensity (0.0->100., default 30) -> displacement (scale)
*/

class FeltFeather : public Inkscape::Extension::Internal::Filter::Filter {
protected:
    virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
    FeltFeather ( ) : Filter() { };
    virtual ~FeltFeather ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

    static void init (void) {
        Inkscape::Extension::build_from_mem(
            "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
              "<name>" N_("Felt Feather") "</name>\n"
              "<id>org.inkscape.effect.filter.FeltFeather</id>\n"
              "<param name=\"type\" _gui-text=\"" N_("Type:") "\" type=\"enum\">\n"
                "<_item value=\"in\">" N_("In") "</_item>\n"
                "<_item value=\"out\">" N_("Out") "</_item>\n"
              "</param>\n"
              "<param name=\"hblur\" _gui-text=\"" N_("Horizontal blur") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.01\" max=\"30.00\">15</param>\n"
              "<param name=\"vblur\" _gui-text=\"" N_("Vertical blur") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.01\" max=\"30.00\">15</param>\n"
              "<param name=\"dilat\" _gui-text=\"" N_("Dilatation") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0\" max=\"100\">1</param>\n"
              "<param name=\"erosion\" _gui-text=\"" N_("Erosion") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0\" max=\"100\">0</param>\n"
              "<param name=\"stroke\" _gui-text=\"" N_("Stroke:") "\" type=\"enum\">\n"
                "<_item value=\"atop\">" N_("Normal") "</_item>\n"
                "<_item value=\"over\">" N_("Wide") "</_item>\n"
                "<_item value=\"in\">" N_("Narrow") "</_item>\n"
                "<_item value=\"xor\">" N_("No fill") "</_item>\n"
              "</param>\n"
              "<param name=\"turbulence\" indent=\"1\" _gui-text=\"" N_("Turbulence:") "\" type=\"enum\">\n"
                "<_item value=\"fractalNoise\">" N_("Fractal noise") "</_item>\n"
                "<_item value=\"turbulence\">" N_("Turbulence") "</_item>\n"
              "</param>\n"
              "<param name=\"hfreq\" _gui-text=\"" N_("Horizontal frequency") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.01\" max=\"100.\">5</param>\n"
              "<param name=\"vfreq\" _gui-text=\"" N_("Vertical frequency") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.01\" max=\"100.\">5</param>\n"
              "<param name=\"complexity\" _gui-text=\"" N_("Complexity") "\" type=\"int\" appearance=\"full\" min=\"1\" max=\"5\">3</param>\n"
              "<param name=\"variation\" _gui-text=\"" N_("Variation") "\" type=\"int\" appearance=\"full\" min=\"1\" max=\"100\">0</param>\n"
              "<param name=\"intensity\" _gui-text=\"" N_("Intensity") "\" type=\"float\" appearance=\"full\" min=\"0.0\" max=\"100\">30</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Distort") "\"/>\n"
                  "</submenu>\n"
                "</effects-menu>\n"
                "<menu-tip>" N_("Blur and displace edges of shapes and pictures") "</menu-tip>\n"
              "</effect>\n"
            "</inkscape-extension>\n", new FeltFeather());
    };

};

gchar const *
FeltFeather::get_filter_text (Inkscape::Extension::Extension * ext)
{
    if (_filter != NULL) g_free((void *)_filter);


    std::ostringstream hblur;
    std::ostringstream vblur;
    std::ostringstream dilat;
    std::ostringstream erosion;

    std::ostringstream turbulence;
    std::ostringstream hfreq;
    std::ostringstream vfreq;
    std::ostringstream complexity;
    std::ostringstream variation;
    std::ostringstream intensity;

    std::ostringstream map;
    std::ostringstream stroke;
    
    hblur << ext->get_param_float("hblur");
    vblur << ext->get_param_float("vblur");
    dilat << ext->get_param_float("dilat");
    erosion << -ext->get_param_float("erosion");
    
    turbulence << ext->get_param_enum("turbulence");
    hfreq << ext->get_param_float("hfreq") / 100;
    vfreq << ext->get_param_float("vfreq") / 100;
    complexity << ext->get_param_int("complexity");
    variation << ext->get_param_int("variation");
    intensity << ext->get_param_float("intensity");

    stroke << ext->get_param_enum("stroke");
    
    const gchar *maptype = ext->get_param_enum("type");
    if (g_ascii_strcasecmp("in", maptype) == 0) {
        map << "composite3";
    } else {
        map << "blur";
    }

    _filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" x=\"-0.3\" width=\"1.6\" y=\"-0.3\" height=\"1.6\" style=\"color-interpolation-filters:sRGB;\" inkscape:label=\"Felt Feather\">\n"
          "<feGaussianBlur stdDeviation=\"%s %s\" result=\"blur\" />\n"
          "<feComposite in=\"SourceGraphic\" in2=\"blur\" operator=\"atop\" result=\"composite1\" />\n"
          "<feComposite in2=\"composite1\" operator=\"in\" result=\"composite2\" />\n"
          "<feComposite in2=\"composite2\" operator=\"in\" result=\"composite3\" />\n"
          "<feTurbulence type=\"%s\" numOctaves=\"%s\" seed=\"%s\" baseFrequency=\"%s %s\" result=\"turbulence\" />\n"
          "<feDisplacementMap in=\"%s\" in2=\"turbulence\" xChannelSelector=\"R\" scale=\"%s\" yChannelSelector=\"G\" result=\"map\" />\n"
          "<feColorMatrix values=\"1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 0 0 0 %s %s \" result=\"colormatrix\" />\n"
          "<feComposite in=\"composite3\" in2=\"colormatrix\" operator=\"%s\" result=\"composite4\" />\n"
        "</filter>\n", hblur.str().c_str(), vblur.str().c_str(), 
                       turbulence.str().c_str(), complexity.str().c_str(), variation.str().c_str(), hfreq.str().c_str(), vfreq.str().c_str(),
                       map.str().c_str(), intensity.str().c_str(), dilat.str().c_str(), erosion.str().c_str(), stroke.str().c_str() );

    return _filter;
}; /* Felt feather filter */

/**
    \brief    Custom predefined Roughen filter.
    
    Small-scale roughening to edges and content

    Filter's parameters:
    * Turbulence type (enum, default fractalNoise else turbulence) -> turbulence (type)
    * Horizontal frequency (0.001->10., default 0.013) -> turbulence (baseFrequency [/100])
    * Vertical frequency (0.001->10., default 0.013) -> turbulence (baseFrequency [/100])
    * Complexity (1->5, default 5) -> turbulence (numOctaves)
    * Variation (1->360, default 1) -> turbulence (seed)
    * Intensity (0.0->50., default 6.6) -> displacement (scale)
*/

class Roughen : public Inkscape::Extension::Internal::Filter::Filter {
protected:
    virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
    Roughen ( ) : Filter() { };
    virtual ~Roughen ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

    static void init (void) {
        Inkscape::Extension::build_from_mem(
            "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
              "<name>" N_("Roughen") "</name>\n"
              "<id>org.inkscape.effect.filter.Roughen</id>\n"
              "<param name=\"type\" _gui-text=\"" N_("Turbulence type:") "\" type=\"enum\">\n"
                "<_item value=\"fractalNoise\">" N_("Fractal noise") "</_item>\n"
                "<_item value=\"turbulence\">" N_("Turbulence") "</_item>\n"
              "</param>\n"
              "<param name=\"hfreq\" _gui-text=\"" N_("Horizontal frequency") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.1\" max=\"1000.00\">1.3</param>\n"
              "<param name=\"vfreq\" _gui-text=\"" N_("Vertical frequency") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.1\" max=\"1000.00\">1.3</param>\n"
              "<param name=\"complexity\" _gui-text=\"" N_("Complexity") "\" type=\"int\" appearance=\"full\" min=\"1\" max=\"5\">5</param>\n"
              "<param name=\"variation\" _gui-text=\"" N_("Variation") "\" type=\"int\" appearance=\"full\" min=\"1\" max=\"360\">0</param>\n"
              "<param name=\"intensity\" _gui-text=\"" N_("Intensity") "\" type=\"float\" appearance=\"full\" min=\"0.0\" max=\"50\">6.6</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Distort") "\"/>\n"
                  "</submenu>\n"
                "</effects-menu>\n"
                "<menu-tip>" N_("Small-scale roughening to edges and content") "</menu-tip>\n"
              "</effect>\n"
            "</inkscape-extension>\n", new Roughen());
    };

};

gchar const *
Roughen::get_filter_text (Inkscape::Extension::Extension * ext)
{
    if (_filter != NULL) g_free((void *)_filter);
  
    std::ostringstream type;
    std::ostringstream hfreq;
    std::ostringstream vfreq;
    std::ostringstream complexity;
    std::ostringstream variation;
    std::ostringstream intensity;
    
    type << ext->get_param_enum("type");
    hfreq << ext->get_param_float("hfreq") / 100;
    vfreq << ext->get_param_float("vfreq") / 100;
    complexity << ext->get_param_int("complexity");
    variation << ext->get_param_int("variation");
    intensity << ext->get_param_float("intensity");

    _filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" style=\"color-interpolation-filters:sRGB;\" inkscape:label=\"Roughen\">\n"
          "<feTurbulence  type=\"%s\" numOctaves=\"%s\" seed=\"%s\" baseFrequency=\"%s %s\" result=\"turbulence\" />\n"
          "<feDisplacementMap in=\"SourceGraphic\" in2=\"turbulence\" scale=\"%s\" yChannelSelector=\"G\" xChannelSelector=\"R\" />\n"
        "</filter>\n", type.str().c_str(), complexity.str().c_str(), variation.str().c_str(), hfreq.str().c_str(), vfreq.str().c_str(), intensity.str().c_str());

    return _filter;
}; /* Roughen filter */

}; /* namespace Filter */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */

/* Change the 'DISTORT' below to be your file name */
#endif /* SEEN_INKSCAPE_EXTENSION_INTERNAL_FILTER_DISTORT_H__ */
