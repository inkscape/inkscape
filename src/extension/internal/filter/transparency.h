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
    
    Blend objecs with background images or with themselves

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
              "<param name=\"source\" gui-text=\"" N_("Source:") "\" type=\"enum\">\n"
                "<_item value=\"BackgroundImage\">" N_("Background") "</_item>\n"
                "<_item value=\"SourceGraphic\">" N_("Image") "</_item>\n"
              "</param>\n"
              "<param name=\"mode\" gui-text=\"" N_("Mode:") "\" type=\"enum\">\n"
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
                "<menu-tip>" N_("Blend objecs with background images or with themselves") "</menu-tip>\n"
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
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" inkscape:label=\"Blend\">\n"
          "<feBlend blend=\"normal\" in2=\"%s\" mode=\"%s\" result=\"blend\" />\n"
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
              "<param name=\"red\" gui-text=\"" N_("Red:") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"-10.00\" max=\"10.00\">-1</param>\n"
              "<param name=\"green\" gui-text=\"" N_("Green:") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"-10.00\" max=\"10.00\">0.5</param>\n"
              "<param name=\"blue\" gui-text=\"" N_("Blue:") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"-10.00\" max=\"10.00\">0.5</param>\n"
              "<param name=\"alpha\" gui-text=\"" N_("Alpha:") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.\" max=\"1.\">1</param>\n"
              "<param name=\"invert\" gui-text=\"" N_("Inverted") "\" type=\"boolean\">false</param>\n"
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
        "<filter inkscape:label=\"Channel Transparency\" color-interpolation-filters=\"sRGB\" x=\"0\" y=\"0\" width=\"1\" height=\"1\">\n"
          "<feColorMatrix values=\"1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 %s %s %s %s 0 \" in=\"SourceGraphic\" result=\"colormatrix\" />\n"
          "<feComposite in=\"colormatrix\" in2=\"SourceGraphic\" operator=\"%s\" result=\"composite1\" />\n"
        "</filter>\n", red.str().c_str(), green.str().c_str(), blue.str().c_str(), alpha.str().c_str(),
                       invert.str().c_str());

    return _filter;
}; /* Channel transparency filter */

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
              "<param name=\"blur\" gui-text=\"" N_("Blur:") "\" type=\"float\" appearance=\"full\" precision=\"2\" min=\"0.01\" max=\"50.00\">0.01</param>\n"
              "<param name=\"cutout\" gui-text=\"" N_("Cutout") "\" type=\"boolean\">false</param>\n"
              "<param name=\"color\" gui-text=\"" N_("Color") "\" type=\"color\">255</param>\n"
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
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" inkscape:label=\"Silhouette\">\n"
          "<feFlood in=\"SourceGraphic\" flood-opacity=\"%s\" flood-color=\"rgb(%s,%s,%s)\" result=\"flood\" />\n"
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
