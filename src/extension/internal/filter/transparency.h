#ifndef SEEN_INKSCAPE_EXTENSION_INTERNAL_FILTER_TRANSPARENCY_H__
#define SEEN_INKSCAPE_EXTENSION_INTERNAL_FILTER_TRANSPARENCY_H__
/* Change the 'TRANSPARENCY' above to be your file name */

/*
 * Copyright (C) 2011 Authors:
 *   Ivan Louette (filters)
 *   Nicolas Dufour (UI) <nicoduf@yahoo.fr>
 *
 * Fill and transparency filters
 *   Blend
 *   Channel transparency
 *   Light eraser
 *   Opacity
 *   Silhouette
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
    \brief    Custom predefined Blend filter.
    
    Blend objects with background images or with themselves

    Filter's parameters:
    * Source (enum [SourceGraphic,BackgroundImage], default BackgroundImage) -> blend (in2)
    * Mode (enum, all blend modes, default Multiply) -> blend (mode)
*/

class Blend : public Inkscape::Extension::Internal::Filter::Filter {
protected:
    virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
    Blend ( ) : Filter() { };
    virtual ~Blend ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

    static void init (void) {
        Inkscape::Extension::build_from_mem(
            "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
              "<name>" N_("Blend") "</name>\n"
              "<id>org.inkscape.effect.filter.Blend</id>\n"
              "<param name=\"source\" _gui-text=\"" N_("Source:") "\" type=\"enum\">\n"
                "<_item value=\"BackgroundImage\">" N_("Background") "</_item>\n"
                "<_item value=\"SourceGraphic\">" N_("Image") "</_item>\n"
              "</param>\n"
              "<param name=\"mode\" _gui-text=\"" N_("Mode:") "\" type=\"enum\">\n"
                "<_item value=\"multiply\">" N_("Multiply") "</_item>\n"
                "<_item value=\"normal\">" N_("Normal") "</_item>\n"
                "<_item value=\"screen\">" N_("Screen") "</_item>\n"
                "<_item value=\"darken\">" N_("Darken") "</_item>\n"
                "<_item value=\"lighten\">" N_("Lighten") "</_item>\n"
              "</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Fill and Transparency") "\"/>\n"
                  "</submenu>\n"
                "</effects-menu>\n"
                "<menu-tip>" N_("Blend objects with background images or with themselves") "</menu-tip>\n"
              "</effect>\n"
            "</inkscape-extension>\n", new Blend());
    };

};

gchar const *
Blend::get_filter_text (Inkscape::Extension::Extension * ext)
{
    if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream source;
    std::ostringstream mode;

    source << ext->get_param_enum("source");
    mode << ext->get_param_enum("mode");

    _filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" style=\"color-interpolation-filters:sRGB;\" inkscape:label=\"Blend\">\n"
          "<feBlend in2=\"%s\" mode=\"%s\" result=\"blend\" />\n"
        "</filter>\n", source.str().c_str(), mode.str().c_str() );

    return _filter;
}; /* Blend filter */

/**
    \brief    Custom predefined Channel transparency filter.
    
    Channel transparency filter.

    Filter's parameters:
    * Saturation (0.->1., default 1.) -> colormatrix1 (values)
    * Red (-10.->10., default -1.) -> colormatrix2 (values)
    * Green (-10.->10., default 0.5) -> colormatrix2 (values)
    * Blue (-10.->10., default 0.5) -> colormatrix2 (values)
    * Alpha (-10.->10., default 1.) -> colormatrix2 (values)
    * Flood colors (guint, default 16777215) -> flood (flood-opacity, flood-color)
    * Inverted (boolean, default false) -> composite1 (operator, true='in', false='out')
    
    Matrix:
      1  0  0  0  0
      0  1  0  0  0
      0  0  1  0  0
      R  G  B  A  0
*/
class ChannelTransparency : public Inkscape::Extension::Internal::Filter::Filter {
protected:
    virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
    ChannelTransparency ( ) : Filter() { };
    virtual ~ChannelTransparency ( ) { if (_filter != NULL) g_free((void *)_filter); return; }
    
