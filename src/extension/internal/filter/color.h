#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_COLOR_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_COLOR_H__
/* Change the 'COLOR' above to be your file name */

/*
 * Copyright (C) 2011 Authors:
 *   Ivan Louette (filters)
 *   Nicolas Dufour (UI) <nicoduf@yahoo.fr>
 *
 * Color filters
 *   Colorize
 *   Duochrome
 *   Electrize
 *   Quadritone
 *   Solarize
 *   Tritone
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
    \brief    Custom predefined Colorize filter.
    
    Blend image or object with a flood color.

    Filter's parameters:
    * Harsh light (0.->10., default 0) -> composite1 (k1)
    * Normal light (0.->10., default 1) -> composite2 (k2)
    * Duotone (boolean, default false) -> colormatrix1 (values="0")
    * Filtered greys (boolean, default false) -> colormatrix2 (values="0")
    * Blend mode 1 (enum, default Multiply) -> blend1 (mode)
    * Blend mode 2 (enum, default Screen) -> blend2 (mode)
*/

class Colorize : public Inkscape::Extension::Internal::Filter::Filter {
protected:
    virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
    Colorize ( ) : Filter() { };
    virtual ~Colorize ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

    static void init (void) {
        Inkscape::Extension::build_from_mem(
            "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
                "<name>" N_("Colorize, custom (Color)") "</name>\n"
                "<id>org.inkscape.effect.filter.Colorize</id>\n"
                "<param name=\"tab\" type=\"notebook\">\n"
                  "<page name=\"optionstab\" _gui-text=\"Options\">\n"
                    "<param name=\"hlight\" gui-text=\"" N_("Harsh light:") "\" type=\"float\" min=\"0\" max=\"10\">0</param>\n"
                    "<param name=\"nlight\" gui-text=\"" N_("Normal light:") "\" type=\"float\" min=\"0\" max=\"10\">1</param>\n"
                    "<param name=\"duotone\" gui-text=\"" N_("Duotone") "\" type=\"boolean\" >false</param>\n"
                    "<param name=\"blend1\" gui-text=\"" N_("Blend 1:") "\" type=\"enum\">\n"
                      "<_item value=\"multiply\">" N_("Multiply") "</_item>\n"
                      "<_item value=\"normal\">" N_("Normal") "</_item>\n"
                      "<_item value=\"screen\">" N_("Screen") "</_item>\n"
                      "<_item value=\"lighten\">" N_("Lighten") "</_item>\n"
                      "<_item value=\"darken\">" N_("Darken") "</_item>\n"
                    "</param>\n"
                    "<param name=\"blend2\" gui-text=\"" N_("Blend 2:") "\" type=\"enum\">\n"
                      "<_item value=\"screen\">" N_("Screen") "</_item>\n"
                      "<_item value=\"multiply\">" N_("Multiply") "</_item>\n"
                      "<_item value=\"normal\">" N_("Normal") "</_item>\n"
                      "<_item value=\"lighten\">" N_("Lighten") "</_item>\n"
                      "<_item value=\"darken\">" N_("Darken") "</_item>\n"
                    "</param>\n"
                  "</page>\n"
                  "<page name=\"colortab\" _gui-text=\"Color\">\n"
                    "<param name=\"color\" gui-text=\"" N_("Color") "\" type=\"color\">-1639776001</param>\n"
                  "</page>\n"
                "</param>\n"
                "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Experimental") "\"/>\n"
                  "</submenu>\n"
                  "</effects-menu>\n"
                "<menu-tip>" N_("Blend image or object with a flood color") "</menu-tip>\n"
              "</effect>\n"
            "</inkscape-extension>\n", new Colorize());
    };

};

