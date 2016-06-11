#ifndef SEEN_INKSCAPE_EXTENSION_INTERNAL_FILTER_BUMPS_H__
#define SEEN_INKSCAPE_EXTENSION_INTERNAL_FILTER_BUMPS_H__
/* Change the 'BUMPS' above to be your file name */

/*
 * Copyright (C) 2011 Authors:
 *   Ivan Louette (filters)
 *   Nicolas Dufour (UI) <nicoduf@yahoo.fr>
 *
 * Bump filters
 *   Bump
 *   Wax bump
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
    \brief    Custom predefined Bump filter.
    
    All purpose bump filter

    Filter's parameters:
    Options
      * Image simplification (0.01->10., default 0.01) -> blur1 (stdDeviation)
      * Bump simplification (0.01->10., default 0.01) -> blur2 (stdDeviation)
      * Crop (-50.->50., default 0.) -> composite1 (k3)
      * Red (-50.->50., default 0.) -> colormatrix1 (values)
      * Green (-50.->50., default 0.) -> colormatrix1 (values)
      * Blue (-50.->50., default 0.) -> colormatrix1 (values)
      * Bump from background (boolean, default false) -> colormatrix1 (false: in="SourceGraphic", true: in="BackgroundImage")
    Lighting
      * Lighting type (enum, default specular) -> lighting block
      * Height (0.->50., default 5.) -> lighting (surfaceScale)
      * Lightness (0.->5., default 1.) -> lighting [diffuselighting (diffuseConstant)|specularlighting (specularConstant)]
      * Precision (1->128, default 15) -> lighting (specularExponent)
      * Color (guint, default -1 (RGB:255,255,255))-> lighting (lighting-color)
    Light source
      * Azimuth (0->360, default 225) -> lightsOptions (distantAzimuth)
      * Elevation (0->180, default 45) -> lightsOptions (distantElevation)
      * X location [point] (-5000->5000, default 526) -> lightsOptions (x)
      * Y location [point] (-5000->5000, default 372) -> lightsOptions (y)
      * Z location [point] (0->5000, default 150) -> lightsOptions (z)
      * X location [spot] (-5000->5000, default 526) -> lightsOptions (x)
      * Y location [spot] (-5000->5000, default 372) -> lightsOptions (y)
      * Z location [spot] (-5000->5000, default 150) -> lightsOptions (z)
      * X target (-5000->5000, default 0) -> lightsOptions (pointsAtX)
      * Y target (-5000->5000, default 0) -> lightsOptions (pointsAtX)
      * Z target (-5000->0, default -1000) -> lightsOptions (pointsAtX)
      * Specular exponent (1->100, default 1) -> lightsOptions (specularExponent)
      * Cone angle (0->100, default 50) -> lightsOptions (limitingConeAngle)
    Color bump
      * Blend type (enum, default normal) -> blend (mode)
      * Image color (guint, default -987158017 (RGB:197,41,41)) -> flood (flood-color)
      * Color bump (boolean, default false) -> composite2 (false: in="diffuselighting", true in="flood")
*/

class Bump : public Inkscape::Extension::Internal::Filter::Filter {
protected:
    virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
    Bump ( ) : Filter() { };
    virtual ~Bump ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

