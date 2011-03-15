#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_ABC_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_ABC_H__
/* Change the 'ABC' above to be your file name */

/*
 * Copyright (C) 2011 Authors:
 *   Ivan Louette (filters)
 *   Nicolas Dufour (UI) <nicoduf@yahoo.fr>
 *
 * Basic filters
 *   Blur
 *   Clean edges
 *   Color shift
 *   Diffuse light
 *   Feather
 *   Matte jelly
 *   Noise fill
 *   Outline
 *   Roughen
 *   Silhouette
 *   Specular light
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
              "<name>" N_("Blur, custom (ABCs)") "</name>\n"
              "<id>org.inkscape.effect.filter.Blur</id>\n"
              "<param name=\"hblur\" gui-text=\"" N_("Horizontal blur:") "\" type=\"float\" appearance=\"full\" min=\"0.01\" max=\"100\">2</param>\n"
              "<param name=\"vblur\" gui-text=\"" N_("Vertical blur:") "\" type=\"float\" appearance=\"full\" min=\"0.01\" max=\"100\">2</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Experimental") "\"/>\n"
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

    std::ostringstream hblur;
    std::ostringstream vblur;

    hblur << ext->get_param_float("hblur");
    vblur << ext->get_param_float("vblur");

    _filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" inkscape:label=\"Blur, custom\">\n"
          "<feGaussianBlur stdDeviation=\"%s %s\" result=\"blur\" />\n"
        "</filter>\n", hblur.str().c_str(), vblur.str().c_str());

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
              "<name>" N_("Clean edges, custom (ABCs)") "</name>\n"
              "<id>org.inkscape.effect.filter.CleanEdges</id>\n"
              "<param name=\"blur\" gui-text=\"" N_("Strength:") "\" type=\"float\" appearance=\"full\" min=\"0.01\" max=\"2.0\">0.4</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Experimental") "\"/>\n"
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
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" inkscape:label=\"Clean edges, custom\">\n"
          "<feGaussianBlur stdDeviation=\"%s\" result=\"blur\" />\n"
          "<feComposite in=\"SourceGraphic\" in2=\"blur\" operator=\"in\" result=\"composite1\" />\n"
          "<feComposite in=\"composite1\" in2=\"composite1\" k2=\"1\" operator=\"in\" result=\"composite2\" />\n"
        "</filter>\n", blur.str().c_str());

    return _filter;
}; /* CleanEdges filter */


/**
    \brief    Custom predefined Color shift filter.
    
    Rotate and desaturate hue

    Filter's parameters:
    * Shift (0->360, default 330) -> color1 (values)
    * Saturation (0.->10., default 6) -> color2 (values [/10])
*/

class ColorShift : public Inkscape::Extension::Internal::Filter::Filter {
protected:
    virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
    ColorShift ( ) : Filter() { };
    virtual ~ColorShift ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

    static void init (void) {
        Inkscape::Extension::build_from_mem(
            "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
              "<name>" N_("Color shift, custom (ABCs)") "</name>\n"
              "<id>org.inkscape.effect.filter.ColorShift</id>\n"
              "<param name=\"shift\" gui-text=\"" N_("Shift:") "\" type=\"int\" appearance=\"full\" min=\"0\" max=\"360\">330</param>\n"
              "<param name=\"sat\" gui-text=\"" N_("Saturation:") "\" type=\"float\" appearance=\"full\" min=\"0.\" max=\"10\">6</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Experimental") "\"/>\n"
                  "</submenu>\n"
                "</effects-menu>\n"
                "<menu-tip>" N_("Rotate and desaturate hue") "</menu-tip>\n"
              "</effect>\n"
            "</inkscape-extension>\n", new ColorShift());
    };

};

