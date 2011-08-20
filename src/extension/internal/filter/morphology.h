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
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" style=\"color-interpolation-filters:sRGB;\" inkscape:label=\"Cross-smooth\">\n"
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
    * Stroke type (enum, default single)
        * single -> composite4 (in="composite3"), composite2 (operator="atop")
        * double -> composite4 (in="SourceGraphic"), composite2 (operator="xor")
    * Stroke position (enum, default inside)
        * inside -> composite1 (operator="out", in="SourceGraphic", in2="blur1")
        * outside -> composite1 (operator="out", in="blur1", in2="SourceGraphic")
        * overlayed -> composite1 (operator="xor", in="blur1", in2="SourceGraphic")
    * Width 1(0.01->20., default 4) -> blur1 (stdDeviation)
    * Width 2 (0.01->20., default 0.5) -> blur2 (stdDeviation)
    * Dilatation 1 (1.->100., default 100) -> colormatrix1 (n-1th value)
    * Erosion 1 (0.->100., default 1) -> colormatrix1 (nth value 0->-100)
    * Dilatation 2 (1.->100., default 50) -> colormatrix2 (n-1th value)
    * Erosion 2 (0.->100., default 5) -> colormatrix2 (nth value 0->-100)
    * Color (guint, default 200,55,55,255) -> flood (flood-color, flood-opacity)
    * Fill opacity (0.->1., default 1) -> composite5 (k2)
    * Stroke opacity (0.->1., default 1) -> composite5 (k3)
        
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
                  "<param name=\"type\" gui-text=\"" N_("Type:") "\" type=\"enum\">\n"
                    "<_item value=\"single\">" N_("Single") "</_item>\n"
                    "<_item value=\"double\">" N_("Double") "</_item>\n"
                  "</param>\n"
                  "<param name=\"position\" gui-text=\"" N_("Position:") "\" type=\"enum\">\n"
                    "<_item value=\"inside\">" N_("Inside") "</_item>\n"
                    "<_item value=\"outside\">" N_("Outside") "</_item>\n"
                    "<_item value=\"overlayed\">" N_("Overlayed") "</_item>\n"
                  "</param>\n"
                  "<param name=\"width1\" gui-text=\"" N_("Width 1:") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.01\" max=\"20.00\">4</param>\n"
                  "<param name=\"width2\" gui-text=\"" N_("Width 2:") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.01\" max=\"20.00\">0.5</param>\n"
                  "<param name=\"dilat1\" gui-text=\"" N_("Dilatation 1:") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"1\" max=\"100\">100</param>\n"
                  "<param name=\"erosion1\" gui-text=\"" N_("Erosion 1:") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0\" max=\"100\">1</param>\n"
                  "<param name=\"dilat2\" gui-text=\"" N_("Dilatation 2:") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"1\" max=\"100\">50</param>\n"
                  "<param name=\"erosion2\" gui-text=\"" N_("Erosion 2:") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0\" max=\"100\">5</param>\n"
                "</page>\n"
                "<page name=\"co11tab\" _gui-text=\"Color\">\n"
                  "<param name=\"color\" gui-text=\"" N_("Color") "\" type=\"color\">1029214207</param>\n"
                  "<param name=\"fopacity\" gui-text=\"" N_("Fill opacity:") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0\" max=\"1\">1</param>\n"
                  "<param name=\"sopacity\" gui-text=\"" N_("Stroke opacity:") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0\" max=\"1\">1</param>\n"
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

    std::ostringstream width1;
    std::ostringstream width2;
    std::ostringstream dilat1;
    std::ostringstream erosion1;
    std::ostringstream dilat2;
    std::ostringstream erosion2;
    std::ostringstream r;
    std::ostringstream g;
    std::ostringstream b;
    std::ostringstream a;
    std::ostringstream fopacity;
    std::ostringstream sopacity;
    std::ostringstream c4in;
    std::ostringstream c4op;
    std::ostringstream c1in;
    std::ostringstream c1in2;
    std::ostringstream c1op;

    width1 << ext->get_param_float("width1");
    width2 << ext->get_param_float("width2");
    dilat1 << ext->get_param_float("dilat1");
    erosion1 << (- ext->get_param_float("erosion1"));
    dilat2 << ext->get_param_float("dilat2");
    erosion2 << (- ext->get_param_float("erosion2"));

    guint32 color = ext->get_param_color("color");
    r << ((color >> 24) & 0xff);
    g << ((color >> 16) & 0xff);
    b << ((color >>  8) & 0xff);
    a << (color & 0xff) / 255.0F;

    fopacity << ext->get_param_float("fopacity");
    sopacity << ext->get_param_float("sopacity");

    const gchar *type = ext->get_param_enum("type");
    if((g_ascii_strcasecmp("single", type) == 0)) {
    // Single
        c4in << "composite3";
        c4op << "atop";
    } else {
    // Double
        c4in << "SourceGraphic";
        c4op << "xor";
    }

    const gchar *position = ext->get_param_enum("position");
    if((g_ascii_strcasecmp("inside", position) == 0)) {
    // Indide
        c1in << "SourceGraphic3";
        c1in2 << "blur1";
        c1op << "out";
    } else if((g_ascii_strcasecmp("outside", position) == 0)) {
    // Outside
        c1in << "blur1";
        c1in2 << "SourceGraphic";
        c1op << "out";
    } else {
    // Overlayed
        c1in << "blur1";
        c1in2 << "SourceGraphic";
        c1op << "xor";
    }

    _filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" style=\"color-interpolation-filters:sRGB;\" height=\"1.4\" width=\"1.4\" y=\"-0.2\" x=\"-0.2\" inkscape:label=\"Outline\">\n"
          "<feGaussianBlur in=\"SourceAlpha\" stdDeviation=\"%s\" result=\"blur1\" />\n"
          "<feComposite in=\"%s\" in2=\"%s\" operator=\"%s\" result=\"composite1\" />\n"
          "<feColorMatrix values=\"1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 0 0 0 %s %s \" result=\"colormatrix1\" />\n"
          "<feGaussianBlur stdDeviation=\"%s\" result=\"blur2\" />\n"
          "<feComposite in2=\"blur2\" operator=\"over\" result=\"composite2\" />\n"
          "<feColorMatrix in=\"composite2\" values=\"1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 0 0 0 %s %s \" result=\"colormatrix2\" />\n"
          "<feFlood flood-opacity=\"%s\" flood-color=\"rgb(%s,%s,%s)\" result=\"flood\" />\n"
          "<feComposite in=\"flood\" in2=\"colormatrix2\" k2=\"1\" operator=\"in\" result=\"composite3\" />\n"
          "<feComposite in=\"%s\" in2=\"colormatrix2\" operator=\"%s\" result=\"composite4\" />\n"
          "<feComposite in=\"composite4\" in2=\"composite3\" k2=\"%s\" k3=\"%s\" operator=\"arithmetic\" result=\"composite5\" />\n"
        "</filter>\n", width1.str().c_str(), c1in.str().c_str(), c1in2.str().c_str(), c1op.str().c_str(),
                       dilat1.str().c_str(), erosion1.str().c_str(),
                       width2.str().c_str(), dilat2.str().c_str(), erosion2.str().c_str(),
                       a.str().c_str(), r.str().c_str(), g.str().c_str(), b.str().c_str(),
                       c4in.str().c_str(), c4op.str().c_str(),
                       fopacity.str().c_str(), sopacity.str().c_str() );

    return _filter;
}; /* Outline filter */

}; /* namespace Filter */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */

/* Change the 'MORPHOLOGY' below to be your file name */
#endif /* SEEN_INKSCAPE_EXTENSION_INTERNAL_FILTER_MORPHOLOGY_H__ */