    static void init (void) {
        Inkscape::Extension::build_from_mem(
            "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
              "<name>" N_("Bump") "</name>\n"
              "<id>org.inkscape.effect.filter.Bump</id>\n"
              "<param name=\"tab\" type=\"notebook\">\n"
                "<page name=\"optionstab\" _gui-text=\"Options\">\n"
                  "<param name=\"simplifyImage\" _gui-text=\"" N_("Image simplification") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.01\" max=\"10.00\">0.01</param>\n"
                  "<param name=\"simplifyBump\" _gui-text=\"" N_("Bump simplification") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.01\" max=\"10.00\">0.01</param>\n"
                  "<param name=\"crop\" _gui-text=\"" N_("Crop") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"-50.\" max=\"50.\">0</param>\n"
                  "<_param name=\"sourceHeader\" type=\"description\" appearance=\"header\">" N_("Bump source") "</_param>\n"
                    "<param name=\"red\" _gui-text=\"" N_("Red") "\" type=\"float\" indent=\"1\" appearance=\"full\" precision=\"2\" min=\"-50.\" max=\"50.\">0</param>\n"
                    "<param name=\"green\" _gui-text=\"" N_("Green") "\" type=\"float\" indent=\"1\" appearance=\"full\" precision=\"2\" min=\"-50.\" max=\"50.\">0</param>\n"
                    "<param name=\"blue\" _gui-text=\"" N_("Blue") "\" type=\"float\" indent=\"1\" appearance=\"full\" precision=\"2\" min=\"-50.\" max=\"50.\">0</param>\n"
                    "<param name=\"background\" _gui-text=\"" N_("Bump from background") "\" indent=\"1\" type=\"boolean\">false</param>\n"
                "</page>\n"
                "<page name=\"lightingtab\" _gui-text=\"Lighting\">\n"
                  "<param name=\"lightType\" _gui-text=\"" N_("Lighting type:") "\" type=\"enum\">\n"
                    "<_item value=\"specular\">" N_("Specular") "</_item>\n"
                    "<_item value=\"diffuse\">" N_("Diffuse") "</_item>\n"
                  "</param>\n"
                  "<param name=\"height\" _gui-text=\"" N_("Height") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0\" max=\"50.\">5</param>\n"
                  "<param name=\"lightness\" _gui-text=\"" N_("Lightness") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0\" max=\"5.\">1</param>\n"
                  "<param name=\"precision\" _gui-text=\"" N_("Precision") "\" type=\"int\" appearance=\"full\" min=\"1\" max=\"128\">15</param>\n"
                  "<param name=\"lightingColor\" _gui-text=\"" N_("Color") "\" type=\"color\">-1</param>\n"
                "</page>\n"
                "<page name=\"lightsourcetab\" _gui-text=\"" N_("Light source") "\">\n"
                  "<param name=\"lightSource\" _gui-text=\"" N_("Light source:") "\" type=\"enum\">\n"
                    "<_item value=\"distant\">" N_("Distant") "</_item>\n"
                    "<_item value=\"point\">" N_("Point") "</_item>\n"
                    "<_item value=\"spot\">" N_("Spot") "</_item>\n"
                  "</param>\n"
                  "<_param name=\"distantHeader\" type=\"description\" appearance=\"header\">" N_("Distant light options") "</_param>\n"
                    "<param name=\"distantAzimuth\" _gui-text=\"" N_("Azimuth") "\" type=\"int\" indent=\"1\" appearance=\"full\" min=\"0\" max=\"360\">225</param>\n"
                    "<param name=\"distantElevation\" _gui-text=\"" N_("Elevation") "\" type=\"int\" indent=\"1\" appearance=\"full\" min=\"0\" max=\"180\">45</param>\n"
                  "<_param name=\"pointHeader\" type=\"description\" appearance=\"header\">" N_("Point light options") "</_param>\n"
                    "<param name=\"pointX\" _gui-text=\"" N_("X location") "\" type=\"int\" indent=\"1\" appearance=\"full\" min=\"-5000\" max=\"5000\">526</param>\n"
                    "<param name=\"pointY\" _gui-text=\"" N_("Y location") "\" type=\"int\" indent=\"1\" appearance=\"full\" min=\"-5000\" max=\"5000\">372</param>\n"
                    "<param name=\"pointZ\" _gui-text=\"" N_("Z location") "\" type=\"int\" indent=\"1\" appearance=\"full\" min=\"0\" max=\"5000\">150</param>\n"
                  "<_param name=\"spotHeader\" type=\"description\" appearance=\"header\">" N_("Spot light options") "</_param>\n"
                    "<param name=\"spotX\" _gui-text=\"" N_("X location") "\" type=\"int\" indent=\"1\" appearance=\"full\" min=\"-5000\" max=\"5000\">526</param>\n"
                    "<param name=\"spotY\" _gui-text=\"" N_("Y location") "\" type=\"int\" indent=\"1\" appearance=\"full\" min=\"-5000\" max=\"5000\">372</param>\n"
                    "<param name=\"spotZ\" _gui-text=\"" N_("Z location") "\" type=\"int\" indent=\"1\" appearance=\"full\" min=\"-5000\" max=\"5000\">150</param>\n"
                    "<param name=\"spotAtX\" _gui-text=\"" N_("X target") "\" type=\"int\" indent=\"1\" appearance=\"full\" min=\"-5000\" max=\"5000\">0</param>\n"
                    "<param name=\"spotAtY\" _gui-text=\"" N_("Y target") "\" type=\"int\" indent=\"1\" appearance=\"full\" min=\"-5000\" max=\"5000\">0</param>\n"
                    "<param name=\"spotAtZ\" _gui-text=\"" N_("Z target") "\" type=\"int\" indent=\"1\" appearance=\"full\" min=\"-5000\" max=\"0\">-1000</param>\n"
                    "<param name=\"spotExponent\" _gui-text=\"" N_("Specular exponent") "\" type=\"int\" indent=\"1\" appearance=\"full\" min=\"1\" max=\"100\">1</param>\n"
                    "<param name=\"spotConeAngle\" _gui-text=\"" N_("Cone angle") "\" type=\"int\" indent=\"1\" appearance=\"full\" min=\"0\" max=\"100\">50</param>\n"
                "</page>\n"
                "<page name=\"colortab\" _gui-text=\"Color bump\">\n"
                  "<param name=\"imageColor\" _gui-text=\"" N_("Image color") "\" type=\"color\">-987158017</param>\n"
                  "<param name=\"colorize\" _gui-text=\"" N_("Color bump") "\" type=\"boolean\" >false</param>\n"
                  "<param name=\"blend\" _gui-text=\"" N_("Blend type:") "\" type=\"enum\">\n"
                    "<_item value=\"normal\">" N_("Normal") "</_item>\n"
                    "<_item value=\"darken\">" N_("Darken") "</_item>\n"
                    "<_item value=\"screen\">" N_("Screen") "</_item>\n"
                    "<_item value=\"multiply\">" N_("Multiply") "</_item>\n"
                    "<_item value=\"lighten\">" N_("Lighten") "</_item>\n"
                  "</param>\n"
                "</page>\n"
              "</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Bumps") "\"/>\n"
                  "</submenu>\n"
                "</effects-menu>\n"
                "<menu-tip>" N_("All purposes bump filter") "</menu-tip>\n"
              "</effect>\n"
            "</inkscape-extension>\n", new Bump());
    };

};