gchar const *
ColorShift::get_filter_text (Inkscape::Extension::Extension * ext)
{
    if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream shift;
    std::ostringstream sat;

    shift << ext->get_param_int("shift");
    sat << (ext->get_param_float("sat") / 10);

    _filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" height=\"1\" width=\"1\" y=\"0\" x=\"0\" inkscape:label=\"Color shift, custom\">\n"
          "<feColorMatrix type=\"hueRotate\" values=\"%s\" result=\"color1\" />\n"
          "<feColorMatrix type=\"saturate\" values=\"%s\" result=\"color2\" />\n"
        "</filter>\n", shift.str().c_str(), sat.str().c_str());

    return _filter;
}; /* ColorShift filter */

/**
    \brief    Custom predefined Diffuse light filter.
    
    Basic diffuse bevel to use for building textures

    Filter's parameters:
    * Smoothness (0.->10., default 6.) -> blur (stdDeviation)
    * Elevation (0->360, default 25) -> feDistantLight (elevation)
    * Azimuth (0->360, default 235) -> feDistantLight (azimuth)
    * Lightning color (guint, default -1 [white]) -> diffuse (lighting-color)
*/

class DiffuseLight : public Inkscape::Extension::Internal::Filter::Filter {
protected:
    virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
    DiffuseLight ( ) : Filter() { };
    virtual ~DiffuseLight ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

    static void init (void) {
        Inkscape::Extension::build_from_mem(
            "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
              "<name>" N_("Diffuse light, custom (ABCs)") "</name>\n"
              "<id>org.inkscape.effect.filter.DiffuseLight</id>\n"
              "<param name=\"smooth\" gui-text=\"" N_("Smoothness:") "\" type=\"float\" appearance=\"full\" min=\"0.0\" max=\"10\">6</param>\n"
              "<param name=\"elevation\" gui-text=\"" N_("Elevation:") "\" type=\"int\" appearance=\"full\" min=\"0\" max=\"360\">25</param>\n"
              "<param name=\"azimuth\" gui-text=\"" N_("Azimuth:") "\" type=\"int\" appearance=\"full\" min=\"0\" max=\"360\">235</param>\n"
              "<param name=\"color\" gui-text=\"" N_("Lightning color") "\" type=\"color\">-1</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Experimental") "\"/>\n"
                  "</submenu>\n"
                "</effects-menu>\n"
                "<menu-tip>" N_("Basic diffuse bevel to use for building textures") "</menu-tip>\n"
              "</effect>\n"
            "</inkscape-extension>\n", new DiffuseLight());
    };

};

gchar const *
DiffuseLight::get_filter_text (Inkscape::Extension::Extension * ext)
{
    if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream smooth;
    std::ostringstream elevation;
    std::ostringstream azimuth;
    std::ostringstream r;
    std::ostringstream g;
    std::ostringstream b;
    std::ostringstream a;

    smooth << ext->get_param_float("smooth");
    elevation << ext->get_param_int("elevation");
    azimuth << ext->get_param_int("azimuth");
    guint32 color = ext->get_param_color("color");

    r << ((color >> 24) & 0xff);
    g << ((color >> 16) & 0xff);
    b << ((color >>  8) & 0xff);
    a << (color & 0xff) / 255.0F;
    
    _filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" inkscape:label=\"Diffuse light, custom\">\n"
          "<feGaussianBlur in=\"SourceGraphic\" stdDeviation=\"%s\" result=\"blur\" />\n"
          "<feDiffuseLighting diffuseConstant=\"1\" surfaceScale=\"10\" lighting-color=\"rgb(%s,%s,%s)\" result=\"diffuse\">\n"
            "<feDistantLight elevation=\"%s\" azimuth=\"%s\" />\n"
          "</feDiffuseLighting>\n"
          "<feComposite in=\"diffuse\" in2=\"diffuse\" operator=\"arithmetic\" k1=\"1\" result=\"composite1\" />\n"
          "<feComposite in=\"composite1\" in2=\"SourceGraphic\" k1=\"%s\" operator=\"arithmetic\" k3=\"1\" result=\"composite2\" />\n"
        "</filter>\n", smooth.str().c_str(), r.str().c_str(), g.str().c_str(), b.str().c_str(), elevation.str().c_str(), azimuth.str().c_str(), a.str().c_str());

    return _filter;
}; /* DiffuseLight filter */

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
              "<name>" N_("Feather, custom (ABCs)") "</name>\n"
              "<id>org.inkscape.effect.filter.Feather</id>\n"
              "<param name=\"blur\" gui-text=\"" N_("Strength:") "\" type=\"float\" appearance=\"full\" min=\"0.01\" max=\"100\">5</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Experimental") "\"/>\n"
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
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" inkscape:label=\"Black outline, custom\">\n"
          "<feGaussianBlur stdDeviation=\"%s\" result=\"blur\" />\n"
          "<feComposite in=\"SourceGraphic\" in2=\"blur\" operator=\"atop\" result=\"composite1\" />\n"
          "<feComposite in2=\"composite1\" operator=\"in\" result=\"composite2\" />\n"
          "<feComposite in2=\"composite2\" operator=\"in\" result=\"composite3\" />\n"
        "</filter>\n", blur.str().c_str());

    return _filter;
}; /* Feather filter */