    static void init (void) {
        Inkscape::Extension::build_from_mem(
            "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
              "<name>" N_("Channel Transparency") "</name>\n"
              "<id>org.inkscape.effect.filter.ChannelTransparency</id>\n"
              "<param name=\"red\" _gui-text=\"" N_("Red") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"-10.\" max=\"10.\">-1</param>\n"
              "<param name=\"green\" _gui-text=\"" N_("Green") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"-10.\" max=\"10.\">0.5</param>\n"
              "<param name=\"blue\" _gui-text=\"" N_("Blue") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"-10.\" max=\"10.\">0.5</param>\n"
              "<param name=\"alpha\" _gui-text=\"" N_("Alpha") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"-10.\" max=\"10.\">1</param>\n"
              "<param name=\"invert\" _gui-text=\"" N_("Inverted") "\" type=\"boolean\">false</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Fill and Transparency") "\"/>\n"
                  "</submenu>\n"
                "</effects-menu>\n"
                "<menu-tip>" N_("Replace RGB with transparency") "</menu-tip>\n"
              "</effect>\n"
            "</inkscape-extension>\n", new ChannelTransparency());
    };
};

gchar const *
ChannelTransparency::get_filter_text (Inkscape::Extension::Extension * ext)
{
    if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream red;
    std::ostringstream green;
    std::ostringstream blue;
    std::ostringstream alpha;
    std::ostringstream invert;

    red << ext->get_param_float("red");
    green << ext->get_param_float("green");
    blue << ext->get_param_float("blue");
    alpha << ext->get_param_float("alpha");

    if (!ext->get_param_bool("invert")) {
        invert << "in";
    } else {
        invert << "xor";
    }
    
    _filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" inkscape:label=\"Channel Transparency\" style=\"color-interpolation-filters:sRGB;\" >\n"
          "<feColorMatrix values=\"1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 %s %s %s %s 0 \" in=\"SourceGraphic\" result=\"colormatrix\" />\n"
          "<feComposite in=\"colormatrix\" in2=\"SourceGraphic\" operator=\"%s\" result=\"composite1\" />\n"
        "</filter>\n", red.str().c_str(), green.str().c_str(), blue.str().c_str(), alpha.str().c_str(),
                       invert.str().c_str());

    return _filter;
}; /* Channel transparency filter */

/**
    \brief    Custom predefined LightEraser filter.
    
    Make the lightest parts of the object progressively transparent.

    Filter's parameters:
    * Expansion (0.->1000., default 50) -> colormatrix (4th value, multiplicator)
    * Erosion (1.->1000., default 100) -> colormatrix (first 3 values, multiplicator)
    * Global opacity (0.->1., default 1.) -> composite (k2)
    * Inverted (boolean, default false) -> colormatrix (values, true: first 3 values positive, 4th negative)
    
*/
class LightEraser : public Inkscape::Extension::Internal::Filter::Filter {
protected:
    virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
    LightEraser ( ) : Filter() { };
    virtual ~LightEraser ( ) { if (_filter != NULL) g_free((void *)_filter); return; }
    
    static void init (void) {
        Inkscape::Extension::build_from_mem(
            "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
              "<name>" N_("Light Eraser") "</name>\n"
              "<id>org.inkscape.effect.filter.LightEraser</id>\n"
              "<param name=\"expand\" _gui-text=\"" N_("Expansion") "\" type=\"float\" appearance=\"full\"  min=\"0\" max=\"1000\">50</param>\n"
              "<param name=\"erode\" _gui-text=\"" N_("Erosion") "\" type=\"float\" appearance=\"full\" min=\"1\" max=\"1000\">100</param>\n"
              "<param name=\"opacity\" _gui-text=\"" N_("Global opacity") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.\" max=\"1.\">1</param>\n"
              "<param name=\"invert\" _gui-text=\"" N_("Inverted") "\" type=\"boolean\">false</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Fill and Transparency") "\"/>\n"
                  "</submenu>\n"
                "</effects-menu>\n"
                "<menu-tip>" N_("Make the lightest parts of the object progressively transparent") "</menu-tip>\n"
              "</effect>\n"
            "</inkscape-extension>\n", new LightEraser());
    };
};

gchar const *
LightEraser::get_filter_text (Inkscape::Extension::Extension * ext)
{
    if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream expand;
    std::ostringstream erode;
    std::ostringstream opacity;

    opacity << ext->get_param_float("opacity");

    if (ext->get_param_bool("invert")) {
        expand << (ext->get_param_float("erode") * 0.2125) << " "
               << (ext->get_param_float("erode") * 0.7154) << " "
               << (ext->get_param_float("erode") * 0.0721);
        erode << (-ext->get_param_float("expand"));
    } else {
        expand << (-ext->get_param_float("erode") * 0.2125) << " "
               << (-ext->get_param_float("erode") * 0.7154) << " "
               << (-ext->get_param_float("erode") * 0.0721);
        erode << ext->get_param_float("expand");
    }

    _filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" inkscape:label=\"Light Eraser\" style=\"color-interpolation-filters:sRGB;\" >\n"
          "<feColorMatrix values=\"1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 %s %s 0 \" result=\"colormatrix\" />\n"
          "<feComposite in2=\"colormatrix\" operator=\"arithmetic\" k2=\"%s\" result=\"composite\" />\n"
        "</filter>\n", expand.str().c_str(), erode.str().c_str(), opacity.str().c_str());

    return _filter;
}; /* Light Eraser filter */


