#ifndef SEEN_INKSCAPE_EXTENSION_INTERNAL_FILTER_DISTORT_H__
#define SEEN_INKSCAPE_EXTENSION_INTERNAL_FILTER_DISTORT_H__
/* Change the 'DISTORT' above to be your file name */

/*
 * Copyright (C) 2011 Authors:
 *   Ivan Louette (filters)
 *   Nicolas Dufour (UI) <nicoduf@yahoo.fr>
 *
 * Distort filters
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
    \brief    Custom predefined Roughen filter.
    
    Small-scale roughening to edges and content

    Filter's parameters:
    * Turbulence type (enum, default fractalNoise else turbulence) -> turbulence (type)
    * Horizontal frequency (*1000) (0.01->10000., default 13) -> turbulence (baseFrequency [/1000])
    * Vertical frequency (*1000) (0.01->10000., default 13) -> turbulence (baseFrequency [/1000])
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
              "<param name=\"type\" gui-text=\"" N_("Turbulence type:") "\" type=\"enum\">\n"
                "<_item value=\"fractalNoise\">Fractal noise</_item>\n"
                "<_item value=\"turbulence\">Turbulence</_item>\n"
              "</param>\n"
              "<param name=\"hfreq\" gui-text=\"" N_("Horizontal frequency:") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.01\" max=\"10000.00\">13</param>\n"
              "<param name=\"vfreq\" gui-text=\"" N_("Vertical frequency:") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.01\" max=\"10000.00\">13</param>\n"
              "<param name=\"complexity\" gui-text=\"" N_("Complexity:") "\" type=\"int\" appearance=\"full\" min=\"1\" max=\"5\">5</param>\n"
              "<param name=\"variation\" gui-text=\"" N_("Variation:") "\" type=\"int\" appearance=\"full\" min=\"1\" max=\"360\">0</param>\n"
              "<param name=\"intensity\" gui-text=\"" N_("Intensity:") "\" type=\"float\" appearance=\"full\" min=\"0.0\" max=\"50\">6.6</param>\n"
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
    hfreq << (ext->get_param_float("hfreq") / 1000);
    vfreq << (ext->get_param_float("vfreq") / 1000);
    complexity << ext->get_param_int("complexity");
    variation << ext->get_param_int("variation");
    intensity << ext->get_param_float("intensity");

    _filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" inkscape:label=\"Roughen\">\n"
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