/**
    \brief    Custom predefined Matte jelly filter.
    
    Bulging, matte jelly covering

    Filter's parameters:
    * Smoothness (0.0->10., default 7.) -> blur (stdDeviation)
    * Brightness (0.0->5., default .9) -> specular (specularConstant)
    * Elevation (0->360, default 60) -> feDistantLight (elevation)
    * Azimuth (0->360, default 225) -> feDistantLight (azimuth)
    * Lightning color (guint, default -1 [white]) -> specular (lighting-color)
*/

class MatteJelly : public Inkscape::Extension::Internal::Filter::Filter {
protected:
    virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
    MatteJelly ( ) : Filter() { };
    virtual ~MatteJelly ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

    static void init (void) {
        Inkscape::Extension::build_from_mem(
            "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
              "<name>" N_("Matte jelly, custom (ABCs)") "</name>\n"
              "<id>org.inkscape.effect.filter.MatteJelly</id>\n"
              "<param name=\"smooth\" gui-text=\"" N_("Smoothness:") "\" type=\"float\" appearance=\"full\" min=\"0.0\" max=\"10\">7</param>\n"
              "<param name=\"bright\" gui-text=\"" N_("Brightness:") "\" type=\"float\" appearance=\"full\" min=\"0.0\" max=\"5\">0.9</param>\n"
              "<param name=\"elevation\" gui-text=\"" N_("Elevation:") "\" type=\"int\" appearance=\"full\" min=\"0\" max=\"360\">60</param>\n"
              "<param name=\"azimuth\" gui-text=\"" N_("Azimuth:") "\" type=\"int\" appearance=\"full\" min=\"0\" max=\"360\">225</param>\n"
              "<param name=\"color\" gui-text=\"" N_("Lightning color") "\" type=\"color\">-1</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Experimental") "\"/>\n"
                  "</submenu>\n"
                "</effects-menu>\n"
                "<menu-tip>" N_("Bulging, matte jelly covering") "</menu-tip>\n"
              "</effect>\n"
            "</inkscape-extension>\n", new MatteJelly());
    };

};

gchar const *
MatteJelly::get_filter_text (Inkscape::Extension::Extension * ext)
{
    if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream smooth;
    std::ostringstream bright;
    std::ostringstream elevation;
    std::ostringstream azimuth;
    std::ostringstream r;
    std::ostringstream g;
    std::ostringstream b;
    std::ostringstream a;

    smooth << ext->get_param_float("smooth");
    bright << ext->get_param_float("bright");
    elevation << ext->get_param_int("elevation");
    azimuth << ext->get_param_int("azimuth");
    guint32 color = ext->get_param_color("color");

    r << ((color >> 24) & 0xff);
    g << ((color >> 16) & 0xff);
    b << ((color >>  8) & 0xff);
    a << (color & 0xff) / 255.0F;
    
    _filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" inkscape:label=\"Matte jelly, custom\">\n"
          "<feColorMatrix values=\"1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 0 0 0 0.85 0\" result=\"color\" in=\"SourceGraphic\" />\n"
          "<feGaussianBlur in=\"SourceAlpha\" stdDeviation=\"%s\" result=\"blur\" />\n"
          "<feSpecularLighting in=\"blur\" specularExponent=\"25\" specularConstant=\"%s\" surfaceScale=\"5\" lighting-color=\"rgb(%s,%s,%s)\" result=\"specular\">\n"
            "<feDistantLight elevation=\"%s\" azimuth=\"%s\" />\n"
          "</feSpecularLighting>\n"
          "<feComposite in=\"specular\" in2=\"SourceGraphic\" k3=\"1\" k2=\"%s\" operator=\"arithmetic\" result=\"composite1\" />\n"
          "<feComposite in=\"composite1\" in2=\"color\" operator=\"atop\" result=\"composite2\" />\n"
        "</filter>\n", smooth.str().c_str(), bright.str().c_str(), r.str().c_str(), g.str().c_str(), b.str().c_str(), elevation.str().c_str(), azimuth.str().c_str(), a.str().c_str());

    return _filter;
}; /* MatteJelly filter */