gchar const *
Bump::get_filter_text (Inkscape::Extension::Extension * ext)
{
    if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream simplifyImage;
    std::ostringstream simplifyBump;
    std::ostringstream red;
    std::ostringstream green;
    std::ostringstream blue;
    std::ostringstream crop;
    std::ostringstream bumpSource;
    std::ostringstream blend;
    
    std::ostringstream lightStart;
    std::ostringstream lightOptions;
    std::ostringstream lightEnd;
    
    std::ostringstream floodRed;
    std::ostringstream floodGreen;
    std::ostringstream floodBlue;
    std::ostringstream floodAlpha;
    std::ostringstream colorize;

    
    simplifyImage << ext->get_param_float("simplifyImage");
    simplifyBump << ext->get_param_float("simplifyBump");
    red << ext->get_param_float("red");
    green << ext->get_param_float("green");
    blue << ext->get_param_float("blue");
    crop << ext->get_param_float("crop");
    blend << ext->get_param_enum("blend");
    
    guint32 lightingColor = ext->get_param_color("lightingColor");
    guint32 imageColor = ext->get_param_color("imageColor");
    
    if (ext->get_param_bool("background")) {
        bumpSource << "BackgroundImage" ;
    } else {
        bumpSource << "blur1" ;
    }

    const gchar *lightType = ext->get_param_enum("lightType");
    if ((g_ascii_strcasecmp("specular", lightType) == 0)) {
    // Specular
        lightStart << "<feSpecularLighting lighting-color=\"rgb(" << ((lightingColor >> 24) & 0xff) << ","
                   << ((lightingColor >> 16) & 0xff) << "," << ((lightingColor >>  8) & 0xff) << ")\" surfaceScale=\""
                   << ext->get_param_float("height") << "\" specularConstant=\"" << ext->get_param_float("lightness")
                   << "\" specularExponent=\"" << ext->get_param_int("precision") << "\" result=\"lighting\">";
        lightEnd << "</feSpecularLighting>";
    } else {
    // Diffuse
        lightStart << "<feDiffuseLighting lighting-color=\"rgb(" << ((lightingColor >> 24) & 0xff) << ","
                   << ((lightingColor >> 16) & 0xff) << "," << ((lightingColor >>  8) & 0xff) << ")\" surfaceScale=\""
                   << ext->get_param_float("height") << "\" diffuseConstant=\"" << ext->get_param_float("lightness")
                   << "\" result=\"lighting\">";
        lightEnd << "</feDiffuseLighting>";
    }

    const gchar *lightSource = ext->get_param_enum("lightSource");
    if ((g_ascii_strcasecmp("distant", lightSource) == 0)) {
    // Distant
        lightOptions << "<feDistantLight azimuth=\"" << ext->get_param_int("distantAzimuth") << "\" elevation=\""
                     << ext->get_param_int("distantElevation") << "\" />";
    } else if ((g_ascii_strcasecmp("point", lightSource) == 0)) {
    // Point
    lightOptions << "<fePointLight z=\"" << ext->get_param_int("pointX") << "\" y=\"" << ext->get_param_int("pointY")
                 << "\" x=\"" << ext->get_param_int("pointZ") << "\" />";
    } else {
    // Spot
    lightOptions << "<feSpotLight x=\"" << ext->get_param_int("pointX") << "\" y=\"" << ext->get_param_int("pointY")
                 << "\" z=\"" << ext->get_param_int("pointZ") << "\" pointsAtX=\"" << ext->get_param_int("spotAtX")
                 << "\" pointsAtY=\"" << ext->get_param_int("spotAtY") << "\" pointsAtZ=\"" << ext->get_param_int("spotAtZ")
                 << "\" specularExponent=\"" << ext->get_param_int("spotExponent")
                 << "\" limitingConeAngle=\"" << ext->get_param_int("spotConeAngle")
                 << "\" />";
    }

    floodRed << ((imageColor >> 24) & 0xff);
    floodGreen << ((imageColor >> 16) & 0xff);
    floodBlue << ((imageColor >>  8) & 0xff);
    floodAlpha << (imageColor & 0xff) / 255.0F;
    
    if (ext->get_param_bool("colorize")) {
        colorize << "flood" ;
    } else {
        colorize << "blur1" ;
    }
    
    _filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" style=\"color-interpolation-filters:sRGB;\" inkscape:label=\"Bump\">\n"
        "<feGaussianBlur in=\"SourceGraphic\" stdDeviation=\"%s\" result=\"blur1\" />\n"
        "<feColorMatrix in=\"%s\" values=\"1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 %s %s %s 1 0 \" result=\"colormatrix1\" />\n"
        "<feColorMatrix in=\"colormatrix1\" type=\"luminanceToAlpha\" result=\"colormatrix2\" />\n"
        "<feComposite in2=\"blur1\" operator=\"arithmetic\" k2=\"1\" k3=\"%s\" result=\"composite1\" />\n"
        "<feGaussianBlur in=\"composite1\" stdDeviation=\"%s\" result=\"blur2\" />\n"
        "%s\n"
          "%s\n"
        "%s\n"
        "<feFlood flood-color=\"rgb(%s,%s,%s)\" flood-opacity=\"%s\" result=\"flood\" />\n"
        "<feComposite in=\"lighting\" in2=\"%s\" operator=\"arithmetic\" k3=\"1\" k2=\"1\" result=\"composite2\" />\n"
        "<feBlend in2=\"SourceGraphic\" mode=\"%s\" result=\"blend\" />\n"
        "<feComposite in=\"blend\" in2=\"SourceGraphic\" operator=\"in\" k2=\"1\" result=\"composite3\" />\n"
        "</filter>\n", simplifyImage.str().c_str(), bumpSource.str().c_str(), red.str().c_str(), green.str().c_str(), blue.str().c_str(),
                       crop.str().c_str(), simplifyBump.str().c_str(),
                       lightStart.str().c_str(), lightOptions.str().c_str(), lightEnd.str().c_str(),
                       floodRed.str().c_str(), floodGreen.str().c_str(), floodBlue.str().c_str(), floodAlpha.str().c_str(),
                       colorize.str().c_str(), blend.str().c_str());

    return _filter;

}; /* Bump filter */

