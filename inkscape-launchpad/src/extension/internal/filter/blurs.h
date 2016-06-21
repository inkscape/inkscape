#ifndef SEEN_INKSCAPE_EXTENSION_INTERNAL_FILTER_BLURS_H__
#define SEEN_INKSCAPE_EXTENSION_INTERNAL_FILTER_BLURS_H__
/* Change the 'BLURS' above to be your file name */

/*
 * Copyright (C) 2011 Authors:
 *   Ivan Louette (filters)
 *   Nicolas Dufour (UI) <nicoduf@yahoo.fr>
 *
 * Blur filters
 *   Blur
 *   Clean edges
 *   Cross blur
 *   Feather
 *   Out of focus
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
    \brief    Custom predefined Blur filter.
    
    Simple horizontal and vertical blur

    Filter's parameters:
    * Horizontal blur (0.01->100., default 2) -> blur (stdDeviation)
    * Vertical blur (0.01->100., default 2) -> blur (stdDeviation)
    * Blur content only (boolean, default false) -> 
*/

class Blur : public Inkscape::Extension::Internal::Filter::Filter {
protected:
    virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
    Blur ( ) : Filter() { };
    virtual ~Blur ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

    static void init (void) {
        Inkscape::Extension::build_from_mem(
            "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
              "<name>" N_("Blur") "</name>\n"
              "<id>org.inkscape.effect.filter.Blur</id>\n"
              "<param name=\"hblur\" _gui-text=\"" N_("Horizontal blur") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.01\" max=\"100.00\">2</param>\n"
              "<param name=\"vblur\" _gui-text=\"" N_("Vertical blur") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.01\" max=\"100.00\">2</param>\n"
              "<param name=\"content\" _gui-text=\"" N_("Blur content only") "\" type=\"boolean\">False</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Blurs") "\"/>\n"
                  "</submenu>\n"
                "</effects-menu>\n"
                "<menu-tip>" N_("Simple vertical and horizontal blur effect") "</menu-tip>\n"
              "</effect>\n"
            "</inkscape-extension>\n", new Blur());
    };

};

gchar const *
Blur::get_filter_text (Inkscape::Extension::Extension * ext)
{
    if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream bbox;
    std::ostringstream hblur;
    std::ostringstream vblur;
    std::ostringstream content;

    hblur << ext->get_param_float("hblur");
    vblur << ext->get_param_float("vblur");

    if (ext->get_param_bool("content")) {
        bbox << "height=\"1\" width=\"1\" y=\"0\" x=\"0\"";
        content << "<feColorMatrix values=\"1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 0 0 0 50 0 \" result=\"colormatrix\" />\n"
                << "<feComposite in=\"colormatrix\" in2=\"SourceGraphic\" operator=\"in\" />\n";
    } else {
        bbox << "" ;
        content << "" ;
    }


    _filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" %s style=\"color-interpolation-filters:sRGB;\" inkscape:label=\"Blur\">\n"
          "<feGaussianBlur stdDeviation=\"%s %s\" result=\"blur\" />\n"
          "%s"
        "</filter>\n", bbox.str().c_str(), hblur.str().c_str(), vblur.str().c_str(), content.str().c_str() );

    return _filter;
}; /* Blur filter */

/**
    \brief    Custom predefined Clean edges filter.
    
    Removes or decreases glows and jaggeries around objects edges after applying some filters

    Filter's parameters:
    * Strength (0.01->2., default 0.4) -> blur (stdDeviation)
*/

class CleanEdges : public Inkscape::Extension::Internal::Filter::Filter {
protected:
    virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
    CleanEdges ( ) : Filter() { };
    virtual ~CleanEdges ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

    static void init (void) {
        Inkscape::Extension::build_from_mem(
            "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
              "<name>" N_("Clean Edges") "</name>\n"
              "<id>org.inkscape.effect.filter.CleanEdges</id>\n"
              "<param name=\"blur\" _gui-text=\"" N_("Strength") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.01\" max=\"2.00\">0.4</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Blurs") "\"/>\n"
                  "</submenu>\n"
                "</effects-menu>\n"
                "<menu-tip>" N_("Removes or decreases glows and jaggeries around objects edges after applying some filters") "</menu-tip>\n"
              "</effect>\n"
            "</inkscape-extension>\n", new CleanEdges());
    };

};