gchar const *
Colorize::get_filter_text (Inkscape::Extension::Extension * ext)
{
    if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream a;
    std::ostringstream r;
    std::ostringstream g;
    std::ostringstream b;
    std::ostringstream hlight;
    std::ostringstream nlight;
    std::ostringstream duotone;
    std::ostringstream blend1;
    std::ostringstream blend2;

    guint32 color = ext->get_param_color("color");
    r << ((color >> 24) & 0xff);
    g << ((color >> 16) & 0xff);
    b << ((color >>  8) & 0xff);
    a << (color & 0xff) / 255.0F;

    hlight << ext->get_param_float("hlight");
    nlight << ext->get_param_float("nlight");
    blend1 << ext->get_param_enum("blend1");
    blend2 << ext->get_param_enum("blend2");
    if (ext->get_param_bool("duotone"))
        duotone << "0";
    else
        duotone << "1";

    _filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" height=\"1\" width=\"1\" y=\"0\" x=\"0\" inkscape:label=\"Colorize, custom\">\n"
          "<feComposite in2=\"SourceGraphic\" operator=\"arithmetic\" k1=\"%s\" k2=\"%s\" result=\"composite1\" />\n"
          "<feColorMatrix in=\"composite1\" values=\"%s\" type=\"saturate\" result=\"colormatrix1\" />\n"
          "<feFlood flood-opacity=\"%s\" flood-color=\"rgb(%s,%s,%s)\" result=\"flood1\" />\n"
          "<feBlend in=\"flood1\" in2=\"colormatrix1\" mode=\"%s\" result=\"blend1\" />\n"
          "<feBlend in2=\"blend1\" mode=\"%s\" result=\"blend2\" />\n"
          "<feColorMatrix in=\"blend2\" values=\"1\" type=\"saturate\" result=\"colormatrix2\" />\n"
          "<feComposite in=\"colormatrix2\" in2=\"SourceGraphic\" operator=\"in\" k2=\"1\" result=\"composite2\" />\n"
        "</filter>\n", hlight.str().c_str(), nlight.str().c_str(), duotone.str().c_str(), a.str().c_str(), r.str().c_str(), g.str().c_str(), b.str().c_str(), blend1.str().c_str(), blend2.str().c_str());

    return _filter;
}; /* Colorize filter */


/**
    \brief    Custom predefined Duochrome filter.
    
    Convert luminance values to a duochrome palette.

    Filter's parameters:
    * Fluorescence level (0.->2., default 0) -> composite4 (k2)
    * Swap (enum, default "No swap") -> composite1, composite2 (operator)
    * Color 1 (guint, default 1364325887) -> flood1 (flood-opacity, flood-color)
    * Color 2 (guint, default -65281) -> flood2 (flood-opacity, flood-color)
*/

class Duochrome : public Inkscape::Extension::Internal::Filter::Filter {
protected:
    virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
    Duochrome ( ) : Filter() { };
    virtual ~Duochrome ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

    static void init (void) {
        Inkscape::Extension::build_from_mem(
            "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
              "<name>" N_("Duochrome, custom (Color)") "</name>\n"
              "<id>org.inkscape.effect.filter.Duochrome</id>\n"
              "<param name=\"tab\" type=\"notebook\">\n"
                "<page name=\"optionstab\" _gui-text=\"Options\">\n"
                  "<param name=\"fluo\" gui-text=\"" N_("Fluorescence level:") "\" type=\"float\" min=\"0\" max=\"2\">0</param>\n"
                  "<param name=\"swap\" gui-text=\"" N_("Swap:") "\" type=\"enum\">\n"
                    "<_item value=\"none\">" N_("No swap") "</_item>\n"
                    "<_item value=\"full\">" N_("Color and alpha") "</_item>\n"
                    "<_item value=\"color\">" N_("Color only") "</_item>\n"
                    "<_item value=\"alpha\">" N_("Alpha only") "</_item>\n"
                  "</param>\n"
                "</page>\n"
                "<page name=\"co11tab\" _gui-text=\"Color 1\">\n"
                  "<param name=\"color1\" gui-text=\"" N_("Color 1") "\" type=\"color\">1364325887</param>\n"
                "</page>\n"
                "<page name=\"co12tab\" _gui-text=\"Color 2\">\n"
                  "<param name=\"color2\" gui-text=\"" N_("Color 2") "\" type=\"color\">-65281</param>\n"
                "</page>\n"
              "</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Experimental") "\"/>\n"
                  "</submenu>\n"
                "</effects-menu>\n"
                "<menu-tip>" N_("Convert luminance values to a duochrome palette") "</menu-tip>\n"
              "</effect>\n"
            "</inkscape-extension>\n", new Duochrome());
    };

};