/**
    \brief    Custom predefined Wax Bump filter.
    
    Turns an image to jelly
    
    Filter's parameters:
    Options
      * Image simplification (0.01->10., default 1.5) -> blur1 (stdDeviation)
      * Bump simplification (0.01->10., default 1) -> blur2 (stdDeviation)
      * Crop (-10.->10., default 1.) -> colormatrix2 (4th value of the last line)
      * Red (-10.->10., default 0.) -> colormatrix2 (values, substract 0.21)
      * Green (-10.->10., default 0.) -> colormatrix2 (values, substract 0.72)
      * Blue (-10.->10., default 0.) -> colormatrix2 (values, substract 0.07)
      * Background (enum, default color) ->
        * color: colormatrix1 (in="flood1")
        * image: colormatrix1 (in="SourceGraphic")
        * blurred image: colormatrix1 (in="blur1")
      * Background opacity (0.->1., default 0) -> colormatrix1 (last value)
    Lighting (specular, distant light)
      * Color (guint, default -1 (RGB:255,255,255))-> lighting (lighting-color)
      * Height (-50.->50., default 5.) -> lighting (surfaceScale)
      * Lightness (0.->10., default 1.4) -> lighting [diffuselighting (diffuseConstant)|specularlighting (specularConstant)]
      * Precision (0->50, default 35) -> lighting (specularExponent)
      * Azimuth (0->360, default 225) -> lightsOptions (distantAzimuth)
      * Elevation (0->180, default 60) -> lightsOptions (distantElevation)
      * Lighting blend (enum, default screen) -> blend1 (mode)
      * Highlight blend (enum, default screen) -> blend2 (mode)
    Bump
      * Trasparency type (enum [in,atop], default atop) -> composite2 (operator)
      * Color (guint, default -520083713 (RGB:225,0,38)) -> flood2 (flood-color)
      * Revert bump (boolean, default false) -> composite1 (false: operator="out", true operator="in")
*/