gchar const *
CleanEdges::get_filter_text (Inkscape::Extension::Extension * ext)
{
    if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream blur;

    blur << ext->get_param_float("blur");

    _filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" style=\"color-interpolation-filters:sRGB;\" inkscape:label=\"Clean Edges\">\n"
          "<feGaussianBlur stdDeviation=\"%s\" result=\"blur\" />\n"
          "<feComposite in=\"SourceGraphic\" in2=\"blur\" operator=\"in\" result=\"composite1\" />\n"
          "<feComposite in=\"composite1\" in2=\"composite1\" k2=\"1\" operator=\"in\" result=\"composite2\" />\n"
        "</filter>\n", blur.str().c_str());

    return _filter;
}; /* CleanEdges filter */

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
              "<name>" N_("Cross Blur") "</name>\n"
              "<id>org.inkscape.effect.filter.CrossBlur</id>\n"
              "<param name=\"bright\" _gui-text=\"" N_("Brightness") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.\" max=\"10.00\">0</param>\n"
              "<param name=\"fade\" _gui-text=\"" N_("Fading") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.\" max=\"1.00\">0</param>\n"
              "<param name=\"hblur\" _gui-text=\"" N_("Horizontal blur") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.01\" max=\"20.00\">5</param>\n"
              "<param name=\"vblur\" _gui-text=\"" N_("Vertical blur") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.01\" max=\"20.00\">5</param>\n"
              "<param name=\"blend\" _gui-text=\"" N_("Blend:") "\" type=\"enum\">\n"
                "<_item value=\"darken\">" N_("Darken") "</_item>\n"
                "<_item value=\"screen\">" N_("Screen") "</_item>\n"
                "<_item value=\"multiply\">" N_("Multiply") "</_item>\n"
                "<_item value=\"lighten\">" N_("Lighten") "</_item>\n"
              "</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Blurs") "\"/>\n"
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
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" style=\"color-interpolation-filters:sRGB;\" inkscape:label=\"Cross Blur\">\n"
          "<feColorMatrix in=\"SourceGraphic\" values=\"1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 -0.2125 -0.7154 -0.0721 1 0 \" result=\"colormatrix\" />\n"
          "<feComposite in=\"SourceGraphic\" in2=\"colormatrix\" operator=\"arithmetic\" k2=\"1\" k3=\"%s\" k4=\"%s\" result=\"composite\" />\n"
          "<feGaussianBlur stdDeviation=\"%s 0.01\" result=\"blur1\" />\n"
          "<feGaussianBlur in=\"composite\" stdDeviation=\"0.01 %s\" result=\"blur2\" />\n"
          "<feBlend in=\"blur2\" in2=\"blur1\" mode=\"%s\" result=\"blend\" />\n"
        "</filter>\n", bright.str().c_str(), fade.str().c_str(), hblur.str().c_str(), vblur.str().c_str(), blend.str().c_str());

    return _filter;
}; /* Cross blur filter */

/**
    \brief    Custom predefined Feather filter.
    
    Blurred mask on the edge without altering the contents

    Filter's parameters:
    * Strength (0.01->100., default 5) -> blur (stdDeviation)
*/

class Feather : public Inkscape::Extension::Internal::Filter::Filter {
protected:
    virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
    Feather ( ) : Filter() { };
    virtual ~Feather ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

    static void init (void) {
        Inkscape::Extension::build_from_mem(
            "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
              "<name>" N_("Feather") "</name>\n"
              "<id>org.inkscape.effect.filter.Feather</id>\n"
              "<param name=\"blur\" _gui-text=\"" N_("Strength") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.01\" max=\"100.00\">5</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Blurs") "\"/>\n"
                  "</submenu>\n"
                "</effects-menu>\n"
                "<menu-tip>" N_("Blurred mask on the edge without altering the contents") "</menu-tip>\n"
              "</effect>\n"
            "</inkscape-extension>\n", new Feather());
    };

};

gchar const *
Feather::get_filter_text (Inkscape::Extension::Extension * ext)
{
    if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream blur;

    blur << ext->get_param_float("blur");

    _filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" style=\"color-interpolation-filters:sRGB;\" inkscape:label=\"Feather\">\n"
          "<feGaussianBlur stdDeviation=\"%s\" result=\"blur\" />\n"
          "<feComposite in=\"SourceGraphic\" in2=\"blur\" operator=\"atop\" result=\"composite1\" />\n"
          "<feComposite in2=\"composite1\" operator=\"in\" result=\"composite2\" />\n"
          "<feComposite in2=\"composite2\" operator=\"in\" result=\"composite3\" />\n"
        "</filter>\n", blur.str().c_str());

    return _filter;
}; /* Feather filter */

/**
    \brief    Custom predefined Out of Focus filter.
    
    Blur eroded by white or transparency

    Filter's parameters:
    * Horizontal blur (0.01->10., default 3) -> blur (stdDeviation)
    * Vertical blur (0.01->10., default 3) -> blur (stdDeviation)
    * Dilatation (n-1th value, 0.->100., default 6) -> colormatrix2 (matrix)
    * Erosion (nth value, 0.->100., default 2) -> colormatrix2 (matrix)
    * Opacity (0.->1., default 1.) -> composite1 (k2)
    * Background color (guint, default -1) -> flood (flood-opacity, flood-color)
    * Blend type (enum, default normal) -> blend (mode)
    * Blend to background (boolean, default false) -> blend (false: in2="flood", true: in2="BackgroundImage")

*/

class ImageBlur : public Inkscape::Extension::Internal::Filter::Filter {
protected:
    virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
    ImageBlur ( ) : Filter() { };
    virtual ~ImageBlur ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