/**
    \brief    Custom predefined Noise fill filter.
    
    Basic noise fill and transparency texture

    Filter's parameters:
    * Turbulence type (enum, default fractalNoise else turbulence) -> turbulence (type)
    * Horizontal frequency (*100) (0.001->1000., default 2) -> turbulence (baseFrequency [/100])
    * Vertical frequency (*100) (0.001->1000., default 4) -> turbulence (baseFrequency [/100])
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
              "<name>" N_("Noise fill, custom (ABCs)") "</name>\n"
              "<id>org.inkscape.effect.filter.NoiseFill</id>\n"
              "<param name=\"tab\" type=\"notebook\">\n"
                "<page name=\"optionstab\" _gui-text=\"Options\">\n"
                  "<param name=\"type\" gui-text=\"" N_("Turbulence type:") "\" type=\"enum\">\n"
                    "<_item value=\"fractalNoise\">Fractal noise</_item>\n"
                    "<_item value=\"turbulence\">Turbulence</_item>\n"
                  "</param>\n"
                  "<param name=\"hfreq\" gui-text=\"" N_("Horizontal frequency (x100):") "\" type=\"float\" appearance=\"full\" min=\"0.001\" max=\"1000\">2</param>\n"
                  "<param name=\"vfreq\" gui-text=\"" N_("Vertical frequency (x100):") "\" type=\"float\" appearance=\"full\" min=\"0.001\" max=\"1000\">4</param>\n"
                  "<param name=\"complexity\" gui-text=\"" N_("Complexity:") "\" type=\"int\" appearance=\"full\" min=\"1\" max=\"5\">5</param>\n"
                  "<param name=\"variation\" gui-text=\"" N_("Variation:") "\" type=\"int\" appearance=\"full\" min=\"1\" max=\"360\">0</param>\n"
                  "<param name=\"dilat\" gui-text=\"" N_("Dilatation:") "\" type=\"float\" appearance=\"full\" min=\"1\" max=\"50\">3</param>\n"
                  "<param name=\"erosion\" gui-text=\"" N_("Erosion:") "\" type=\"float\" appearance=\"full\" min=\"0\" max=\"50\">1</param>\n"
                  "<param name=\"inverted\" gui-text=\"" N_("Inverted") "\" type=\"boolean\" >false</param>\n"
                "</page>\n"
                "<page name=\"co11tab\" _gui-text=\"Noise color\">\n"
                  "<param name=\"color\" gui-text=\"" N_("Color") "\" type=\"color\">354957823</param>\n"
                "</page>\n"
              "</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Experimental") "\"/>\n"
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
    hfreq << (ext->get_param_float("hfreq") / 100);
    vfreq << (ext->get_param_float("vfreq") / 100);
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
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" inkscape:label=\"Noise fill, custom\">\n"
          "<feTurbulence type=\"%s\" baseFrequency=\"%s %s\" numOctaves=\"%s\" seed=\"%s\" result=\"turbulence\"/>\n"
          "<feComposite in=\"SourceGraphic\" in2=\"turbulence\" operator=\"%s\" result=\"composite1\" />\n"
          "<feColorMatrix values=\"1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 0 0 0 %s %s \" result=\"color\" />\n"
          "<feFlood in=\"color\" flood-opacity=\"%s\" flood-color=\"rgb(%s,%s,%s)\" result=\"flood\" />\n"
          "<feMerge result=\"merge\">\n"
            "<feMergeNode in=\"flood\" />\n"
            "<feMergeNode in=\"color\" />\n"
          "</feMerge>\n"
          "<feComposite in2=\"SourceGraphic\" operator=\"in\" result=\"composite2\" />\n"
        "</filter>\n", type.str().c_str(), hfreq.str().c_str(), vfreq.str().c_str(), complexity.str().c_str(), variation.str().c_str(), inverted.str().c_str(), dilat.str().c_str(), erosion.str().c_str(), a.str().c_str(), r.str().c_str(), g.str().c_str(), b.str().c_str());

    return _filter;
}; /* NoiseFill filter */

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
              "<name>" N_("Outline, custom (ABCs)") "</name>\n"
              "<id>org.inkscape.effect.filter.Outline</id>\n"
              "<param name=\"tab\" type=\"notebook\">\n"
                "<page name=\"optionstab\" _gui-text=\"Options\">\n"
                  "<param name=\"width\" gui-text=\"" N_("Width:") "\" type=\"float\" appearance=\"full\" min=\"0.01\" max=\"50\">5</param>\n"
                  "<param name=\"melt\" gui-text=\"" N_("Melt:") "\" type=\"float\" appearance=\"full\" min=\"0.01\" max=\"50\">2</param>\n"
                  "<param name=\"dilat\" gui-text=\"" N_("Dilatation:") "\" type=\"float\" appearance=\"full\" min=\"1\" max=\"50\">8</param>\n"
                  "<param name=\"erosion\" gui-text=\"" N_("Erosion:") "\" type=\"float\" appearance=\"full\" min=\"0\" max=\"50\">5</param>\n"
                "</page>\n"
                "<page name=\"co11tab\" _gui-text=\"Color\">\n"
                  "<param name=\"color\" gui-text=\"" N_("Color") "\" type=\"color\">1029214207</param>\n"
                "</page>\n"
              "</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Experimental") "\"/>\n"
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
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" height=\"1.3\" width=\"1.3\" y=\"-0.15\" x=\"-0.15\" inkscape:label=\"Outline, custom\">\n"
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

