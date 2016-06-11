#ifndef SEEN_INKSCAPE_EXTENSION_INTERNAL_FILTER_OVERLAYS_H__
#define SEEN_INKSCAPE_EXTENSION_INTERNAL_FILTER_OVERLAYS_H__
/* Change the 'OVERLAYS' above to be your file name */

/*
 * Copyright (C) 2011 Authors:
 *   Ivan Louette (filters)
 *   Nicolas Dufour (UI) <nicoduf@yahoo.fr>
 *
 * Overlays filters
 *   Noise fill
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
    \brief    Custom predefined Noise fill filter.
    
    Basic noise fill and transparency texture

    Filter's parameters:
    * Turbulence type (enum, default fractalNoise else turbulence) -> turbulence (type)
    * Horizontal frequency (*1000) (0.01->10000., default 20) -> turbulence (baseFrequency [/1000])
    * Vertical frequency (*1000) (0.01->10000., default 40) -> turbulence (baseFrequency [/1000])
    * Complexity (1->5, default 5) -> turbulence (numOctaves)
    * Variation (1->360, default 1) -> turbulence (seed)
    * Dilatation (1.->50., default 3) -> color (n-1th value)
    * Erosion (0.->50., default 1) -> color (nth value 0->-50)
    * Color (guint, default 148,115,39,255) -> flood (flood-color, flood-opacity)
    * Inverted (boolean, default false)  -> composite1 (operator, true="in", false="out")
*/

class NoiseFill : public Inkscape::Extension::Internal::Filter::Filter {
protected:
    virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
    NoiseFill ( ) : Filter() { };
    virtual ~NoiseFill ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

    static void init (void) {
        Inkscape::Extension::build_from_mem(
            "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
              "<name>" N_("Noise Fill") "</name>\n"
              "<id>org.inkscape.effect.filter.NoiseFill</id>\n"
              "<param name=\"tab\" type=\"notebook\">\n"
                "<page name=\"optionstab\" _gui-text=\"" N_("Options") "\">\n"
                  "<param name=\"type\" _gui-text=\"" N_("Turbulence type:") "\" type=\"enum\">\n"
                    "<_item value=\"fractalNoise\">" N_("Fractal noise") "</_item>\n"
                    "<_item value=\"turbulence\">" N_("Turbulence") "</_item>\n"
                  "</param>\n"
                  "<param name=\"hfreq\" _gui-text=\"" N_("Horizontal frequency:") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.01\" max=\"10000.00\">20</param>\n"
                  "<param name=\"vfreq\" _gui-text=\"" N_("Vertical frequency:") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.01\" max=\"10000.00\">40</param>\n"
                  "<param name=\"complexity\" _gui-text=\"" N_("Complexity:") "\" type=\"int\" appearance=\"full\" min=\"1\" max=\"5\">5</param>\n"
                  "<param name=\"variation\" _gui-text=\"" N_("Variation:") "\" type=\"int\" appearance=\"full\" min=\"1\" max=\"360\">0</param>\n"
                  "<param name=\"dilat\" _gui-text=\"" N_("Dilatation:") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"1\" max=\"50\">3</param>\n"
                  "<param name=\"erosion\" _gui-text=\"" N_("Erosion:") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0\" max=\"50\">1</param>\n"
                  "<param name=\"inverted\" _gui-text=\"" N_("Inverted") "\" type=\"boolean\" >false</param>\n"
                "</page>\n"
                "<page name=\"co11tab\" _gui-text=\"" N_("Noise color") "\">\n"
                  "<param name=\"color\" _gui-text=\"" N_("Color") "\" type=\"color\">354957823</param>\n"
                "</page>\n"
              "</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Overlays") "\"/>\n"
                  "</submenu>\n"
                "</effects-menu>\n"
                "<menu-tip>" N_("Basic noise fill and transparency texture") "</menu-tip>\n"
              "</effect>\n"
            "</inkscape-extension>\n", new NoiseFill());
    };

};

gchar const *
NoiseFill::get_filter_text (Inkscape::Extension::Extension * ext)
{
    if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream type;
    std::ostringstream hfreq;
    std::ostringstream vfreq;
    std::ostringstream complexity;
    std::ostringstream variation;
    std::ostringstream dilat;
    std::ostringstream erosion;
    std::ostringstream r;
    std::ostringstream g;
    std::ostringstream b;
    std::ostringstream a;
    std::ostringstream inverted;

    type << ext->get_param_enum("type");
    hfreq << (ext->get_param_float("hfreq") / 1000);
    vfreq << (ext->get_param_float("vfreq") / 1000);
    complexity << ext->get_param_int("complexity");
    variation << ext->get_param_int("variation");
    dilat << ext->get_param_float("dilat");
    erosion << (- ext->get_param_float("erosion"));
    guint32 color = ext->get_param_color("color");
    r << ((color >> 24) & 0xff);
    g << ((color >> 16) & 0xff);
    b << ((color >>  8) & 0xff);
    a << (color & 0xff) / 255.0F;
    if (ext->get_param_bool("inverted"))
        inverted << "out";
    else
        inverted << "in";

    _filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" style=\"color-interpolation-filters:sRGB;\" inkscape:label=\"Noise Fill\">\n"
          "<feTurbulence type=\"%s\" baseFrequency=\"%s %s\" numOctaves=\"%s\" seed=\"%s\" result=\"turbulence\"/>\n"
          "<feComposite in=\"SourceGraphic\" in2=\"turbulence\" operator=\"%s\" result=\"composite1\" />\n"
          "<feColorMatrix values=\"1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 0 0 0 %s %s \" result=\"color\" />\n"
          "<feFlood flood-opacity=\"%s\" flood-color=\"rgb(%s,%s,%s)\" result=\"flood\" />\n"
          "<feMerge result=\"merge\">\n"
            "<feMergeNode in=\"flood\" />\n"
            "<feMergeNode in=\"color\" />\n"
          "</feMerge>\n"
          "<feComposite in2=\"SourceGraphic\" operator=\"in\" result=\"composite2\" />\n"
        "</filter>\n", type.str().c_str(), hfreq.str().c_str(), vfreq.str().c_str(), complexity.str().c_str(), variation.str().c_str(), inverted.str().c_str(), dilat.str().c_str(), erosion.str().c_str(), a.str().c_str(), r.str().c_str(), g.str().c_str(), b.str().c_str());

    return _filter;
}; /* NoiseFill filter */

}; /* namespace Filter */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */

/* Change the 'OVERLAYS' below to be your file name */
#endif /* SEEN_INKSCAPE_EXTENSION_INTERNAL_FILTER_OVERLAYS_H__ */
