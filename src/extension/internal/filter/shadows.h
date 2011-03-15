#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_SHADOWS_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_SHADOWS_H__
/* Change the 'SHADOWS' above to be your file name */

/*
 * Copyright (C) 2011 Authors:
 *   Ivan Louette (filters)
 *   Nicolas Dufour (UI) <nicoduf@yahoo.fr>
 *
 * Color filters
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
    * Color (guint, default 0,0,0,127) -> flood (flood-opacity, flood-color)
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
                "<name>" N_("Drop shadow, custom (Shadows and Glows)") "</name>\n"
                "<id>org.inkscape.effect.filter.ColorDropShadow</id>\n"
                "<param name=\"blur\" gui-text=\"" N_("Blur radius (px):") "\" type=\"float\" appearance=\"full\" min=\"0.0\" max=\"200.0\">3.0</param>\n"
                "<param name=\"xoffset\" gui-text=\"" N_("Horizontal offset (px):") "\" type=\"float\" appearance=\"full\" min=\"-50.0\" max=\"50.0\">6.0</param>\n"
                "<param name=\"yoffset\" gui-text=\"" N_("Vertical offset (px):") "\" type=\"float\" appearance=\"full\" min=\"-50.0\" max=\"50.0\">6.0</param>\n"
                "<param name=\"color\" gui-text=\"" N_("Color") "\" type=\"color\">127</param>\n"
                "<effect>\n"
                    "<object-type>all</object-type>\n"
                    "<effects-menu>\n"
                        "<submenu name=\"" N_("Filters") "\">\n"
                           "<submenu name=\"" N_("Experimental") "\"/>\n"
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

    guint32 color = ext->get_param_color("color");

    blur << ext->get_param_float("blur");
    x << ext->get_param_float("xoffset");
    y << ext->get_param_float("yoffset");
    a << (color & 0xff) / 255.0F;
    r << ((color >> 24) & 0xff);
    g << ((color >> 16) & 0xff);
    b << ((color >>  8) & 0xff);

    _filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" color-interpolation-filters=\"sRGB\" height=\"1.2\" width=\"1.2\" y=\"-0.1\" x=\"-0.1\" inkscape:label=\"Drop shadow, custom\">\n"
          "<feFlood flood-opacity=\"%s\" flood-color=\"rgb(%s,%s,%s)\" result=\"flood\" />\n"
          "<feComposite in=\"flood\" in2=\"SourceGraphic\" operator=\"in\" result=\"composite1\" />\n"
          "<feGaussianBlur in=\"composite1\" stdDeviation=\"%s\" result=\"blur\" />\n"
          "<feOffset dx=\"%s\" dy=\"%s\" result=\"offset\" />\n"
          "<feComposite in=\"SourceGraphic\" in2=\"offset\" operator=\"over\" result=\"composite2\" />\n"
        "</filter>\n", a.str().c_str(), r.str().c_str(), g.str().c_str(), b.str().c_str(), blur.str().c_str(), x.str().c_str(), y.str().c_str());

    return _filter;
};

}; /* namespace Filter */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */

/* Change the 'SHADOWS' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_SHADOWS_H__ */