/**
    \brief    Custom predefined Roughen filter.
    
    Small-scale roughening to edges and content

    Filter's parameters:
    * Turbulence type (enum, default fractalNoise else turbulence) -> turbulence (type)
    * Horizontal frequency (*100) (0.001->1000., default 1.3) -> turbulence (baseFrequency)
    * Vertical frequency (*100) (0.001->1000., default 1.3) -> turbulence (baseFrequency)
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
              "<name>" N_("Roughen, custom (ABCs)") "</name>\n"
              "<id>org.inkscape.effect.filter.Roughen</id>\n"
              "<param name=\"type\" gui-text=\"" N_("Turbulence type:") "\" type=\"enum\">\n"
                "<_item value=\"fractalNoise\">Fractal noise</_item>\n"
                "<_item value=\"turbulence\">Turbulence</_item>\n"
              "</param>\n"
              "<param name=\"hfreq\" gui-text=\"" N_("Horizontal frequency (x100):") "\" type=\"float\" appearance=\"full\" min=\"0.001\" max=\"1000\">1.3</param>\n"
              "<param name=\"vfreq\" gui-text=\"" N_("Vertical frequency (x100):") "\" type=\"float\" appearance=\"full\" min=\"0.001\" max=\"1000\">1.3</param>\n"
              "<param name=\"complexity\" gui-text=\"" N_("Complexity:") "\" type=\"int\" appearance=\"full\" min=\"1\" max=\"5\">5</param>\n"
              "<param name=\"variation\" gui-text=\"" N_("Variation:") "\" type=\"int\" appearance=\"full\" min=\"1\" max=\"360\">0</param>\n"
              "<param name=\"intensity\" gui-text=\"" N_("Intensity:") "\" type=\"float\" appearance=\"full\" min=\"0.0\" max=\"50\">6.6</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Experimental") "\"/>\n"
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
    hfreq << (ext->get_param_float("hfreq") / 100);
    vfreq << (ext->get_param_float("vfreq") / 100);
    complexity << ext->get_param_int("complexity");
    variation << ext->get_param_int("variation");
    intensity << ext->get_param_float("intensity");

    _filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" inkscape:label=\"Roughen, custom\">\n"
          "<feTurbulence  type=\"%s\" numOctaves=\"%s\" seed=\"%s\" baseFrequency=\"%s %s\" result=\"turbulence\" />\n"
          "<feDisplacementMap in=\"SourceGraphic\" in2=\"turbulence\" scale=\"%s\" yChannelSelector=\"G\" xChannelSelector=\"R\" />\n"
        "</filter>\n", type.str().c_str(), complexity.str().c_str(), variation.str().c_str(), hfreq.str().c_str(), vfreq.str().c_str(), intensity.str().c_str());

    return _filter;
}; /* Roughen filter */

