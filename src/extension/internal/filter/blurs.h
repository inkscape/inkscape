#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_BLURS_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_BLURS_H__
/* Change the 'BLURS' above to be your file name */

/*
 * Copyright (C) 2011 Authors:
 *   Ivan Louette (filters)
 *   Nicolas Dufour (UI) <nicoduf@yahoo.fr>
 *
 * Blur filters
 *   Cross blur
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
    \brief    Custom predefined Cross blur filter.
    
    Combine vertical and horizontal blur

    Filter's parameters:
    * Brighness (0.->10., default 0) -> composite (k3)
    * Fading (0.->1., default 0) -> composite (k4)
    * Horizontal blur (0.01->20., default 5) -> blur (stdDeviation)
    * Vertical blur (0.01->20., default 5) -> blur (stdDeviation)
    * Blend mode (enum, default Darken) -> blend (mode)
*/

class CrossBlur : public Inkscape::Extension::Internal::Filter::Filter {
protected:
    virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
    CrossBlur ( ) : Filter() { };
    virtual ~CrossBlur ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

    static void init (void) {
        Inkscape::Extension::build_from_mem(
            "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
              "<name>" N_("Cross blur, custom (Blurs)") "</name>\n"
              "<id>org.inkscape.effect.filter.CrossBlur</id>\n"
              "<param name=\"bright\" gui-text=\"" N_("Brightness:") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.\" max=\"10.00\">0</param>\n"
              "<param name=\"fade\" gui-text=\"" N_("Fading:") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.\" max=\"1.00\">0</param>\n"
              "<param name=\"hblur\" gui-text=\"" N_("Horizontal blur:") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.01\" max=\"20.00\">5</param>\n"
              "<param name=\"vblur\" gui-text=\"" N_("Vertical blur:") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.01\" max=\"20.00\">5</param>\n"
              "<param name=\"blend\" gui-text=\"" N_("Blend:") "\" type=\"enum\">\n"
                "<_item value=\"darken\">" N_("Darken") "</_item>\n"
                "<_item value=\"screen\">" N_("Screen") "</_item>\n"
                "<_item value=\"multiply\">" N_("Multiply") "</_item>\n"
                "<_item value=\"lighten\">" N_("Lighten") "</_item>\n"
              "</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Experimental") "\"/>\n"
                  "</submenu>\n"
                "</effects-menu>\n"
                "<menu-tip>" N_("Combine vertical and horizontal blur") "</menu-tip>\n"
              "</effect>\n"
            "</inkscape-extension>\n", new CrossBlur());
    };

};

gchar const *
CrossBlur::get_filter_text (Inkscape::Extension::Extension * ext)
{
    if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream bright;
    std::ostringstream fade;
    std::ostringstream hblur;
    std::ostringstream vblur;
    std::ostringstream blend;

    bright << ext->get_param_float("bright");
    fade << ext->get_param_float("fade");
    hblur << ext->get_param_float("hblur");
    vblur << ext->get_param_float("vblur");
    blend << ext->get_param_enum("blend");
    
    _filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" inkscape:label=\"Cross blur, custom\">\n"
          "<feColorMatrix in=\"SourceGraphic\" values=\"1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 -0.2125 -0.7154 -0.0721 1 0 \" result=\"colormatrix\" />\n"
          "<feComposite in=\"SourceGraphic\" in2=\"colormatrix\" operator=\"arithmetic\" k2=\"1\" k3=\"%s\" k4=\"%s\" result=\"composite\" />\n"
          "<feGaussianBlur stdDeviation=\"%s 0.01\" result=\"blur1\" />\n"
          "<feGaussianBlur in=\"composite\" stdDeviation=\"0.01 %s\" result=\"blur2\" />\n"
          "<feBlend in=\"blur2\" in2=\"blur1\" blend=\"normal\" mode=\"%s\" result=\"blend\" />\n"
        "</filter>\n", bright.str().c_str(), fade.str().c_str(), hblur.str().c_str(), vblur.str().c_str(), blend.str().c_str());

    return _filter;
}; /* Cross blur filter */


}; /* namespace Filter */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */

/* Change the 'BLURS' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_BLURS_H__ */