gchar const *
Duochrome::get_filter_text (Inkscape::Extension::Extension * ext)
{
    if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream a1;
    std::ostringstream r1;
    std::ostringstream g1;
    std::ostringstream b1;
    std::ostringstream a2;
    std::ostringstream r2;
    std::ostringstream g2;
    std::ostringstream b2;
    std::ostringstream fluo;
    std::ostringstream swap1;
    std::ostringstream swap2;
    guint32 color1 = ext->get_param_color("color1");
    guint32 color2 = ext->get_param_color("color2");
    float fluorescence = ext->get_param_float("fluo");
    const gchar *swaptype = ext->get_param_enum("swap");

    r1 << ((color1 >> 24) & 0xff);
    g1 << ((color1 >> 16) & 0xff);
    b1 << ((color1 >>  8) & 0xff);
    r2 << ((color2 >> 24) & 0xff);
    g2 << ((color2 >> 16) & 0xff);
    b2 << ((color2 >>  8) & 0xff);
    fluo << fluorescence;

    if((g_ascii_strcasecmp("full", swaptype) == 0)) {
        swap1 << "in";
        swap2 << "out";
        a1 << (color1 & 0xff) / 255.0F;
        a2 << (color2 & 0xff) / 255.0F;
    } else if((g_ascii_strcasecmp("color", swaptype) == 0)) {
        swap1 << "in";
        swap2 << "out";
        a1 << (color2 & 0xff) / 255.0F;
        a2 << (color1 & 0xff) / 255.0F;
    } else if((g_ascii_strcasecmp("alpha", swaptype) == 0)) {
        swap1 << "out";
        swap2 << "in";
        a1 << (color2 & 0xff) / 255.0F;
        a2 << (color1 & 0xff) / 255.0F;
    } else {
        swap1 << "out";
        swap2 << "in";
        a1 << (color1 & 0xff) / 255.0F;
        a2 << (color2 & 0xff) / 255.0F;
    }

    _filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" height=\"1\" width=\"1\" y=\"0\" x=\"0\" inkscape:label=\"Duochrome, custom\">\n"
          "<feColorMatrix type=\"luminanceToAlpha\" result=\"colormatrix1\" />\n"
          "<feFlood flood-opacity=\"%s\" flood-color=\"rgb(%s,%s,%s)\" result=\"flood1\" />\n"
          "<feComposite in2=\"colormatrix1\" operator=\"%s\" result=\"composite1\" />\n"
          "<feFlood in=\"colormatrix1\" flood-opacity=\"%s\" flood-color=\"rgb(%s,%s,%s)\" result=\"flood2\" />\n"
          "<feComposite in2=\"colormatrix1\" result=\"composite2\" operator=\"%s\" />\n"
          "<feComposite in=\"composite2\" in2=\"composite1\" k2=\"1\"  k3=\"1\" operator=\"arithmetic\" result=\"composite3\" />\n"
          "<feColorMatrix in=\"composite3\" type=\"matrix\" values=\"2 -1 0 0 0 0 2 -1 0 0 -1 0 2 0 0 0 0 0 1 0 \" result=\"colormatrix2\" />\n"
          "<feComposite in=\"colormatrix2\" in2=\"composite3\" operator=\"arithmetic\" k2=\"%s\" result=\"composite4\" />\n"
          "<feBlend in=\"composite4\" in2=\"composite3\" blend=\"normal\" mode=\"normal\" result=\"blend\" />\n"
          "<feComposite in2=\"SourceGraphic\" operator=\"in\" />\n"
        "</filter>\n", a1.str().c_str(), r1.str().c_str(), g1.str().c_str(), b1.str().c_str(), swap1.str().c_str(), a2.str().c_str(), r2.str().c_str(), g2.str().c_str(), b2.str().c_str(), swap2.str().c_str(), fluo.str().c_str());

    return _filter;
}; /* Duochrome filter */

