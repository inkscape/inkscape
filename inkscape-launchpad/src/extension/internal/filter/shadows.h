#ifndef SEEN_INKSCAPE_EXTENSION_INTERNAL_FILTER_SHADOWS_H__
#define SEEN_INKSCAPE_EXTENSION_INTERNAL_FILTER_SHADOWS_H__
/* Change the 'SHADOWS' above to be your file name */

/*
 * Copyright (C) 2013 Authors:
 *   Ivan Louette (filters)
 *   Nicolas Dufour (UI) <nicoduf@yahoo.fr>
 *
 * Shadow filters
 *   Drop shadow
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
    \brief    Custom predefined Drop shadow filter.
    
    Colorizable Drop shadow.

    Filter's parameters:
    * Blur radius (0.->200., default 3) -> blur (stdDeviation)
    * Horizontal offset (-50.->50., default 6.0) -> offset (dx)
    * Vertical offset (-50.->50., default 6.0) -> offset (dy)
    * Blur type (enum, default outer) ->
        outer = composite1 (operator="in"), composite2 (operator="over", in1="SourceGraphic", in2="offset")
        inner = composite1 (operator="out"), composite2 (operator="atop", in1="offset", in2="SourceGraphic")
        innercut = composite1 (operator="in"), composite2 (operator="out", in1="offset", in2="SourceGraphic")
        outercut = composite1 (operator="out"), composite2 (operator="in", in1="SourceGraphic", in2="offset")
        shadow = composite1 (operator="out"), composite2 (operator="atop", in1="offset", in2="offset")
    * Color (guint, default 0,0,0,127) -> flood (flood-opacity, flood-color)
    * Use object's color (boolean, default false) -> composite1 (in1, in2)
*/
class ColorizableDropShadow : public Inkscape::Extension::Internal::Filter::Filter {
protected:
    virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
    ColorizableDropShadow ( ) : Filter() { };
    virtual ~ColorizableDropShadow ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

    static void init (void) {
        Inkscape::Extension::build_from_mem(
            "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
              "<name>" N_("Drop Shadow") "</name>\n"
              "<id>org.inkscape.effect.filter.ColorDropShadow</id>\n"
              "<param name=\"tab\" type=\"notebook\">\n"
                "<page name=\"optionstab\" _gui-text=\"" N_("Options") "\">\n"
                  "<param name=\"blur\" _gui-text=\"" N_("Blur radius (px)") "\" type=\"float\" appearance=\"full\" min=\"0.0\" max=\"200.0\">3.0</param>\n"
                  "<param name=\"xoffset\" _gui-text=\"" N_("Horizontal offset (px)") "\" type=\"float\" appearance=\"full\" min=\"-50.0\" max=\"50.0\">6.0</param>\n"
                  "<param name=\"yoffset\" _gui-text=\"" N_("Vertical offset (px)") "\" type=\"float\" appearance=\"full\" min=\"-50.0\" max=\"50.0\">6.0</param>\n"
                  "<param name=\"type\" _gui-text=\"" N_("Shadow type:") "\" type=\"enum\" >\n"
                    "<_item value=\"outer\">" N_("Outer") "</_item>\n"
                    "<_item value=\"inner\">" N_("Inner") "</_item>\n"
                    "<_item value=\"outercut\">" N_("Outer cutout") "</_item>\n"
                    "<_item value=\"innercut\">" N_("Inner cutout") "</_item>\n"
                    "<_item value=\"shadow\">" N_("Shadow only") "</_item>\n"
                  "</param>\n"
                "</page>\n"
                "<page name=\"coltab\" _gui-text=\"" N_("Blur color") "\">\n"
                  "<param name=\"color\" _gui-text=\"" N_("Color") "\" type=\"color\">127</param>\n"
                  "<param name=\"objcolor\" _gui-text=\"" N_("Use object's color") "\" type=\"boolean\" >false</param>\n"
                "</page>\n"
              "</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                     "<submenu name=\"" N_("Shadows and Glows") "\"/>\n"
                  "</submenu>\n"
                "</effects-menu>\n"
              "<menu-tip>" N_("Colorizable Drop shadow") "</menu-tip>\n"
              "</effect>\n"
            "</inkscape-extension>\n", new ColorizableDropShadow());
    };

};