/**
    \brief    Custom predefined Silhouette filter.
    
    Repaint anything visible monochrome

    Filter's parameters:
    * Blur (0.01->50., default 0.01) -> blur (stdDeviation)
    * Cutout (boolean, default False) -> composite (false=in, true=out)
    * Color (guint, default 0,0,0,255) -> flood (flood-color, flood-opacity)
*/

class Silhouette : public Inkscape::Extension::Internal::Filter::Filter {
protected:
    virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
    Silhouette ( ) : Filter() { };
    virtual ~Silhouette ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

    static void init (void) {
        Inkscape::Extension::build_from_mem(
            "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
              "<name>" N_("Silhouette, custom (ABCs)") "</name>\n"
              "<id>org.inkscape.effect.filter.Silhouette</id>\n"
              "<param name=\"blur\" gui-text=\"" N_("Blur:") "\" type=\"float\" appearance=\"full\" min=\"0.01\" max=\"50\">0.01</param>\n"
              "<param name=\"cutout\" gui-text=\"" N_("Cutout") "\" type=\"boolean\">false</param>\n"
              "<param name=\"color\" gui-text=\"" N_("Color") "\" type=\"color\">255</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Experimental") "\"/>\n"
                  "</submenu>\n"
                "</effects-menu>\n"
                "<menu-tip>" N_("Repaint anything visible monochrome") "</menu-tip>\n"
              "</effect>\n"
            "</inkscape-extension>\n", new Silhouette());
    };

};

gchar const *
Silhouette::get_filter_text (Inkscape::Extension::Extension * ext)
{
    if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream a;
    std::ostringstream r;
    std::ostringstream g;
    std::ostringstream b;
    std::ostringstream cutout;
    std::ostringstream blur;

    guint32 color = ext->get_param_color("color");
    r << ((color >> 24) & 0xff);
    g << ((color >> 16) & 0xff);
    b << ((color >>  8) & 0xff);
    a << (color & 0xff) / 255.0F;
    if (ext->get_param_bool("cutout"))
        cutout << "out";
    else
        cutout << "in";
    blur << ext->get_param_float("blur");

    _filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" inkscape:label=\"Silhouette, custom\">\n"
          "<feFlood in=\"SourceGraphic\" flood-opacity=\"%s\" flood-color=\"rgb(%s,%s,%s)\" result=\"flood\" />\n"
          "<feComposite in=\"flood\" in2=\"SourceGraphic\" operator=\"%s\" result=\"composite\" />\n"
          "<feGaussianBlur stdDeviation=\"%s\" />\n"
        "</filter>\n", a.str().c_str(), r.str().c_str(), g.str().c_str(), b.str().c_str(), cutout.str().c_str(), blur.str().c_str());

    return _filter;
}; /* Silhouette filter */