/**
    \brief    Custom predefined Electrize filter.
    
    Electro solarization effects.

    Filter's parameters:
    * Simplify (0.01->10., default 2.) -> blur (stdDeviation)
    * Effect type (enum: table or discrete, default "table") -> component (type)
    * Level (0->10, default 3) -> component (tableValues)
    * Inverted (boolean, default false) -> component (tableValues)
*/
class Electrize : public Inkscape::Extension::Internal::Filter::Filter {
protected:
    virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
    Electrize ( ) : Filter() { };
    virtual ~Electrize ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

    static void init (void) {
        Inkscape::Extension::build_from_mem(
            "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
              "<name>" N_("Electrize, custom (Color)") "</name>\n"
              "<id>org.inkscape.effect.filter.Electrize</id>\n"
              "<param name=\"blur\" gui-text=\"" N_("Simplify:") "\" type=\"float\" min=\"0.01\" max=\"10.0\">2.0</param>\n"
              "<param name=\"type\" gui-text=\"" N_("Effect type:") "\" type=\"enum\">\n"
                "<_item value=\"table\">" N_("Table") "</_item>\n"
                "<_item value=\"discrete\">" N_("Discrete") "</_item>\n"
              "</param>\n"
              "<param name=\"levels\" gui-text=\"" N_("Levels:") "\" type=\"int\" min=\"0\" max=\"10\">3</param>\n"
              "<param name=\"invert\" gui-text=\"" N_("Inverted") "\" type=\"boolean\">false</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Experimental") "\"/>\n"
                  "</submenu>\n"
                "</effects-menu>\n"
                "<menu-tip>" N_("Electro solarization effects") "</menu-tip>\n"
              "</effect>\n"
            "</inkscape-extension>\n", new Electrize());
    };
};

gchar const *
Electrize::get_filter_text (Inkscape::Extension::Extension * ext)
{
    if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream blur;
    std::ostringstream type;
    std::ostringstream values;

    blur << ext->get_param_float("blur");
    type << ext->get_param_enum("type");

    // TransfertComponent table values are calculated based on the effect level and inverted parameters.
    int val = 0;
    int levels = ext->get_param_int("levels") + 1;
    if (ext->get_param_bool("invert"))
        val = 1;
    values << val;
    for ( int step = 1 ; step <= levels ; step++ ) {
        if (val == 1) {
            val = 0;
        }
        else {
            val = 1;
        }
        values << " " << val;
    }
  
    _filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" height=\"1\" width=\"1\" y=\"0\" x=\"0\" inkscape:label=\"Electrize, custom\">\n"
          "<feGaussianBlur stdDeviation=\"%s\" result=\"blur\" />\n"
          "<feComponentTransfer in=\"blur\" stdDeviation=\"2\" result=\"component\" >\n"
            "<feFuncR type=\"%s\" tableValues=\"%s\" />\n"
            "<feFuncG type=\"%s\" tableValues=\"%s\" />\n"
            "<feFuncB type=\"%s\" tableValues=\"%s\" />\n"
          "</feComponentTransfer>\n"
        "</filter>\n", blur.str().c_str(), type.str().c_str(), values.str().c_str(), type.str().c_str(), values.str().c_str(), type.str().c_str(), values.str().c_str());

    return _filter;
}; /* Electrize filter */

/**
    \brief    Custom predefined Quadritone filter.
    
    Replace hue by two colors.

    Filter's parameters:
    * Hue distribution (0->360, default 280) -> colormatrix1 (values)
    * Colors (0->360, default 100) -> colormatrix3 (values)
    * Blend mode 1 (enum, default Normal) -> blend1 (mode)
    * Over-saturation (0.->1., default 0) -> composite1 (k2)
    * Blend mode 2 (enum, default Normal) -> blend2 (mode)
*/

class Quadritone : public Inkscape::Extension::Internal::Filter::Filter {
protected:
    virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
    Quadritone ( ) : Filter() { };
    virtual ~Quadritone ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