gchar const *
ColorizableDropShadow::get_filter_text (Inkscape::Extension::Extension * ext)
{
    if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream blur;
    std::ostringstream a;
    std::ostringstream r;
    std::ostringstream g;
    std::ostringstream b;
    std::ostringstream x;
    std::ostringstream y;
    std::ostringstream comp1in1;
    std::ostringstream comp1in2;
    std::ostringstream comp1op;
    std::ostringstream comp2in1;
    std::ostringstream comp2in2;
    std::ostringstream comp2op;
    
    const gchar *type = ext->get_param_enum("type");
    guint32 color = ext->get_param_color("color");

    blur << ext->get_param_float("blur");
    x << ext->get_param_float("xoffset");
    y << ext->get_param_float("yoffset");
    a << (color & 0xff) / 255.0F;
    r << ((color >> 24) & 0xff);
    g << ((color >> 16) & 0xff);
    b << ((color >>  8) & 0xff);

    // Select object or user-defined color
    if ((g_ascii_strcasecmp("innercut", type) == 0)) {
        if (ext->get_param_bool("objcolor")) {
            comp2in1 << "SourceGraphic";
            comp2in2 << "offset";
        } else {
            comp2in1 << "offset";
            comp2in2 << "SourceGraphic";
        }
    } else {
        if (ext->get_param_bool("objcolor")) {
            comp1in1 << "SourceGraphic";
            comp1in2 << "flood";
        } else {
            comp1in1 << "flood";
            comp1in2 << "SourceGraphic";
        }
    }

    // Shadow mode
    if ((g_ascii_strcasecmp("outer", type) == 0)) {
        comp1op << "in";
        comp2op << "over";
        comp2in1 << "SourceGraphic";
        comp2in2 << "offset";
    } else if ((g_ascii_strcasecmp("inner", type) == 0)) {
        comp1op << "out";
        comp2op << "atop";
        comp2in1 << "offset";
        comp2in2 << "SourceGraphic";
    } else if ((g_ascii_strcasecmp("outercut", type) == 0)) {
        comp1op << "in";
        comp2op << "out";
        comp2in1 << "offset";
        comp2in2 << "SourceGraphic";
    } else if ((g_ascii_strcasecmp("innercut", type) == 0)){
        comp1op << "out";
        comp1in1 << "flood";
        comp1in2 << "SourceGraphic";
        comp2op << "in";
    } else { //shadow
        comp1op << "in";
        comp2op << "atop";
        comp2in1 << "offset";
        comp2in2 << "offset";
    }

    _filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" style=\"color-interpolation-filters:sRGB;\" inkscape:label=\"Drop Shadow\">\n"
          "<feFlood flood-opacity=\"%s\" flood-color=\"rgb(%s,%s,%s)\" result=\"flood\" />\n"
          "<feComposite in=\"%s\" in2=\"%s\" operator=\"%s\" result=\"composite1\" />\n"
          "<feGaussianBlur in=\"composite1\" stdDeviation=\"%s\" result=\"blur\" />\n"
          "<feOffset dx=\"%s\" dy=\"%s\" result=\"offset\" />\n"
          "<feComposite in=\"%s\" in2=\"%s\" operator=\"%s\" result=\"composite2\" />\n"
        "</filter>\n", a.str().c_str(), r.str().c_str(), g.str().c_str(), b.str().c_str(),
                       comp1in1.str().c_str(), comp1in2.str().c_str(), comp1op.str().c_str(),
                       blur.str().c_str(), x.str().c_str(), y.str().c_str(),
                       comp2in1.str().c_str(), comp2in2.str().c_str(), comp2op.str().c_str());

    return _filter;
}; /* Drop shadow filter */

}; /* namespace Filter */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */

/* Change the 'SHADOWS' below to be your file name */
#endif /* SEEN_INKSCAPE_EXTENSION_INTERNAL_FILTER_SHADOWS_H__ */