class WaxBump : public Inkscape::Extension::Internal::Filter::Filter {
protected:
    virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
    WaxBump ( ) : Filter() { };
    virtual ~WaxBump ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

    static void init (void) {
        Inkscape::Extension::build_from_mem(
            "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
              "<name>" N_("Wax Bump") "</name>\n"
              "<id>org.inkscape.effect.filter.WaxBump</id>\n"
              "<param name=\"tab\" type=\"notebook\">\n"
                "<page name=\"optionstab\" _gui-text=\"Options\">\n"
                  "<param name=\"simplifyImage\" _gui-text=\"" N_("Image simplification") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.01\" max=\"10.00\">1.5</param>\n"
                  "<param name=\"simplifyBump\" _gui-text=\"" N_("Bump simplification") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.01\" max=\"10.00\">1</param>\n"
                  "<param name=\"crop\" _gui-text=\"" N_("Crop") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"-10.\" max=\"10.\">1</param>\n"
                  "<_param name=\"sourceHeader\" type=\"description\" appearance=\"header\">" N_("Bump source") "</_param>\n"
                    "<param name=\"red\" _gui-text=\"" N_("Red") "\" type=\"float\" indent=\"1\" appearance=\"full\" precision=\"2\" min=\"-10.\" max=\"10.\">0</param>\n"
                    "<param name=\"green\" _gui-text=\"" N_("Green") "\" type=\"float\" indent=\"1\" appearance=\"full\" precision=\"2\" min=\"-10.\" max=\"10.\">0</param>\n"
                    "<param name=\"blue\" _gui-text=\"" N_("Blue") "\" type=\"float\" indent=\"1\" appearance=\"full\" precision=\"2\" min=\"-10.\" max=\"10.\">0</param>\n"
                    "<param name=\"background\" _gui-text=\"" N_("Background:") "\" type=\"enum\">\n"
                      "<_item value=\"flood1\">" N_("Color") "</_item>\n"
                      "<_item value=\"SourceGraphic\">" N_("Image") "</_item>\n"
                      "<_item value=\"blur1\">" N_("Blurred image") "</_item>\n"
                    "</param>\n"
                    "<param name=\"bgopacity\" _gui-text=\"" N_("Background opacity") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.\" max=\"1.\">0</param>\n"
                "</page>\n"
                "<page name=\"lightingtab\" _gui-text=\"" N_("Lighting") "\">\n"
                  "<param name=\"lightingColor\" _gui-text=\"" N_("Color") "\" type=\"color\">-1</param>\n"
                  "<param name=\"height\" _gui-text=\"" N_("Height") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"-50.\" max=\"50.\">5</param>\n"
                  "<param name=\"lightness\" _gui-text=\"" N_("Lightness") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0\" max=\"10.\">1.4</param>\n"
                  "<param name=\"precision\" _gui-text=\"" N_("Precision") "\" type=\"int\" appearance=\"full\" min=\"1\" max=\"50\">35</param>\n"
                  "<param name=\"distantAzimuth\" _gui-text=\"" N_("Azimuth") "\" type=\"int\" indent=\"1\" appearance=\"full\" min=\"0\" max=\"360\">225</param>\n"
                  "<param name=\"distantElevation\" _gui-text=\"" N_("Elevation") "\" type=\"int\" indent=\"1\" appearance=\"full\" min=\"0\" max=\"180\">60</param>\n"
                  "<param name=\"lightingblend\" _gui-text=\"" N_("Lighting blend:") "\" type=\"enum\">\n"
                    "<_item value=\"screen\">" N_("Screen") "</_item>\n"
                    "<_item value=\"normal\">" N_("Normal") "</_item>\n"
                    "<_item value=\"darken\">" N_("Darken") "</_item>\n"
                    "<_item value=\"multiply\">" N_("Multiply") "</_item>\n"
                    "<_item value=\"lighten\">" N_("Lighten") "</_item>\n"
                  "</param>\n"
                  "<param name=\"highlightblend\" _gui-text=\"" N_("Highlight blend:") "\" type=\"enum\">\n"
                    "<_item value=\"screen\">" N_("Screen") "</_item>\n"
                    "<_item value=\"normal\">" N_("Normal") "</_item>\n"
                    "<_item value=\"darken\">" N_("Darken") "</_item>\n"
                    "<_item value=\"multiply\">" N_("Multiply") "</_item>\n"
                    "<_item value=\"lighten\">" N_("Lighten") "</_item>\n"
                  "</param>\n"
                "</page>\n"
                "<page name=\"colortab\" _gui-text=\"Bump\">\n"
                  "<param name=\"imageColor\" _gui-text=\"" N_("Bump color") "\" type=\"color\">-520083713</param>\n"
                  "<param name=\"revert\" _gui-text=\"" N_("Revert bump") "\" type=\"boolean\" >false</param>\n"
                  "<param name=\"transparency\" _gui-text=\"" N_("Transparency type:") "\" type=\"enum\">\n"
                    "<_item value=\"atop\">" N_("Atop") "</_item>\n"
                    "<_item value=\"in\">" N_("In") "</_item>\n"
                  "</param>\n"
                "</page>\n"
              "</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Bumps") "\"/>\n"
                  "</submenu>\n"
                "</effects-menu>\n"
                "<menu-tip>" N_("Turns an image to jelly") "</menu-tip>\n"
              "</effect>\n"
            "</inkscape-extension>\n", new WaxBump());
    };

};