    static void init (void) {
        Inkscape::Extension::build_from_mem(
            "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
              "<name>" N_("Quadritone fantasy, custom (Color)") "</name>\n"
              "<id>org.inkscape.effect.filter.Quadritone</id>\n"
                "<param name=\"dist\" gui-text=\"" N_("Hue distribution:") "\" type=\"int\" min=\"0\" max=\"360\">280</param>\n"
                "<param name=\"colors\" gui-text=\"" N_("Colors:") "\" type=\"int\" min=\"0\" max=\"360\">100</param>\n"
                "<param name=\"blend1\" gui-text=\"" N_("Blend 1:") "\" type=\"enum\">\n"
                  "<_item value=\"normal\">" N_("Normal") "</_item>\n"
                  "<_item value=\"multiply\">" N_("Multiply") "</_item>\n"
                  "<_item value=\"screen\">" N_("Screen") "</_item>\n"
                "</param>\n"
                "<param name=\"sat\" gui-text=\"" N_("Over-saturation:") "\" type=\"float\" min=\"0\" max=\"1\">0</param>\n"
                "<param name=\"blend2\" gui-text=\"" N_("Blend 2:") "\" type=\"enum\">\n"
                  "<_item value=\"normal\">" N_("Normal") "</_item>\n"
                  "<_item value=\"screen\">" N_("Screen") "</_item>\n"
                  "<_item value=\"multiply\">" N_("Multiply") "</_item>\n"
                  "<_item value=\"lighten\">" N_("Lighten") "</_item>\n"
                  "<_item value=\"darken\">" N_("Darken") "</_item>\n"
                "</param>\n"
                "<effect>\n"
                  "<object-type>all</object-type>\n"
                  "<effects-menu>\n"
                    "<submenu name=\"" N_("Filters") "\">\n"
                      "<submenu name=\"" N_("Experimental") "\"/>\n"
                    "</submenu>\n"
                  "</effects-menu>\n"
                "<menu-tip>" N_("Replace hue by two colors") "</menu-tip>\n"
              "</effect>\n"
            "</inkscape-extension>\n", new Quadritone());
    };

};

gchar const *
Quadritone::get_filter_text (Inkscape::Extension::Extension * ext)
{
    if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream dist;
    std::ostringstream colors;
    std::ostringstream blend1;
    std::ostringstream sat;
    std::ostringstream blend2;

    dist << ext->get_param_int("dist");
    colors << ext->get_param_int("colors");
    blend1 << ext->get_param_enum("blend1");
    sat << ext->get_param_float("sat");
    blend2 << ext->get_param_enum("blend2");

    _filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" height=\"1\" width=\"1\" y=\"0\" x=\"0\" inkscape:label=\"Quadritone fantasy, custom\">\n"
          "<feColorMatrix in=\"SourceGraphic\" type=\"hueRotate\" values=\"%s\" result=\"colormatrix1\" />\n"
          "<feColorMatrix type=\"matrix\" values=\"0.5 0 0.5 0 0 0 1 0 0 0 0.5 0 0.5 0 0 0 0 0 1 0 \" result=\"colormatrix2\" />\n"
          "<feColorMatrix type=\"hueRotate\" values=\"%s\" result=\"colormatrix3\" />\n"
          "<feBlend in2=\"colormatrix3\" blend=\"normal\" mode=\"%s\" result=\"blend1\" />\n"
          "<feColorMatrix type=\"matrix\" values=\"2.5 -0.75 -0.75 0 0 -0.75 2.5 -0.75 0 0 -0.75 -0.75 2.5 0 0 0 0 0 1 0 \" result=\"colormatrix4\" />\n"
          "<feComposite in=\"colormatrix4\" in2=\"blend1\" operator=\"arithmetic\" k2=\"%s\" result=\"composite1\" />\n"
          "<feBlend in2=\"blend1\" blend=\"normal\" mode=\"%s\" result=\"blend2\" />\n"
        "</filter>\n", dist.str().c_str(), colors.str().c_str(), blend1.str().c_str(), sat.str().c_str(), blend2.str().c_str());

    return _filter;
}; /* Quadritone filter */


/**
    \brief    Custom predefined Solarize filter.
    
    Classic photographic solarization effect.

    Filter's parameters:
    * Type (enum, default "Solarize") ->
        Solarize = blend1 (mode="darken"), blend2 (mode="screen")
        Moonarize = blend1 (mode="lighten"), blend2 (mode="multiply") [No other access to the blend modes]
    * Hue rotation (0->360, default 0) -> colormatrix1 (values)
*/