/**
    \brief    Custom predefined Opacity filter.
    
    Set opacity and strength of opacity boundaries.

    Filter's parameters:
    * Expansion (0.->1000., default 5) -> colormatrix (last-1th value)
    * Erosion (0.->1000., default 1) -> colormatrix (last value)
    * Global opacity (0.->1., default 1.) -> composite (k2)
    
*/
class Opacity : public Inkscape::Extension::Internal::Filter::Filter {
protected:
    virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
    Opacity ( ) : Filter() { };
    virtual ~Opacity ( ) { if (_filter != NULL) g_free((void *)_filter); return; }
    
    static void init (void) {
        Inkscape::Extension::build_from_mem(
            "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
              "<name>" N_("Opacity") "</name>\n"
              "<id>org.inkscape.effect.filter.Opacity</id>\n"
              "<param name=\"expand\" _gui-text=\"" N_("Expansion") "\" type=\"float\" appearance=\"full\"  min=\"1\" max=\"1000\">5</param>\n"
              "<param name=\"erode\" _gui-text=\"" N_("Erosion") "\" type=\"float\" appearance=\"full\" min=\"0\" max=\"1000\">1</param>\n"
              "<param name=\"opacity\" _gui-text=\"" N_("Global opacity") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.\" max=\"1.\">1</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Fill and Transparency") "\"/>\n"
                  "</submenu>\n"
                "</effects-menu>\n"
                "<menu-tip>" N_("Set opacity and strength of opacity boundaries") "</menu-tip>\n"
              "</effect>\n"
            "</inkscape-extension>\n", new Opacity());
    };
};

gchar const *
Opacity::get_filter_text (Inkscape::Extension::Extension * ext)
{
    if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream matrix;
    std::ostringstream opacity;

    opacity << ext->get_param_float("opacity");

    matrix << (ext->get_param_float("expand")) << " "
           << (-ext->get_param_float("erode"));

    _filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" inkscape:label=\"Opacity\" style=\"color-interpolation-filters:sRGB;\" >\n"
          "<feColorMatrix values=\"1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 0 0 0 %s \" result=\"colormatrix\" />\n"
          "<feComposite in2=\"colormatrix\" operator=\"arithmetic\" k2=\"%s\" result=\"composite\" />\n"
        "</filter>\n", matrix.str().c_str(), opacity.str().c_str());

    return _filter;
}; /* Opacity filter */

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
              "<name>" N_("Silhouette") "</name>\n"
              "<id>org.inkscape.effect.filter.Silhouette</id>\n"
              "<param name=\"blur\" _gui-text=\"" N_("Blur") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.01\" max=\"50.00\">0.01</param>\n"
              "<param name=\"cutout\" _gui-text=\"" N_("Cutout") "\" type=\"boolean\">false</param>\n"
              "<param name=\"color\" _gui-text=\"" N_("Color") "\" type=\"color\">255</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Fill and Transparency") "\"/>\n"
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
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" style=\"color-interpolation-filters:sRGB;\" inkscape:label=\"Silhouette\">\n"
          "<feFlood flood-opacity=\"%s\" flood-color=\"rgb(%s,%s,%s)\" result=\"flood\" />\n"
          "<feComposite in=\"flood\" in2=\"SourceGraphic\" operator=\"%s\" result=\"composite\" />\n"
          "<feGaussianBlur stdDeviation=\"%s\" />\n"
        "</filter>\n", a.str().c_str(), r.str().c_str(), g.str().c_str(), b.str().c_str(), cutout.str().c_str(), blur.str().c_str());

    return _filter;
}; /* Silhouette filter */

}; /* namespace Filter */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */

/* Change the 'TRANSPARENCY' below to be your file name */
#endif /* SEEN_INKSCAPE_EXTENSION_INTERNAL_FILTER_TRANSPARENCY_H__ */