gchar const *
WaxBump::get_filter_text (Inkscape::Extension::Extension * ext)
{
    if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream simplifyImage;
    std::ostringstream simplifyBump;
    std::ostringstream crop;

    std::ostringstream red;
    std::ostringstream green;
    std::ostringstream blue;

    std::ostringstream background;
    std::ostringstream bgopacity;
    
    std::ostringstream height;
    std::ostringstream lightness;
    std::ostringstream precision;
    std::ostringstream distantAzimuth;
    std::ostringstream distantElevation;
    
    std::ostringstream lightRed;
    std::ostringstream lightGreen;
    std::ostringstream lightBlue;

    std::ostringstream floodRed;
    std::ostringstream floodGreen;
    std::ostringstream floodBlue;
    std::ostringstream floodAlpha;

    std::ostringstream revert;
    std::ostringstream lightingblend;
    std::ostringstream highlightblend;
    std::ostringstream transparency;

    simplifyImage << ext->get_param_float("simplifyImage");
    simplifyBump << ext->get_param_float("simplifyBump");
    crop << ext->get_param_float("crop");

    red << ext->get_param_float("red") - 0.21;
    green << ext->get_param_float("green") - 0.72;
    blue << ext->get_param_float("blue") - 0.07;

    background << ext->get_param_enum("background");
    bgopacity << ext->get_param_float("bgopacity");

    height << ext->get_param_float("height");
    lightness << ext->get_param_float("lightness");
    precision << ext->get_param_int("precision");
    distantAzimuth << ext->get_param_int("distantAzimuth");
    distantElevation << ext->get_param_int("distantElevation");

    guint32 lightingColor = ext->get_param_color("lightingColor");
    lightRed << ((lightingColor >> 24) & 0xff);
    lightGreen << ((lightingColor >> 16) & 0xff);
    lightBlue << ((lightingColor >>  8) & 0xff);

    guint32 imageColor = ext->get_param_color("imageColor");
    floodRed << ((imageColor >> 24) & 0xff);
    floodGreen << ((imageColor >> 16) & 0xff);
    floodBlue << ((imageColor >>  8) & 0xff);
    floodAlpha << (imageColor & 0xff) / 255.0F;
    
    if (ext->get_param_bool("revert")) {
        revert << "in" ;
    } else {
        revert << "out" ;
    }

    lightingblend << ext->get_param_enum("lightingblend");
    highlightblend << ext->get_param_enum("highlightblend");
    transparency << ext->get_param_enum("transparency");

    _filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" style=\"color-interpolation-filters:sRGB;\" inkscape:label=\"Wax Bump\">\n"
          "<feGaussianBlur in=\"SourceGraphic\" stdDeviation=\"%s\" result=\"blur1\" />\n"
          "<feFlood flood-opacity=\"1\" flood-color=\"rgb(255,255,255)\" result=\"flood1\" />\n"
          "<feColorMatrix in=\"%s\" values=\"1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 0 0 0 0 %s \" result=\"colormatrix1\" />\n"
          "<feColorMatrix in=\"blur1\" values=\"0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 %s %s %s %s 0 \" result=\"colormatrix2\" />\n"
          "<feFlood flood-color=\"rgb(%s,%s,%s)\" flood-opacity=\"%s\" result=\"flood2\" />\n"
          "<feComposite in=\"flood2\" in2=\"colormatrix2\" operator=\"%s\" result=\"composite1\" />\n"
          "<feGaussianBlur in=\"composite1\" stdDeviation=\"%s\" result=\"blur2\" />\n"
          "<feSpecularLighting in=\"blur2\" lighting-color=\"rgb(%s,%s,%s)\" specularConstant=\"%s\" surfaceScale=\"%s\" specularExponent=\"%s\" result=\"specular\">\n"
            "<feDistantLight elevation=\"%s\" azimuth=\"%s\" />\n"
          "</feSpecularLighting>\n"
          "<feBlend in=\"specular\" in2=\"blur2\" specularConstant=\"1\" mode=\"%s\" result=\"blend1\" />\n"
          "<feComposite in=\"blend1\" in2=\"blur2\" k2=\"0\" operator=\"%s\" k1=\"0.5\" k3=\"0.5\" k4=\"0\" result=\"composite2\" />\n"
          "<feMerge result=\"merge\">\n"
            "<feMergeNode in=\"colormatrix1\" />\n"
            "<feMergeNode in=\"composite2\" />\n"
          "</feMerge>\n"
          "<feBlend in2=\"composite2\" mode=\"%s\" result=\"blend2\" />\n"
          "<feComposite in=\"blend2\" in2=\"SourceGraphic\" operator=\"in\" result=\"composite3\" />\n"
        "</filter>\n", simplifyImage.str().c_str(), background.str().c_str(), bgopacity.str().c_str(),
                       red.str().c_str(), green.str().c_str(), blue.str().c_str(), crop.str().c_str(),
                       floodRed.str().c_str(), floodGreen.str().c_str(), floodBlue.str().c_str(), floodAlpha.str().c_str(),
                       revert.str().c_str(), simplifyBump.str().c_str(),
                       lightRed.str().c_str(), lightGreen.str().c_str(), lightBlue.str().c_str(),
                       lightness.str().c_str(), height.str().c_str(), precision.str().c_str(),
                       distantElevation.str().c_str(), distantAzimuth.str().c_str(),
                       lightingblend.str().c_str(), transparency.str().c_str(), highlightblend.str().c_str() );

    return _filter;

}; /* Wax bump filter */

}; /* namespace Filter */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */

/* Change the 'BUMPS' below to be your file name */
#endif /* SEEN_INKSCAPE_EXTENSION_INTERNAL_FILTER_BUMPS_H__ */