    static void init (void) {
        Inkscape::Extension::build_from_mem(
            "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
              "<name>" N_("Out of Focus") "</name>\n"
              "<id>org.inkscape.effect.filter.ImageBlur</id>\n"
              "<param name=\"tab\" type=\"notebook\">\n"
                "<page name=\"optionstab\" _gui-text=\"Options\">\n"
                  "<param name=\"hblur\" _gui-text=\"" N_("Horizontal blur") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.01\" max=\"50.00\">3</param>\n"
                  "<param name=\"vblur\" _gui-text=\"" N_("Vertical blur") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.01\" max=\"50.00\">3</param>\n"
                  "<param name=\"dilat\" _gui-text=\"" N_("Dilatation") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0\" max=\"100\">6</param>\n"
                  "<param name=\"erosion\" _gui-text=\"" N_("Erosion") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0\" max=\"100\">2</param>\n"
                  "<param name=\"opacity\" _gui-text=\"" N_("Opacity") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0\" max=\"1\">1</param>\n"
                "</page>\n"
                "<page name=\"backgroundtab\" _gui-text=\"Background\">\n"
                  "<param name=\"color\" _gui-text=\"" N_("Background color") "\" type=\"color\">-1</param>\n"
                  "<param name=\"blend\" _gui-text=\"" N_("Blend type:") "\" type=\"enum\">\n"
                    "<_item value=\"normal\">" N_("Normal") "</_item>\n"
                    "<_item value=\"darken\">" N_("Darken") "</_item>\n"
                    "<_item value=\"screen\">" N_("Screen") "</_item>\n"
                    "<_item value=\"multiply\">" N_("Multiply") "</_item>\n"
                    "<_item value=\"lighten\">" N_("Lighten") "</_item>\n"
                  "</param>\n"
                  "<param name=\"background\" _gui-text=\"" N_("Blend to background") "\" type=\"boolean\" >false</param>\n"
                "</page>\n"
              "</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Blurs") "\"/>\n"
                  "</submenu>\n"
                "</effects-menu>\n"
                "<menu-tip>" N_("Blur eroded by white or transparency") "</menu-tip>\n"
              "</effect>\n"
            "</inkscape-extension>\n", new ImageBlur());
    };

};

gchar const *
ImageBlur::get_filter_text (Inkscape::Extension::Extension * ext)
{
    if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream hblur;
    std::ostringstream vblur;
    std::ostringstream dilat;
    std::ostringstream erosion;
    std::ostringstream opacity;
    std::ostringstream r;
    std::ostringstream g;
    std::ostringstream b;
    std::ostringstream a;
    std::ostringstream blend;
    std::ostringstream background;

    hblur << ext->get_param_float("hblur");
    vblur << ext->get_param_float("vblur");
    dilat << ext->get_param_float("dilat");
    erosion << -ext->get_param_float("erosion");
    opacity << ext->get_param_float("opacity");

    guint32 color = ext->get_param_color("color");
    r << ((color >> 24) & 0xff);
    g << ((color >> 16) & 0xff);
    b << ((color >>  8) & 0xff);
    a << (color & 0xff) / 255.0F;
    blend << ext->get_param_enum("blend");

    if (ext->get_param_bool("background")) {
        background << "BackgroundImage" ;
    } else {
        background << "flood" ;
    }

    _filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" style=\"color-interpolation-filters:sRGB;\" inkscape:label=\"Out of Focus\">\n"
          "<feFlood flood-opacity=\"%s\" flood-color=\"rgb(%s,%s,%s)\" result=\"flood\" />\n"
          "<feColorMatrix in=\"SourceGraphic\" values=\"1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 -0.2125 -0.7154 -0.0721 1 0 \" result=\"colormatrix1\" />\n"
          "<feGaussianBlur in=\"colormatrix1\" stdDeviation=\"%s %s\" result=\"blur\" />\n"
          "<feColorMatrix in=\"blur\" values=\"1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 0 0 0 %s %s \" result=\"colormatrix2\" />\n"
          "<feBlend in=\"colormatrix2\" in2=\"%s\" mode=\"%s\" result=\"blend\" />\n"
          "<feComposite in=\"blend\" in2=\"blend\" operator=\"arithmetic\" k2=\"%s\" result=\"composite1\" />\n"
          "<feComposite in2=\"SourceGraphic\" operator=\"in\" />\n"
        "</filter>\n", a.str().c_str(), r.str().c_str(), g.str().c_str(), b.str().c_str(),
                       hblur.str().c_str(), vblur.str().c_str(), dilat.str().c_str(), erosion.str().c_str(),
                       background.str().c_str(), blend.str().c_str(), opacity.str().c_str());

    return _filter;
}; /* Out of Focus filter */

}; /* namespace Filter */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */

/* Change the 'BLURS' below to be your file name */
#endif /* SEEN_INKSCAPE_EXTENSION_INTERNAL_FILTER_BLURS_H__ */
