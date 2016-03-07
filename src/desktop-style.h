#ifndef SEEN_SP_DESKTOP_STYLE_H
#define SEEN_SP_DESKTOP_STYLE_H

/*
 * Desktop style management
 *
 * Authors:
 *   bulia byak
 *   verbalshadow
 *
 * Copyright (C) 2004, 2006 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

class ColorRGBA;
class SPCSSAttr;
class SPDesktop;
class SPObject;
class SPItem;
class SPStyle;
namespace Inkscape {
namespace XML {
class Node;
}
}
namespace Glib { class ustring; }

enum { // what kind of a style the query is returning
    QUERY_STYLE_NOTHING,   // nothing was queried - e.g. no selection
    QUERY_STYLE_SINGLE,  // single object was queried
    QUERY_STYLE_MULTIPLE_SAME, // multiple objects were queried, the results were the same
    QUERY_STYLE_MULTIPLE_DIFFERENT, // multiple objects were queried, the results could NOT be meaningfully averaged
    QUERY_STYLE_MULTIPLE_AVERAGED // multiple objects were queried, the results were meaningfully averaged
};

enum { // which property was queried (add when you need more)
    QUERY_STYLE_PROPERTY_EVERYTHING,
    QUERY_STYLE_PROPERTY_FILL,  // fill, fill-opacity
    QUERY_STYLE_PROPERTY_STROKE,  // stroke, stroke-opacity
    QUERY_STYLE_PROPERTY_STROKEWIDTH,  // stroke-width
    QUERY_STYLE_PROPERTY_STROKEMITERLIMIT,  // miter limit
    QUERY_STYLE_PROPERTY_STROKEJOIN,  // stroke join
    QUERY_STYLE_PROPERTY_STROKECAP,  // stroke cap
    QUERY_STYLE_PROPERTY_STROKESTYLE, // markers, dasharray, miterlimit, stroke-width, stroke-cap, stroke-join
    QUERY_STYLE_PROPERTY_PAINTORDER, // paint-order
    QUERY_STYLE_PROPERTY_FONT_SPECIFICATION, //-inkscape-font-specification
    QUERY_STYLE_PROPERTY_FONTFAMILY, // font-family
    QUERY_STYLE_PROPERTY_FONTSTYLE, // font style 
    QUERY_STYLE_PROPERTY_FONTVARIANTS, // font variants (OpenType features)
    QUERY_STYLE_PROPERTY_FONTFEATURESETTINGS, // font feature settings (OpenType features)
    QUERY_STYLE_PROPERTY_FONTNUMBERS, // size, spacings
    QUERY_STYLE_PROPERTY_BASELINES, // baseline-shift
    QUERY_STYLE_PROPERTY_WRITINGMODES, // writing-mode, text-orientation
    QUERY_STYLE_PROPERTY_MASTEROPACITY, // opacity
    QUERY_STYLE_PROPERTY_BLEND, // blend
    QUERY_STYLE_PROPERTY_BLUR // blur
};

void sp_desktop_apply_css_recursive(SPObject *o, SPCSSAttr *css, bool skip_lines);
void sp_desktop_set_color(SPDesktop *desktop, ColorRGBA const &color, bool is_relative, bool fill);
void sp_desktop_set_style(SPDesktop *desktop, SPCSSAttr *css, bool change = true, bool write_current = true);
SPCSSAttr *sp_desktop_get_style(SPDesktop *desktop, bool with_text);
guint32 sp_desktop_get_color (SPDesktop *desktop, bool is_fill);
double sp_desktop_get_master_opacity_tool(SPDesktop *desktop, Glib::ustring const &tool, bool* has_opacity = NULL);
double sp_desktop_get_opacity_tool(SPDesktop *desktop, Glib::ustring const &tool, bool is_fill);
guint32 sp_desktop_get_color_tool(SPDesktop *desktop, Glib::ustring const &tool, bool is_fill, bool* has_color = NULL);
double sp_desktop_get_font_size_tool (SPDesktop *desktop);
void sp_desktop_apply_style_tool(SPDesktop *desktop, Inkscape::XML::Node *repr, Glib::ustring const &tool, bool with_text);

gdouble stroke_average_width (const std::vector<SPItem*> &objects);

int objects_query_fillstroke (const std::vector<SPItem*> &objects, SPStyle *style_res, bool const isfill);
int objects_query_fontnumbers (const std::vector<SPItem*> &objects, SPStyle *style_res);
int objects_query_fontstyle (const std::vector<SPItem*> &objects, SPStyle *style_res);
int objects_query_fontfamily (const std::vector<SPItem*> &objects, SPStyle *style_res);
int objects_query_fontvariants (const std::vector<SPItem*> &objects, SPStyle *style_res);
int objects_query_opacity (const std::vector<SPItem*> &objects, SPStyle *style_res);
int objects_query_strokewidth (const std::vector<SPItem*> &objects, SPStyle *style_res);
int objects_query_miterlimit (const std::vector<SPItem*> &objects, SPStyle *style_res);
int objects_query_strokecap (const std::vector<SPItem*> &objects, SPStyle *style_res);
int objects_query_strokejoin (const std::vector<SPItem*> &objects, SPStyle *style_res);

int objects_query_blur (const std::vector<SPItem*> &objects, SPStyle *style_res);

int sp_desktop_query_style_from_list (const std::vector<SPItem*> &list, SPStyle *style, int property);
int sp_desktop_query_style(SPDesktop *desktop, SPStyle *style, int property);
bool sp_desktop_query_style_all (SPDesktop *desktop, SPStyle *query);

#endif // SEEN_SP_DESKTOP_STYLE_H


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