class Solarize : public Inkscape::Extension::Internal::Filter::Filter {
protected:
    virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
    Solarize ( ) : Filter() { };
    virtual ~Solarize ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

    static void init (void) {
        Inkscape::Extension::build_from_mem(
            "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
              "<name>" N_("Solarize, custom (Color)") "</name>\n"
              "<id>org.inkscape.effect.filter.Solarize</id>\n"
              "<param name=\"rotate\" gui-text=\"" N_("Hue rotation:") "\" type=\"int\" min=\"0\" max=\"360\">0</param>\n"
              "<param name=\"type\" gui-text=\"" N_("Type:") "\" type=\"enum\">\n"
                "<_item value=\"solarize\">" N_("Solarize") "</_item>\n"
                "<_item value=\"moonarize\">" N_("Moonarize") "</_item>\n"
              "</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Experimental") "\"/>\n"
                  "</submenu>\n"
                "</effects-menu>\n"
                "<menu-tip>" N_("Classic photographic solarization effect") "</menu-tip>\n"
              "</effect>\n"
            "</inkscape-extension>\n", new Solarize());
    };

};

gchar const *
Solarize::get_filter_text (Inkscape::Extension::Extension * ext)
{
    if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream rotate;
    std::ostringstream blend1;
    std::ostringstream blend2;

    rotate << ext->get_param_int("rotate");
    const gchar *type = ext->get_param_enum("type");
    if((g_ascii_strcasecmp("solarize", type) == 0)) {
    // Solarize
        blend1 << "darken";
        blend2 << "screen";
    } else {
    // Moonarize
        blend1 << "lighten";
        blend2 << "multiply";
    }

    _filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" height=\"1\" width=\"1\" y=\"0\" x=\"0\" inkscape:label=\"Solarize, custom\">\n"
          "<feColorMatrix values=\"1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 0 0 0 1 1 \" />\n"
          "<feColorMatrix type=\"hueRotate\" values=\"%s\" result=\"colormatrix2\" />\n"
          "<feColorMatrix in=\"colormatrix2\" values=\"-1 0 0 0 1 0 -1 0 0 1 0 0 -1 0 1 0 0 0 1 0 \" result=\"colormatrix3\" />\n"
          "<feBlend in=\"colormatrix3\" in2=\"colormatrix2\" mode=\"%s\" result=\"blend1\" />\n"
          "<feBlend in2=\"blend1\" mode=\"%s\" result=\"blend2\" />\n"
          "<feComposite in2=\"SourceGraphic\" operator=\"in\" />\n"
        "</filter>\n", rotate.str().c_str(), blend1.str().c_str(), blend2.str().c_str());

    return _filter;
}; /* Solarize filter */


/**
    \brief    Custom predefined Tritone filter.
    
    Create a custom tritone palette with additional glow, blend modes and hue moving.

    Filter's parameters:
    * Option (enum, default Normal) ->
        Normal = composite1 (in="qminp", in2="flood"), composite2 (in="p", in2="blend6"), blend6 (in2="qminpc")
        Enhance hue = Normal + composite2 (in="SourceGraphic")
        Radiation = Normal + blend6 (in2="SourceGraphic") composite2 (in="blend6", in2="qminpc")
        Hue to background = Normal + composite1 (in2="BackgroundImage") [a template with an activated background is needed, or colors become black]
    * Hue distribution (0->360, default 0) -> colormatrix1 (values)
    * Colors (guint, default -73203457) -> flood (flood-opacity, flood-color)
    * Global blend (enum, default Lighten) -> blend5 (mode) [Multiply, Screen, Darken, Lighten only!]
    * Glow (0.01->10., default 0.01) -> feGaussianBlur (stdDeviation)
    * Glow & blend (enum, default Normal) -> blend6 (mode) [Normal, Multiply and Darken only!]
    * Local light (0.->10., default 0) -> composite2 (k1)
    * Global light (0.->10., default 1) -> composite2 (k3) [k2 must be fixed to 1].
*/

