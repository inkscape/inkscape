#ifndef SEEN_INKSCAPE_EXTENSION_INTERNAL_FILTER_BEVELS_H__
#define SEEN_INKSCAPE_EXTENSION_INTERNAL_FILTER_BEVELS_H__
/* Change the 'BEVELS' above to be your file name */

/*
 * Copyright (C) 2011 Authors:
 *   Ivan Louette (filters)
 *   Nicolas Dufour (UI) <nicoduf@yahoo.fr>
 *
 * Bevel filters
 *   Diffuse light
 *   Matte jelly
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
    \brief    Custom predefined Diffuse light filter.
    
    Basic diffuse bevel to use for building textures

    Filter's parameters:
    * Smoothness (0.->10., default 6.) -> blur (stdDeviation)
    * Elevation (0->360, default 25) -> feDistantLight (elevation)
    * Azimuth (0->360, default 235) -> feDistantLight (azimuth)
    * Lighting color (guint, default -1 [white]) -> diffuse (lighting-color)
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
              "<name>" N_("Diffuse Light") "</name>\n"
              "<id>org.inkscape.effect.filter.DiffuseLight</id>\n"
              "<param name=\"smooth\" _gui-text=\"" N_("Smoothness") "\" type=\"float\" appearance=\"full\" min=\"0.0\" max=\"10\">6</param>\n"
              "<param name=\"elevation\" _gui-text=\"" N_("Elevation (°)") "\" type=\"int\" appearance=\"full\" min=\"0\" max=\"360\">25</param>\n"
              "<param name=\"azimuth\" _gui-text=\"" N_("Azimuth (°)") "\" type=\"int\" appearance=\"full\" min=\"0\" max=\"360\">235</param>\n"
              "<param name=\"color\" _gui-text=\"" N_("Lighting color") "\" type=\"color\">-1</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Bevels") "\"/>\n"
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
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" style=\"color-interpolation-filters:sRGB;\" inkscape:label=\"Diffuse Light\">\n"
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
    \brief    Custom predefined Matte jelly filter.
    
    Bulging, matte jelly covering

    Filter's parameters:
    * Smoothness (0.0->10., default 7.) -> blur (stdDeviation)
    * Brightness (0.0->5., default .9) -> specular (specularConstant)
    * Elevation (0->360, default 60) -> feDistantLight (elevation)
    * Azimuth (0->360, default 225) -> feDistantLight (azimuth)
    * Lighting color (guint, default -1 [white]) -> specular (lighting-color)
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
              "<name>" N_("Matte Jelly") "</name>\n"
              "<id>org.inkscape.effect.filter.MatteJelly</id>\n"
              "<param name=\"smooth\" _gui-text=\"" N_("Smoothness") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.00\" max=\"10.00\">7</param>\n"
              "<param name=\"bright\" _gui-text=\"" N_("Brightness") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.00\" max=\"5.00\">0.9</param>\n"
              "<param name=\"elevation\" _gui-text=\"" N_("Elevation (°)") "\" type=\"int\" appearance=\"full\" min=\"0\" max=\"360\">60</param>\n"
              "<param name=\"azimuth\" _gui-text=\"" N_("Azimuth (°)") "\" type=\"int\" appearance=\"full\" min=\"0\" max=\"360\">225</param>\n"
              "<param name=\"color\" _gui-text=\"" N_("Lighting color") "\" type=\"color\">-1</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Bevels") "\"/>\n"
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
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" style=\"color-interpolation-filters:sRGB;\" inkscape:label=\"Matte Jelly\">\n"
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
    \brief    Custom predefined Specular light filter.
    
    Basic specular bevel to use for building textures

    Filter's parameters:
    * Smoothness (0.0->10., default 6.) -> blur (stdDeviation)
    * Brightness (0.0->5., default 1.) -> specular (specularConstant)
    * Elevation (0->360, default 45) -> feDistantLight (elevation)
    * Azimuth (0->360, default 235) -> feDistantLight (azimuth)
    * Lighting color (guint, default -1 [white]) -> specular (lighting-color)
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
              "<name>" N_("Specular Light") "</name>\n"
              "<id>org.inkscape.effect.filter.SpecularLight</id>\n"
              "<param name=\"smooth\" _gui-text=\"" N_("Smoothness") "\" type=\"float\" appearance=\"full\" min=\"0.0\" max=\"10\">6</param>\n"
              "<param name=\"bright\" _gui-text=\"" N_("Brightness") "\" type=\"float\" appearance=\"full\" min=\"0.0\" max=\"5\">1</param>\n"
              "<param name=\"elevation\" _gui-text=\"" N_("Elevation (°)") "\" type=\"int\" appearance=\"full\" min=\"0\" max=\"360\">45</param>\n"
              "<param name=\"azimuth\" _gui-text=\"" N_("Azimuth (°)") "\" type=\"int\" appearance=\"full\" min=\"0\" max=\"360\">235</param>\n"
              "<param name=\"color\" _gui-text=\"" N_("Lighting color") "\" type=\"color\">-1</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Bevels") "\"/>\n"
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
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" style=\"color-interpolation-filters:sRGB;\" inkscape:label=\"Specular Light\">\n"
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

/* Change the 'BEVELS' below to be your file name */
#endif /* SEEN_INKSCAPE_EXTENSION_INTERNAL_FILTER_BEVELS_H__ */
