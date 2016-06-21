#ifndef SEEN_INKSCAPE_EXTENSION_INTERNAL_FILTER_IMAGE_H__
#define SEEN_INKSCAPE_EXTENSION_INTERNAL_FILTER_IMAGE_H__
/* Change the 'IMAGE' above to be your file name */

/*
 * Copyright (C) 2011 Authors:
 *   Ivan Louette (filters)
 *   Nicolas Dufour (UI) <nicoduf@yahoo.fr>
 *
 * Image filters
 *   Edge detect
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
    \brief    Custom predefined Edge detect filter.
    
    Detect color edges in object.

    Filter's parameters:
    * Detection type (enum, default Full) -> convolve (kernelMatrix)
    * Level (0.01->10., default 1.) -> convolve (divisor)
    * Inverted (boolean, default false) -> convolve (bias)
*/
class EdgeDetect : public Inkscape::Extension::Internal::Filter::Filter {
protected:
    virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
    EdgeDetect ( ) : Filter() { };
    virtual ~EdgeDetect ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

    static void init (void) {
        Inkscape::Extension::build_from_mem(
            "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
              "<name>" N_("Edge Detect") "</name>\n"
              "<id>org.inkscape.effect.filter.EdgeDetect</id>\n"
              "<param name=\"type\" _gui-text=\"" N_("Detect:") "\" type=\"enum\" >\n"
                "<_item value=\"all\">" N_("All") "</_item>\n"
                "<_item value=\"vertical\">" N_("Vertical lines") "</_item>\n"
                "<_item value=\"horizontal\">" N_("Horizontal lines") "</_item>\n"
              "</param>\n"
              "<param name=\"level\" _gui-text=\"" N_("Level") "\" type=\"float\" appearance=\"full\" min=\"0.1\" max=\"100.0\">1.0</param>\n"
              "<param name=\"inverted\" _gui-text=\"" N_("Invert colors") "\" type=\"boolean\" >false</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                     "<submenu name=\"" N_("Image Effects") "\"/>\n"
                  "</submenu>\n"
                "</effects-menu>\n"
              "<menu-tip>" N_("Detect color edges in object") "</menu-tip>\n"
              "</effect>\n"
            "</inkscape-extension>\n", new EdgeDetect());
    };

};

gchar const *
EdgeDetect::get_filter_text (Inkscape::Extension::Extension * ext)
{
    if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream matrix;
    std::ostringstream inverted;
    std::ostringstream level;
    
    const gchar *type = ext->get_param_enum("type");

    level << 1 / ext->get_param_float("level");
    
    if ((g_ascii_strcasecmp("vertical", type) == 0)) {
        matrix << "0 0 0 1 -2 1 0 0 0";
    } else if ((g_ascii_strcasecmp("horizontal", type) == 0)) {
        matrix << "0 1 0 0 -2 0 0 1 0";
    } else {
        matrix << "0 1 0 1 -4 1 0 1 0";
    }
    
    if (ext->get_param_bool("inverted")) {
        inverted << "1";
    } else {
        inverted << "0";
    }
    
    _filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" style=\"color-interpolation-filters:sRGB;\" inkscape:label=\"Edge Detect\">\n"
          "<feConvolveMatrix in=\"SourceGraphic\" kernelMatrix=\"%s\" order=\"3 3\" bias=\"%s\" divisor=\"%s\" targetX=\"1\" targetY=\"1\" preserveAlpha=\"true\" result=\"convolve\" />\n"
        "</filter>\n", matrix.str().c_str(), inverted.str().c_str(), level.str().c_str());

    return _filter;
};

}; /* namespace Filter */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */

/* Change the 'IMAGE' below to be your file name */
#endif /* SEEN_INKSCAPE_EXTENSION_INTERNAL_FILTER_IMAGE_H__ */