class Tritone : public Inkscape::Extension::Internal::Filter::Filter {
protected:
    virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
    Tritone ( ) : Filter() { };
    virtual ~Tritone ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

    static void init (void) {
        Inkscape::Extension::build_from_mem(
            "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
              "<name>" N_("Tritone, custom (Color)") "</name>\n"
              "<id>org.inkscape.effect.filter.Tritone</id>\n"
              "<param name=\"tab\" type=\"notebook\">\n"
                "<page name=\"optionstab\" _gui-text=\"Options\">\n"
                  "<param name=\"type\" gui-text=\"" N_("Type:") "\" type=\"enum\">\n"
                    "<_item value=\"normal\">" N_("Normal") "</_item>\n"
                    "<_item value=\"enhue\">" N_("Enhance hue") "</_item>\n"
                    "<_item value=\"rad\">" N_("Radiation") "</_item>\n"
                    "<_item value=\"htb\">" N_("Hue to background") "</_item>\n"
                  "</param>\n"
                  "<param name=\"globalblend\" gui-text=\"" N_("Global blend:") "\" type=\"enum\">\n"
                    "<_item value=\"lighten\">" N_("Lighten") "</_item>\n"
                    "<_item value=\"screen\">" N_("Screen") "</_item>\n"
                    "<_item value=\"multiply\">" N_("Multiply") "</_item>\n"
                    "<_item value=\"darken\">" N_("Darken") "</_item>\n"
                  "</param>\n"
                  "<param name=\"glow\" gui-text=\"" N_("Glow:") "\" type=\"float\" min=\"0.01\" max=\"10\">0.01</param>\n"
                  "<param name=\"glowblend\" gui-text=\"" N_("Glow blend:") "\" type=\"enum\">\n"
                    "<_item value=\"normal\">" N_("Normal") "</_item>\n"
                    "<_item value=\"multiply\">" N_("Multiply") "</_item>\n"
                    "<_item value=\"darken\">" N_("Darken") "</_item>\n"
                  "</param>\n"
                  "<param name=\"llight\" gui-text=\"" N_("Local light:") "\" type=\"float\" min=\"0\" max=\"10\">0</param>\n"
                  "<param name=\"glight\" gui-text=\"" N_("Global light:") "\" type=\"float\" min=\"0\" max=\"10\">1</param>\n"
                "</page>\n"
                "<page name=\"co1tab\" _gui-text=\"Color\">\n"
                  "<param name=\"dist\" gui-text=\"" N_("Hue distribution:") "\" type=\"int\" min=\"0\" max=\"360\">0</param>\n"
                  "<param name=\"color\" gui-text=\"" N_("Color") "\" type=\"color\">-73203457</param>\n"
                "</page>\n"
              "</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Experimental") "\"/>\n"
                  "</submenu>\n"
                "</effects-menu>\n"
                "<menu-tip>" N_("Create a custom tritone palette with additional glow, blend modes and hue moving") "</menu-tip>\n"
              "</effect>\n"
            "</inkscape-extension>\n", new Tritone());
    };

};