/**
    \brief    Custom predefined Specular light filter.
    
    Basic specular bevel to use for building textures

    Filter's parameters:
    * Smoothness (0.0->10., default 6.) -> blur (stdDeviation)
    * Brightness (0.0->5., default 1.) -> specular (specularConstant)
    * Elevation (0->360, default 45) -> feDistantLight (elevation)
    * Azimuth (0->360, default 235) -> feDistantLight (azimuth)
    * Lightning color (guint, default -1 [white]) -> specular (lighting-color)
*/

class SpecularLight : public Inkscape::Extension::Internal::Filter::Filter {
protected:
    virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
    SpecularLight ( ) : Filter() { };
    virtual ~SpecularLight ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

    static void init (void) {
        Inkscape::Extension::build_from_mem(
            "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
              "<name>" N_("Specular light, custom (ABCs)") "</name>\n"
              "<id>org.inkscape.effect.filter.SpecularLight</id>\n"
              "<param name=\"smooth\" gui-text=\"" N_("Smoothness:") "\" type=\"float\" appearance=\"full\" min=\"0.0\" max=\"10\">6</param>\n"
              "<param name=\"bright\" gui-text=\"" N_("Brightness:") "\" type=\"float\" appearance=\"full\" min=\"0.0\" max=\"5\">1</param>\n"
              "<param name=\"elevation\" gui-text=\"" N_("Elevation:") "\" type=\"int\" appearance=\"full\" min=\"0\" max=\"360\">45</param>\n"
              "<param name=\"azimuth\" gui-text=\"" N_("Azimuth:") "\" type=\"int\" appearance=\"full\" min=\"0\" max=\"360\">235</param>\n"
              "<param name=\"color\" gui-text=\"" N_("Lightning color") "\" type=\"color\">-1</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Experimental") "\"/>\n"
                  "</submenu>\n"
                "</effects-menu>\n"
                "<menu-tip>" N_("Basic specular bevel to use for building textures") "</menu-tip>\n"
              "</effect>\n"
            "</inkscape-extension>\n", new SpecularLight());
    };

};

gchar const *
SpecularLight::get_filter_text (Inkscape::Extension::Extension * ext)
{
    if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream smooth;
    std::ostringstream bright;
    std::ostringstream elevation;
    std::ostringstream azimuth;
    std::ostringstream r;
    std::ostringstream g;
    std::ostringstream b;
    std::ostringstream a;

    smooth << ext->get_param_float("smooth");
    bright << ext->get_param_float("bright");
    elevation << ext->get_param_int("elevation");
    azimuth << ext->get_param_int("azimuth");
    guint32 color = ext->get_param_color("color");

    r << ((color >> 24) & 0xff);
    g << ((color >> 16) & 0xff);
    b << ((color >>  8) & 0xff);
    a << (color & 0xff) / 255.0F;
    
    _filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" inkscape:label=\"Specular light, custom\">\n"
          "<feGaussianBlur in=\"SourceAlpha\" stdDeviation=\"%s\" result=\"blur\" />\n"
          "<feSpecularLighting in=\"blur\" specularExponent=\"25\" specularConstant=\"%s\" surfaceScale=\"10\" lighting-color=\"rgb(%s,%s,%s)\" result=\"specular\">\n"
            "<feDistantLight elevation=\"%s\" azimuth=\"%s\" />\n"
          "</feSpecularLighting>\n"
          "<feComposite in=\"specular\" in2=\"SourceGraphic\" k3=\"1\" k2=\"%s\" operator=\"arithmetic\" result=\"composite1\" />\n"
          "<feComposite in=\"composite1\" in2=\"SourceAlpha\" operator=\"in\" result=\"composite2\" />\n"
        "</filter>\n", smooth.str().c_str(), bright.str().c_str(), r.str().c_str(), g.str().c_str(), b.str().c_str(), elevation.str().c_str(), azimuth.str().c_str(), a.str().c_str());

    return _filter;
}; /* SpecularLight filter */


}; /* namespace Filter */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */

/* Change the 'ABC' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_ABC_H__ */
