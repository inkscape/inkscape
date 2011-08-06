#ifndef SEEN_INKSCAPE_EXTENSION_INTERNAL_FILTER_MORPHOLOGY_H__
#define SEEN_INKSCAPE_EXTENSION_INTERNAL_FILTER_MORPHOLOGY_H__
/* Change the 'MORPHOLOGY' above to be your file name */

/*
 * Copyright (C) 2011 Authors:
 *   Ivan Louette (filters)
 *   Nicolas Dufour (UI) <nicoduf@yahoo.fr>
 *
 * Morphology filters
 *   Cross-smooth
 *   Outline
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
    \brief    Custom predefined Cross-smooth filter.
    
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
              "<name>" N_("Cross-smooth") "</name>\n"
              "<id>org.inkscape.effect.filter.crosssmooth</id>\n"
              "<param name=\"type\" gui-text=\"" N_("Type:") "\" type=\"enum\">\n"
                "<_item value=\"edges\">Smooth edges</_item>\n"
                "<_item value=\"all\">Smooth all</_item>\n"
              "</param>\n"
              "<param name=\"blur\" gui-text=\"" N_("Blur:") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.01\" max=\"10.00\">5</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Morphology") "\"/>\n"
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
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" inkscape:label=\"Cross-smooth\">\n"
          "<feGaussianBlur stdDeviation=\"%s\" result=\"blur\" />\n"
          "<feComposite in=\"%s\" in2=\"blur\" operator=\"atop\" result=\"composite1\" />\n"
          "<feComposite in2=\"composite1\" operator=\"in\" result=\"composite2\" />\n"
          "<feComposite in2=\"composite2\" operator=\"in\" result=\"composite3\" />\n"
          "<feColorMatrix values=\"1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 0 0 0 20 -10 \" result=\"colormatrix\" />\n"
        "</filter>\n", blur.str().c_str(), c1in.str().c_str());

    return _filter;
}; /* Crosssmooth filter */

/**
    \brief    Custom predefined Outline filter.
    
    Adds a colorizable outline

    Filter's parameters:
    * Width (0.01->50., default 5) -> blur1 (stdDeviation)
    * Melt (0.01->50., default 2) -> blur2 (stdDeviation)
    * Dilatation (1.->50., default 8) -> color2 (n-1th value)
    * Erosion (0.->50., default 5) -> color2 (nth value 0->-50)
    * Color (guint, default 156,102,102,255) -> flood (flood-color, flood-opacity)
    * Blend (enum, default Normal) -> blend (mode)
*/

class Outline : public Inkscape::Extension::Internal::Filter::Filter {
protected:
    virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
    Outline ( ) : Filter() { };
    virtual ~Outline ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

    static void init (void) {
        Inkscape::Extension::build_from_mem(
            "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
              "<name>" N_("Outline") "</name>\n"
              "<id>org.inkscape.effect.filter.Outline</id>\n"
              "<param name=\"tab\" type=\"notebook\">\n"
                "<page name=\"optionstab\" _gui-text=\"Options\">\n"
                  "<param name=\"width\" gui-text=\"" N_("Width:") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.01\" max=\"50.00\">5</param>\n"
                  "<param name=\"melt\" gui-text=\"" N_("Melt:") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.01\" max=\"50.00\">2</param>\n"
                  "<param name=\"dilat\" gui-text=\"" N_("Dilatation:") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"1\" max=\"50\">8</param>\n"
                  "<param name=\"erosion\" gui-text=\"" N_("Erosion:") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0\" max=\"50\">5</param>\n"
                "</page>\n"
                "<page name=\"co11tab\" _gui-text=\"Color\">\n"
                  "<param name=\"color\" gui-text=\"" N_("Color") "\" type=\"color\">1029214207</param>\n"
                "</page>\n"
              "</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Morphology") "\"/>\n"
                  "</submenu>\n"
                "</effects-menu>\n"
                "<menu-tip>" N_("Adds a colorizable outline") "</menu-tip>\n"
              "</effect>\n"
            "</inkscape-extension>\n", new Outline());
    };

};

gchar const *
Outline::get_filter_text (Inkscape::Extension::Extension * ext)
{
    if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream width;
    std::ostringstream melt;
    std::ostringstream dilat;
    std::ostringstream erosion;
    std::ostringstream r;
    std::ostringstream g;
    std::ostringstream b;
    std::ostringstream a;
    std::ostringstream blend;

    width << ext->get_param_float("width");
    melt << ext->get_param_float("melt");
    dilat << ext->get_param_float("dilat");
    erosion << (- ext->get_param_float("erosion"));
    guint32 color = ext->get_param_color("color");
    r << ((color >> 24) & 0xff);
    g << ((color >> 16) & 0xff);
    b << ((color >>  8) & 0xff);
    a << (color & 0xff) / 255.0F;

    _filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" height=\"1.3\" width=\"1.3\" y=\"-0.15\" x=\"-0.15\" inkscape:label=\"Outline\">\n"
          "<feGaussianBlur in=\"SourceAlpha\" stdDeviation=\"%s\" result=\"blur1\" />\n"
          "<feColorMatrix result=\"color1\" values=\"0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 200 -1 \" />\n"
          "<feGaussianBlur in=\"color1\" stdDeviation=\"%s\" result=\"blur2\" />\n"
          "<feColorMatrix values=\"1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 0 0 0 %s %s \" result=\"color2\" />\n"
          "<feFlood in=\"color2\" result=\"flood\" flood-opacity=\"%s\" flood-color=\"rgb(%s,%s,%s)\" />\n"
          "<feComposite in=\"flood\" in2=\"color2\" operator=\"in\" result=\"composite\" />\n"
          "<feBlend in=\"SourceGraphic\" in2=\"composite\" mode=\"normal\" blend=\"normal\" />\n"
        "</filter>\n", width.str().c_str(), melt.str().c_str(), dilat.str().c_str(), erosion.str().c_str(), a.str().c_str(), r.str().c_str(), g.str().c_str(), b.str().c_str());

    return _filter;
}; /* Outline filter */

}; /* namespace Filter */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */

/* Change the 'MORPHOLOGY' below to be your file name */
#endif /* SEEN_INKSCAPE_EXTENSION_INTERNAL_FILTER_MORPHOLOGY_H__ */