gchar const *
Tritone::get_filter_text (Inkscape::Extension::Extension * ext)
{
    if (_filter != NULL) g_free((void *)_filter);
    
    std::ostringstream dist;
    std::ostringstream a;
    std::ostringstream r;
    std::ostringstream g;
    std::ostringstream b;
    std::ostringstream globalblend;
    std::ostringstream glow;
    std::ostringstream glowblend;
    std::ostringstream llight;
    std::ostringstream glight;
    std::ostringstream c1in;
    std::ostringstream c1in2;
    std::ostringstream c2in;
    std::ostringstream c2in2;
    std::ostringstream b6in2;
    
    guint32 color = ext->get_param_color("color");
    r << ((color >> 24) & 0xff);
    g << ((color >> 16) & 0xff);
    b << ((color >>  8) & 0xff);
    a << (color & 0xff) / 255.0F;
    globalblend << ext->get_param_enum("globalblend");
    dist << ext->get_param_int("dist");
    glow << ext->get_param_float("glow");
    glowblend << ext->get_param_enum("glowblend");
    llight << ext->get_param_float("llight");
    glight << ext->get_param_float("glight");
    
    const gchar *type = ext->get_param_enum("type");
    if((g_ascii_strcasecmp("enhue", type) == 0)) {
    // Enhance hue
        c1in << "qminp";
        c1in2 << "flood";
        c2in << "SourceGraphic";
        c2in2 << "blend6";
        b6in2 << "qminpc";
    } else if((g_ascii_strcasecmp("rad", type) == 0)) {
    // Radiation
        c1in << "qminp";
        c1in2 << "flood";
        c2in << "blend6";
        c2in2 << "qminpc";
        b6in2 << "SourceGraphic";
    } else if((g_ascii_strcasecmp("htb", type) == 0)) {
    // Hue to background
        c1in << "qminp";
        c1in2 << "BackgroundImage";
        c2in << "p";
        c2in2 << "blend6";
        b6in2 << "qminpc";
    } else {
    // Normal
        c1in << "qminp";
        c1in2 << "flood";
        c2in << "p";
        c2in2 << "blend6";
        b6in2 << "qminpc";
    }
    
    _filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" height=\"1\" width=\"1\" y=\"0\" x=\"0\" inkscape:label=\"Tritone, custom\">\n"
          "<feColorMatrix type=\"hueRotate\" result=\"colormatrix1\" values=\"%s\" />\n"
          "<feColorMatrix in=\"colormatrix1\" result=\"r\" type=\"matrix\" values=\"1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 0 0 0 0 1 \" />\n"
          "<feColorMatrix in=\"colormatrix1\" result=\"g\" type=\"matrix\" values=\"0 1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 0 0 0 1 \" />\n"
          "<feColorMatrix in=\"colormatrix1\" result=\"b\" type=\"matrix\" values=\"0 0 1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 0 0 1 \" />\n"
          "<feBlend in2=\"g\" mode=\"darken\" in=\"r\" result=\"minrg\" />\n"
          "<feBlend in2=\"b\" mode=\"darken\" in=\"minrg\" result=\"p\" />\n"
          "<feBlend in2=\"g\" mode=\"lighten\" in=\"r\" result=\"maxrg\" />\n"
          "<feBlend in2=\"b\" mode=\"lighten\" in=\"maxrg\" result=\"q\" />\n"
          "<feComponentTransfer in=\"q\" result=\"q2\">\n"
            "<feFuncR type=\"linear\" slope=\"0\" />\n"
          "</feComponentTransfer>\n"
          "<feBlend in2=\"q2\" mode=\"%s\" in=\"p\" result=\"pq\" />\n"
          "<feColorMatrix in=\"pq\" result=\"qminp\" type=\"matrix\" values=\"-1 1 0 0 0 -1 1 0 0 0 -1 1 0 0 0 0 0 0 0 1 \" />\n"
          "<feFlood in=\"qminp\" flood-opacity=\"%s\" flood-color=\"rgb(%s,%s,%s)\" result=\"flood\" />\n"
          "<feComposite in=\"%s\" in2=\"%s\" result=\"qminpc\" operator=\"arithmetic\" k1=\"1\" />\n"
          "<feGaussianBlur stdDeviation=\"%s\" />\n"
          "<feBlend in2=\"%s\" blend=\"normal\" result=\"blend6\" mode=\"%s\" />\n"
          "<feComposite in=\"%s\" in2=\"%s\" operator=\"arithmetic\" k1=\"%s\" k2=\"1\" k3=\"%s\" k4=\"0\" result=\"composite2\" />\n"
          "<feComposite in2=\"SourceGraphic\" in=\"composite2\" operator=\"in\" />\n"
        "</filter>\n", dist.str().c_str(), globalblend.str().c_str(), a.str().c_str(), r.str().c_str(), g.str().c_str(), b.str().c_str(), c1in.str().c_str(), c1in2.str().c_str(), glow.str().c_str(), b6in2.str().c_str(), glowblend.str().c_str(), c2in.str().c_str(), c2in2.str().c_str(), llight.str().c_str(), glight.str().c_str());

    return _filter;
}; /* Tritone filter */

}; /* namespace Filter */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */

/* Change the 'COLOR' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_COLOR_H__ */
