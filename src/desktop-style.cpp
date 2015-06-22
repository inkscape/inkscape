/*
 * Desktop style management
 *
 * Authors:
 *   bulia byak
 *   verbalshadow
 *   Jon A. Cruz <jon@joncruz.org> 
 *   Abhishek Sharma
 *
 * Copyright (C) 2004, 2006 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <string>
#include <cstring>
#include <glibmm.h>

#include "desktop.h"
#include "color-rgba.h"
#include "svg/css-ostringstream.h"
#include "svg/svg.h"
#include "svg/svg-color.h"
#include "selection.h"
#include "inkscape.h"
#include "style.h"
#include "preferences.h"
#include "sp-use.h"
#include "filters/blend.h"
#include "sp-filter.h"
#include "sp-filter-reference.h"
#include "filters/gaussian-blur.h"
#include "sp-flowtext.h"
#include "sp-flowregion.h"
#include "sp-flowdiv.h"
#include "sp-linear-gradient.h"
#include "sp-pattern.h"
#include "sp-radial-gradient.h"
#include "sp-textpath.h"
#include "sp-tref.h"
#include "sp-tspan.h"
#include "xml/repr.h"
#include "xml/sp-css-attr.h"
#include "sp-path.h"
#include "ui/tools/tool-base.h"

#include "desktop-style.h"
#include "svg/svg-icc-color.h"
#include "box3d-side.h"
#include <2geom/math-utils.h>

namespace {

bool isTextualItem(SPObject const *obj)
{
    bool isTextual = dynamic_cast<SPText const *>(obj) //
        || dynamic_cast<SPFlowtext const *>(obj) //
        || dynamic_cast<SPTSpan const *>(obj) //
        || dynamic_cast<SPTRef const *>(obj) //
        || dynamic_cast<SPTextPath const *>(obj) //
        || dynamic_cast<SPFlowdiv const *>(obj) //
        || dynamic_cast<SPFlowpara const *>(obj) //
        || dynamic_cast<SPFlowtspan const *>(obj);

    return isTextual;
}

} // namespace

/**
 * Set color on selection on desktop.
 */
void
sp_desktop_set_color(SPDesktop *desktop, ColorRGBA const &color, bool is_relative, bool fill)
{
    /// \todo relative color setting
    if (is_relative) {
        g_warning("FIXME: relative color setting not yet implemented");
        return;
    }

    guint32 rgba = SP_RGBA32_F_COMPOSE(color[0], color[1], color[2], color[3]);
    gchar b[64];
    sp_svg_write_color(b, sizeof(b), rgba);
    SPCSSAttr *css = sp_repr_css_attr_new();
    if (fill) {
        sp_repr_css_set_property(css, "fill", b);
        Inkscape::CSSOStringStream osalpha;
        osalpha << color[3];
        sp_repr_css_set_property(css, "fill-opacity", osalpha.str().c_str());
    } else {
        sp_repr_css_set_property(css, "stroke", b);
        Inkscape::CSSOStringStream osalpha;
        osalpha << color[3];
        sp_repr_css_set_property(css, "stroke-opacity", osalpha.str().c_str());
    }

    sp_desktop_set_style(desktop, css);

    sp_repr_css_attr_unref(css);
}

/**
 * Apply style on object and children, recursively.
 */
void
sp_desktop_apply_css_recursive(SPObject *o, SPCSSAttr *css, bool skip_lines)
{
    // non-items should not have style
    SPItem *item = dynamic_cast<SPItem *>(o);
    if (!item) {
        return;
    }

    // 1. tspans with role=line are not regular objects in that they are not supposed to have style of their own,
    // but must always inherit from the parent text. Same for textPath.
    // However, if the line tspan or textPath contains some style (old file?), we reluctantly set our style to it too.

    // 2. Generally we allow setting style on clones, but when it's inside flowRegion, do not touch
    // it, be it clone or not; it's just styleless shape (because that's how Inkscape does
    // flowtext).

    SPTSpan *tspan = dynamic_cast<SPTSpan *>(o);

    if (!(skip_lines
          && ((tspan && tspan->role == SP_TSPAN_ROLE_LINE)
              || dynamic_cast<SPFlowdiv *>(o)
              || dynamic_cast<SPFlowpara *>(o)
              || dynamic_cast<SPTextPath *>(o))
          &&  !o->getAttribute("style"))
        &&
        !(dynamic_cast<SPFlowregionbreak *>(o) ||
          dynamic_cast<SPFlowregionExclude *>(o) ||
          (dynamic_cast<SPUse *>(o) &&
           o->parent &&
           (dynamic_cast<SPFlowregion *>(o->parent) ||
            dynamic_cast<SPFlowregionExclude *>(o->parent)
               )
              )
            )
        ) {

        SPCSSAttr *css_set = sp_repr_css_attr_new();
        sp_repr_css_merge(css_set, css);

        // Scale the style by the inverse of the accumulated parent transform in the paste context.
        {
            Geom::Affine const local(item->i2doc_affine());
            double const ex(local.descrim());
            if ( ( ex != 0. )
                 && ( ex != 1. ) ) {
                sp_css_attr_scale(css_set, 1/ex);
            }
        }

        o->changeCSS(css_set,"style");

        sp_repr_css_attr_unref(css_set);
    }

    // setting style on child of clone spills into the clone original (via shared repr), don't do it!
    if (dynamic_cast<SPUse *>(o)) {
        return;
    }

    for ( SPObject *child = o->firstChild() ; child ; child = child->getNext() ) {
        if (sp_repr_css_property(css, "opacity", NULL) != NULL) {
            // Unset properties which are accumulating and thus should not be set recursively.
            // For example, setting opacity 0.5 on a group recursively would result in the visible opacity of 0.25 for an item in the group.
            SPCSSAttr *css_recurse = sp_repr_css_attr_new();
            sp_repr_css_merge(css_recurse, css);
            sp_repr_css_set_property(css_recurse, "opacity", NULL);
            sp_desktop_apply_css_recursive(child, css_recurse, skip_lines);
            sp_repr_css_attr_unref(css_recurse);
        } else {
            sp_desktop_apply_css_recursive(child, css, skip_lines);
        }
    }
}

/**
 * Apply style on selection on desktop.
 */
void
sp_desktop_set_style(SPDesktop *desktop, SPCSSAttr *css, bool change, bool write_current)
{
    if (write_current) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        // 1. Set internal value
        sp_repr_css_merge(desktop->current, css);

        // 1a. Write to prefs; make a copy and unset any URIs first
        SPCSSAttr *css_write = sp_repr_css_attr_new();
        sp_repr_css_merge(css_write, css);
        sp_css_attr_unset_uris(css_write);
        prefs->mergeStyle("/desktop/style", css_write);
        std::vector<SPItem*> const itemlist = desktop->selection->itemList();
        for (std::vector<SPItem*>::const_iterator i = itemlist.begin(); i!= itemlist.end(); i++) {
            /* last used styles for 3D box faces are stored separately */
            SPObject *obj = *i;
            Box3DSide *side = dynamic_cast<Box3DSide *>(obj);
            if (side) {
                const char * descr  = box3d_side_axes_string(side);
                if (descr != NULL) {
                    prefs->mergeStyle(Glib::ustring("/desktop/") + descr + "/style", css_write);
                }
            }
        }
        sp_repr_css_attr_unref(css_write);
    }

    if (!change)
        return;

// 2. Emit signal
    bool intercepted = desktop->_set_style_signal.emit(css);

/** \todo
 * FIXME: in set_style, compensate pattern and gradient fills, stroke width,
 * rect corners, font size for the object's own transform so that pasting
 * fills does not depend on preserve/optimize.
 */

// 3. If nobody has intercepted the signal, apply the style to the selection
    if (!intercepted) {
        // If we have an event context, update its cursor (TODO: it could be neater to do this with the signal sent above, but what if the signal gets intercepted?)
        if (desktop->event_context) {
            desktop->event_context->sp_event_context_update_cursor();
        }

        // Remove text attributes if not text...
        // Do this once in case a zillion objects are selected.
        SPCSSAttr *css_no_text = sp_repr_css_attr_new();
        sp_repr_css_merge(css_no_text, css);
        css_no_text = sp_css_attr_unset_text(css_no_text);

        std::vector<SPItem*> const itemlist = desktop->selection->itemList();
        for (std::vector<SPItem*>::const_iterator i = itemlist.begin(); i!= itemlist.end(); i++) {
            SPItem *item = *i;

            // If not text, don't apply text attributes (can a group have text attributes? Yes! FIXME)
            if (isTextualItem(item)) {

                // If any font property has changed, then we have written out the font
                // properties in longhand and we need to remove the 'font' shorthand.
                if( !sp_repr_css_property_is_unset(css, "font-family") ) {
                    sp_repr_css_unset_property(css, "font");
                }

                sp_desktop_apply_css_recursive(item, css, true);

            } else {

                sp_desktop_apply_css_recursive(item, css_no_text, true);

            }
        }
        sp_repr_css_attr_unref(css_no_text);
    }
}

/**
 * Return the desktop's current style.
 */
SPCSSAttr *
sp_desktop_get_style(SPDesktop *desktop, bool with_text)
{
    SPCSSAttr *css = sp_repr_css_attr_new();
    sp_repr_css_merge(css, desktop->current);
    if (!css->attributeList()) {
        sp_repr_css_attr_unref(css);
        return NULL;
    } else {
        if (!with_text) {
            css = sp_css_attr_unset_text(css);
        }
        return css;
    }
}

/**
 * Return the desktop's current color.
 */
guint32
sp_desktop_get_color(SPDesktop *desktop, bool is_fill)
{
    guint32 r = 0; // if there's no color, return black
    gchar const *property = sp_repr_css_property(desktop->current,
                                                 is_fill ? "fill" : "stroke",
                                                 "#000");

    if (desktop->current && property) { // if there is style and the property in it,
        if (strncmp(property, "url", 3)) { // and if it's not url,
            // read it
            r = sp_svg_read_color(property, r);
        }
    }

    return r;
}

double
sp_desktop_get_master_opacity_tool(SPDesktop *desktop, Glib::ustring const &tool, bool *has_opacity)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    SPCSSAttr *css = NULL;
    gfloat value = 1.0; // default if nothing else found
    if (has_opacity)
        *has_opacity = false;
    if (prefs->getBool(tool + "/usecurrent")) {
        css = sp_desktop_get_style(desktop, true);
    } else {
        css = prefs->getStyle(tool + "/style");
    }

    if (css) {
        gchar const *property = css ? sp_repr_css_property(css, "opacity", "1.000") : 0;

        if (desktop->current && property) { // if there is style and the property in it,
            if ( !sp_svg_number_read_f(property, &value) ) {
                value = 1.0; // things failed. set back to the default
            } else {
                if (has_opacity)
                   *has_opacity = true;
            }
        }

        sp_repr_css_attr_unref(css);
    }

    return value;
}
double
sp_desktop_get_opacity_tool(SPDesktop *desktop, Glib::ustring const &tool, bool is_fill)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    SPCSSAttr *css = NULL;
    gfloat value = 1.0; // default if nothing else found
    if (prefs->getBool(tool + "/usecurrent")) {
        css = sp_desktop_get_style(desktop, true);
    } else {
        css = prefs->getStyle(tool + "/style");
    }

    if (css) {
        gchar const *property = css ? sp_repr_css_property(css, is_fill ? "fill-opacity": "stroke-opacity", "1.000") : 0;

        if (desktop->current && property) { // if there is style and the property in it,
            if ( !sp_svg_number_read_f(property, &value) ) {
                value = 1.0; // things failed. set back to the default
            }
        }

        sp_repr_css_attr_unref(css);
    }

    return value;
}

guint32
sp_desktop_get_color_tool(SPDesktop *desktop, Glib::ustring const &tool, bool is_fill, bool *has_color)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    SPCSSAttr *css = NULL;
    guint32 r = 0; // if there's no color, return black
    if (has_color)
        *has_color = false;
    if (prefs->getBool(tool + "/usecurrent")) {
        css = sp_desktop_get_style(desktop, true);
    } else {
        css = prefs->getStyle(tool + "/style");
    }

    if (css) {
        gchar const *property = sp_repr_css_property(css, is_fill ? "fill" : "stroke", "#000");

        if (desktop->current && property) { // if there is style and the property in it,
            if (strncmp(property, "url", 3) && strncmp(property, "none", 4)) { // and if it's not url or none,
                // read it
                r = sp_svg_read_color(property, r);
                if (has_color)
                    *has_color = true;
            }
        }

        sp_repr_css_attr_unref(css);
    }

    return r | 0xff;
}

/**
 * Apply the desktop's current style or the tool style to repr.
 */
void
sp_desktop_apply_style_tool(SPDesktop *desktop, Inkscape::XML::Node *repr, Glib::ustring const &tool_path, bool with_text)
{
    SPCSSAttr *css_current = sp_desktop_get_style(desktop, with_text);
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    if (prefs->getBool(tool_path + "/usecurrent") && css_current) {
        sp_repr_css_set(repr, css_current, "style");
    } else {
        SPCSSAttr *css = prefs->getInheritedStyle(tool_path + "/style");
        sp_repr_css_set(repr, css, "style");
        sp_repr_css_attr_unref(css);
    }
    if (css_current) {
        sp_repr_css_attr_unref(css_current);
    }
}

/**
 * Returns the font size (in SVG pixels) of the text tool style (if text
 * tool uses its own style) or desktop style (otherwise).
*/
double
sp_desktop_get_font_size_tool(SPDesktop *desktop)
{
    (void)desktop; // TODO cleanup
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    Glib::ustring desktop_style = prefs->getString("/desktop/style");
    Glib::ustring style_str;
    if ((prefs->getBool("/tools/text/usecurrent")) && !desktop_style.empty()) {
        style_str = desktop_style;
    } else {
        style_str = prefs->getString("/tools/text/style");
    }

    double ret = 12;
    if (!style_str.empty()) {
        SPStyle style(SP_ACTIVE_DOCUMENT);
        style.mergeString(style_str.data());
        ret = style.font_size.computed;
    }
    return ret;
}

/** Determine average stroke width, simple method */
// see TODO in dialogs/stroke-style.cpp on how to get rid of this eventually
gdouble
stroke_average_width (const std::vector<SPItem*> &objects)
{
    if (objects.empty())
        return Geom::infinity();

    gdouble avgwidth = 0.0;
    bool notstroked = true;
    int n_notstroked = 0;
    for (std::vector<SPItem*>::const_iterator i = objects.begin(); i != objects.end(); i++) {
        SPItem *item = *i;
        if (!item) {
            continue;
        }

        Geom::Affine i2dt = item->i2dt_affine();

        double width = item->style->stroke_width.computed * i2dt.descrim();

        if ( item->style->stroke.isNone() || IS_NAN(width)) {
            ++n_notstroked;   // do not count nonstroked objects
            continue;
        } else {
            notstroked = false;
        }

        avgwidth += width;
    }

    if (notstroked)
        return Geom::infinity();

    return avgwidth / (objects.size() - n_notstroked);
}

static bool vectorsClose( std::vector<double> const &lhs, std::vector<double> const &rhs )
{
    bool isClose = false;
    if ( lhs.size() == rhs.size() ) {
        static double epsilon = 1e-6;
        isClose = true;
        for ( size_t i = 0; (i < lhs.size()) && isClose; ++i ) {
            isClose = fabs(lhs[i] - rhs[i]) < epsilon;
        }
    }
    return isClose;
}


/**
 * Write to style_res the average fill or stroke of list of objects, if applicable.
 */
int
objects_query_fillstroke (const std::vector<SPItem*> &objects, SPStyle *style_res, bool const isfill)
{
    if (objects.empty()) {
        /* No objects, set empty */
        return QUERY_STYLE_NOTHING;
    }

    SPIPaint *paint_res = isfill? &style_res->fill : &style_res->stroke;
    bool paintImpossible = true;
    paint_res->set = TRUE;

    SVGICCColor* iccColor = 0;

    bool iccSeen = false;
    gfloat c[4];
    c[0] = c[1] = c[2] = c[3] = 0.0;
    gint num = 0;

    gfloat prev[3];
    prev[0] = prev[1] = prev[2] = 0.0;
    bool same_color = true;

        for (std::vector<SPItem*>::const_iterator i = objects.begin(); i!= objects.end(); i++) {
        SPObject *obj = *i;
        if (!obj) {
            continue;
        }
        SPStyle *style = obj->style;
        if (!style) {
            continue;
        }

        SPIPaint *paint = isfill? &style->fill : &style->stroke;

        // We consider paint "effectively set" for anything within text hierarchy
        SPObject *parent = obj->parent;
        bool paint_effectively_set =
            paint->set || (dynamic_cast<SPText *>(parent) || dynamic_cast<SPTextPath *>(parent) || dynamic_cast<SPTSpan *>(parent)
            || dynamic_cast<SPFlowtext *>(parent) || dynamic_cast<SPFlowdiv *>(parent) || dynamic_cast<SPFlowpara *>(parent)
            || dynamic_cast<SPFlowtspan *>(parent) || dynamic_cast<SPFlowline*>(parent));

        // 1. Bail out with QUERY_STYLE_MULTIPLE_DIFFERENT if necessary
        
        // cppcheck-suppress comparisonOfBoolWithInt
        if ((!paintImpossible) && (!paint->isSameType(*paint_res) || (paint_res->set != paint_effectively_set))) {
            return QUERY_STYLE_MULTIPLE_DIFFERENT;  // different types of paint
        }

        if (paint_res->set && paint->set && paint_res->isPaintserver()) {
            // both previous paint and this paint were a server, see if the servers are compatible

            SPPaintServer *server_res = isfill ? style_res->getFillPaintServer() : style_res->getStrokePaintServer();
            SPPaintServer *server = isfill ? style->getFillPaintServer() : style->getStrokePaintServer();

            SPLinearGradient *linear_res = dynamic_cast<SPLinearGradient *>(server_res);
            SPRadialGradient *radial_res = linear_res ? NULL : dynamic_cast<SPRadialGradient *>(server_res);
            SPPattern *pattern_res = (linear_res || radial_res) ? NULL : dynamic_cast<SPPattern *>(server_res);
            if (linear_res) {
                SPLinearGradient *linear = dynamic_cast<SPLinearGradient *>(server);
                if (!linear) {
                   return QUERY_STYLE_MULTIPLE_DIFFERENT;  // different kind of server
                }

                SPGradient *vector = linear->getVector();
                SPGradient *vector_res = linear_res->getVector();
                if (vector_res != vector) {
                   return QUERY_STYLE_MULTIPLE_DIFFERENT;  // different gradient vectors
                }
            } else if (radial_res) {
                SPRadialGradient *radial = dynamic_cast<SPRadialGradient *>(server);
                if (!radial) {
                   return QUERY_STYLE_MULTIPLE_DIFFERENT;  // different kind of server
                }

                SPGradient *vector = radial->getVector();
                SPGradient *vector_res = radial_res->getVector();
                if (vector_res != vector) {
                   return QUERY_STYLE_MULTIPLE_DIFFERENT;  // different gradient vectors
                }
            } else if (pattern_res) {
                SPPattern *pattern = dynamic_cast<SPPattern *>(server);
                if (!pattern) {
                   return QUERY_STYLE_MULTIPLE_DIFFERENT;  // different kind of server
                }

                SPPattern *pat = SP_PATTERN (server)->rootPattern();
                SPPattern *pat_res = SP_PATTERN (server_res)->rootPattern();
                if (pat_res != pat) {
                   return QUERY_STYLE_MULTIPLE_DIFFERENT;  // different pattern roots
                }
            }
        }

        // 2. Sum color, copy server from paint to paint_res

        if (paint_res->set && paint_effectively_set && paint->isColor()) {
            gfloat d[3];
            sp_color_get_rgb_floatv (&paint->value.color, d);

            // Check if this color is the same as previous
            if (paintImpossible) {
                prev[0] = d[0];
                prev[1] = d[1];
                prev[2] = d[2];
                paint_res->setColor(d[0], d[1], d[2]);
                iccColor = paint->value.color.icc;
                iccSeen = true;
            } else {
                if (same_color && (prev[0] != d[0] || prev[1] != d[1] || prev[2] != d[2])) {
                    same_color = false;
                    iccColor = 0;
                }
                if ( iccSeen && iccColor ) {
                    if ( !paint->value.color.icc
                         || (iccColor->colorProfile != paint->value.color.icc->colorProfile)
                         || !vectorsClose(iccColor->colors, paint->value.color.icc->colors) ) {
                        same_color = false;
                        iccColor = 0;
                    }
                }
            }

            // average color
            c[0] += d[0];
            c[1] += d[1];
            c[2] += d[2];
            c[3] += SP_SCALE24_TO_FLOAT (isfill? style->fill_opacity.value : style->stroke_opacity.value);

            num ++;
        }

       paintImpossible = false;
       paint_res->colorSet = paint->colorSet;
       paint_res->paintOrigin = paint->paintOrigin;
       if (paint_res->set && paint_effectively_set && paint->isPaintserver()) { // copy the server
           gchar const *string = NULL; // memory leak results if style->get* called inside sp_style_set_to_uri_string.
           if (isfill) {
               string = style->getFillURI();
               sp_style_set_to_uri_string (style_res, true, string);
           } else {
               string = style->getStrokeURI();
               sp_style_set_to_uri_string (style_res, false, string);
           }
           if(string)g_free((void *) string);
       }
       paint_res->set = paint_effectively_set;
       style_res->fill_rule.computed = style->fill_rule.computed; // no averaging on this, just use the last one
    }

    // After all objects processed, divide the color if necessary and return
    if (paint_res->set && paint_res->isColor()) { // set the color
        g_assert (num >= 1);

        c[0] /= num;
        c[1] /= num;
        c[2] /= num;
        c[3] /= num;
        paint_res->setColor(c[0], c[1], c[2]);
        if (isfill) {
            style_res->fill_opacity.value = SP_SCALE24_FROM_FLOAT (c[3]);
        } else {
            style_res->stroke_opacity.value = SP_SCALE24_FROM_FLOAT (c[3]);
        }


        if ( iccSeen && iccColor ) {
            // TODO check for existing
            SVGICCColor* tmp = new SVGICCColor(*iccColor);
            paint_res->value.color.icc = tmp;
        }

        if (num > 1) {
            if (same_color)
                return QUERY_STYLE_MULTIPLE_SAME;
            else
                return QUERY_STYLE_MULTIPLE_AVERAGED;
        } else {
            return QUERY_STYLE_SINGLE;
        }
    }

    // Not color
    if (objects.size() > 1) {
        return QUERY_STYLE_MULTIPLE_SAME;
    } else {
        return QUERY_STYLE_SINGLE;
    }
}

/**
 * Write to style_res the average opacity of a list of objects.
 */
int
objects_query_opacity (const std::vector<SPItem*> &objects, SPStyle *style_res)
{
    if (objects.empty()) {
        /* No objects, set empty */
        return QUERY_STYLE_NOTHING;
    }

    gdouble opacity_sum = 0;
    gdouble opacity_prev = -1;
    bool same_opacity = true;
    guint opacity_items = 0;


    for (std::vector<SPItem*>::const_iterator i = objects.begin(); i != objects.end(); i++) {
        SPObject *obj = *i;
        if (!obj) {
            continue;
        }
        SPStyle *style = obj->style;
        if (!style) {
            continue;
        }

        double opacity = SP_SCALE24_TO_FLOAT(style->opacity.value);
        opacity_sum += opacity;
        if (opacity_prev != -1 && opacity != opacity_prev) {
            same_opacity = false;
        }
        opacity_prev = opacity;
        opacity_items ++;
    }
    if (opacity_items > 1) {
        opacity_sum /= opacity_items;
    }

    style_res->opacity.value = SP_SCALE24_FROM_FLOAT(opacity_sum);

    if (opacity_items == 0) {
        return QUERY_STYLE_NOTHING;
    } else if (opacity_items == 1) {
        return QUERY_STYLE_SINGLE;
    } else {
        if (same_opacity) {
            return QUERY_STYLE_MULTIPLE_SAME;
        } else {
            return QUERY_STYLE_MULTIPLE_AVERAGED;
        }
    }
}

/**
 * Write to style_res the average stroke width of a list of objects.
 */
int
objects_query_strokewidth (const std::vector<SPItem*> &objects, SPStyle *style_res)
{
    if (objects.empty()) {
        /* No objects, set empty */
        return QUERY_STYLE_NOTHING;
    }

    gdouble avgwidth = 0.0;

    gdouble prev_sw = -1;
    bool same_sw = true;
    bool noneSet = true; // is stroke set to none?

    int n_stroked = 0;

    for (std::vector<SPItem*>::const_iterator i = objects.begin(); i != objects.end(); i++) {
        SPObject *obj = *i;
        if (!obj) {
            continue;
        }
        SPItem *item = dynamic_cast<SPItem *>(obj);
        if (!item) {
            continue;
        }
        SPStyle *style = obj->style;
        if (!style) {
            continue;
        }

        if ( style->stroke.isNone() && !(
                 style->marker.set       || // stroke width affects markers, so if there's no
                 style->marker_start.set || // stroke but only markers then we should
                 style->marker_mid.set   || // still calculate the stroke width
                 style->marker_end.set))
        {
            continue;
        }

        noneSet &= style->stroke.isNone();

        Geom::Affine i2d = item->i2dt_affine();
        double sw = style->stroke_width.computed * i2d.descrim();

        if (!IS_NAN(sw)) {
            if (prev_sw != -1 && fabs(sw - prev_sw) > 1e-3)
                same_sw = false;
            prev_sw = sw;

            avgwidth += sw;
            n_stroked ++;
        }
    }

    if (n_stroked > 1)
        avgwidth /= (n_stroked);

    style_res->stroke_width.computed = avgwidth;
    style_res->stroke_width.set = true;
    style_res->stroke.noneSet = noneSet; // Will only be true if none of the selected objects has it's stroke set.

    if (n_stroked == 0) {
        return QUERY_STYLE_NOTHING;
    } else if (n_stroked == 1) {
        return QUERY_STYLE_SINGLE;
    } else {
        if (same_sw)
            return QUERY_STYLE_MULTIPLE_SAME;
        else
            return QUERY_STYLE_MULTIPLE_AVERAGED;
    }
}

/**
 * Write to style_res the average miter limit of a list of objects.
 */
int
objects_query_miterlimit (const std::vector<SPItem*> &objects, SPStyle *style_res)
{
    if (objects.empty()) {
        /* No objects, set empty */
        return QUERY_STYLE_NOTHING;
    }

    gdouble avgml = 0.0;
    int n_stroked = 0;

    gdouble prev_ml = -1;
    bool same_ml = true;

    for (std::vector<SPItem*>::const_iterator i = objects.begin(); i != objects.end(); i++) {
        SPObject *obj = *i;
        if (!dynamic_cast<SPItem *>(obj)) {
            continue;
        }
        SPStyle *style = obj->style;
        if (!style) {
            continue;
        }

        if ( style->stroke.isNone() ) {
            continue;
        }

        n_stroked ++;

        if (prev_ml != -1 && fabs(style->stroke_miterlimit.value - prev_ml) > 1e-3) {
            same_ml = false;
        }
        prev_ml = style->stroke_miterlimit.value;

        avgml += style->stroke_miterlimit.value;
    }

    if (n_stroked > 1) {
        avgml /= (n_stroked);
    }

    style_res->stroke_miterlimit.value = avgml;
    style_res->stroke_miterlimit.set = true;

    if (n_stroked == 0) {
        return QUERY_STYLE_NOTHING;
    } else if (n_stroked == 1) {
        return QUERY_STYLE_SINGLE;
    } else {
        if (same_ml)
            return QUERY_STYLE_MULTIPLE_SAME;
        else
            return QUERY_STYLE_MULTIPLE_AVERAGED;
    }
}

/**
 * Write to style_res the stroke cap of a list of objects.
 */
int
objects_query_strokecap (const std::vector<SPItem*> &objects, SPStyle *style_res)
{
    if (objects.empty()) {
        /* No objects, set empty */
        return QUERY_STYLE_NOTHING;
    }

    int cap = -1;
    gdouble prev_cap = -1;
    bool same_cap = true;
    int n_stroked = 0;

    for (std::vector<SPItem*>::const_iterator i = objects.begin(); i != objects.end(); i++) {
        SPObject *obj = *i;
        if (!dynamic_cast<SPItem *>(obj)) {
            continue;
        }
        SPStyle *style = obj->style;
        if (!style) {
            continue;
        }

        if ( style->stroke.isNone() ) {
            continue;
        }

        n_stroked ++;

        if (prev_cap != -1 && style->stroke_linecap.value != prev_cap)
            same_cap = false;
        prev_cap = style->stroke_linecap.value;

        cap = style->stroke_linecap.value;
    }

    style_res->stroke_linecap.value = cap;
    style_res->stroke_linecap.set = true;

    if (n_stroked == 0) {
        return QUERY_STYLE_NOTHING;
    } else if (n_stroked == 1) {
        return QUERY_STYLE_SINGLE;
    } else {
        if (same_cap)
            return QUERY_STYLE_MULTIPLE_SAME;
        else
            return QUERY_STYLE_MULTIPLE_DIFFERENT;
    }
}

/**
 * Write to style_res the stroke join of a list of objects.
 */
int
objects_query_strokejoin (const std::vector<SPItem*> &objects, SPStyle *style_res)
{
    if (objects.empty()) {
        /* No objects, set empty */
        return QUERY_STYLE_NOTHING;
    }

    int join = -1;
    gdouble prev_join = -1;
    bool same_join = true;
    int n_stroked = 0;

    for (std::vector<SPItem*>::const_iterator i = objects.begin(); i != objects.end(); i++) {
        SPObject *obj = *i;
        if (!dynamic_cast<SPItem *>(obj)) {
            continue;
        }
        SPStyle *style = obj->style;
        if (!style) {
            continue;
        }

        if ( style->stroke.isNone() ) {
            continue;
        }

        n_stroked ++;

        if (prev_join != -1 && style->stroke_linejoin.value != prev_join) {
            same_join = false;
        }
        prev_join = style->stroke_linejoin.value;

        join = style->stroke_linejoin.value;
    }

    style_res->stroke_linejoin.value = join;
    style_res->stroke_linejoin.set = true;

    if (n_stroked == 0) {
        return QUERY_STYLE_NOTHING;
    } else if (n_stroked == 1) {
        return QUERY_STYLE_SINGLE;
    } else {
        if (same_join)
            return QUERY_STYLE_MULTIPLE_SAME;
        else
            return QUERY_STYLE_MULTIPLE_DIFFERENT;
    }
}

/**
 * Write to style_res the average font size and spacing of objects.
 */
int
objects_query_fontnumbers (const std::vector<SPItem*> &objects, SPStyle *style_res)
{
    bool different = false;

    double size = 0;
    double letterspacing = 0;
    double wordspacing = 0;
    double linespacing = 0;
    bool letterspacing_normal = false;
    bool wordspacing_normal = false;
    bool linespacing_normal = false;

    double size_prev = 0;
    double letterspacing_prev = 0;
    double wordspacing_prev = 0;
    double linespacing_prev = 0;

    int texts = 0;
    int no_size = 0;

    for (std::vector<SPItem*>::const_iterator i = objects.begin(); i != objects.end(); i++) {
        SPObject *obj = *i;

        if (!isTextualItem(obj)) {
            continue;
        }

        SPStyle *style = obj->style;
        if (!style) {
            continue;
        }

        texts ++;
        SPItem *item = dynamic_cast<SPItem *>(obj);
        g_assert(item != NULL);
        double dummy = style->font_size.computed * Geom::Affine(item->i2dt_affine()).descrim();
        if (!IS_NAN(dummy)) {
            size += dummy; /// \todo FIXME: we assume non-% units here
        } else {
            no_size++;
        }

        if (style->letter_spacing.normal) {
            if (!different && (letterspacing_prev == 0 || letterspacing_prev == letterspacing)) {
                letterspacing_normal = true;
            }
        } else {
            letterspacing += style->letter_spacing.computed * Geom::Affine(item->i2dt_affine()).descrim(); /// \todo FIXME: we assume non-% units here
            letterspacing_normal = false;
        }

        if (style->word_spacing.normal) {
            if (!different && (wordspacing_prev == 0 || wordspacing_prev == wordspacing)) {
                wordspacing_normal = true;
            }
        } else {
            wordspacing += style->word_spacing.computed * Geom::Affine(item->i2dt_affine()).descrim(); /// \todo FIXME: we assume non-% units here
            wordspacing_normal = false;
        }

        double linespacing_current;
        if (style->line_height.normal) {
            linespacing_current = Inkscape::Text::Layout::LINE_HEIGHT_NORMAL;
            if (!different && (linespacing_prev == 0 || linespacing_prev == linespacing_current))
                linespacing_normal = true;
        } else if (style->line_height.unit == SP_CSS_UNIT_PERCENT || style->font_size.computed == 0) {
            linespacing_current = style->line_height.value;
            linespacing_normal = false;
        } else {
            linespacing_current = style->line_height.computed;
            linespacing_normal = false;
        }
        linespacing += linespacing_current;

        if ((size_prev != 0 && style->font_size.computed != size_prev) ||
            (letterspacing_prev != 0 && style->letter_spacing.computed != letterspacing_prev) ||
            (wordspacing_prev != 0 && style->word_spacing.computed != wordspacing_prev) ||
            (linespacing_prev != 0 && linespacing_current != linespacing_prev)) {
            different = true;
        }

        size_prev = style->font_size.computed;
        letterspacing_prev = style->letter_spacing.computed;
        wordspacing_prev = style->word_spacing.computed;
        linespacing_prev = linespacing_current;

        // FIXME: we must detect MULTIPLE_DIFFERENT for these too
        style_res->text_anchor.computed = style->text_anchor.computed;
        style_res->writing_mode.computed = style->writing_mode.computed;
    }

    if (texts == 0)
        return QUERY_STYLE_NOTHING;

    if (texts > 1) {
        if (texts - no_size > 0) {
            size /= (texts - no_size);
        }
        letterspacing /= texts;
        wordspacing /= texts;
        linespacing /= texts;
    }

    style_res->font_size.computed = size;
    style_res->font_size.type = SP_FONT_SIZE_LENGTH;

    style_res->letter_spacing.normal = letterspacing_normal;
    style_res->letter_spacing.computed = letterspacing;

    style_res->word_spacing.normal = wordspacing_normal;
    style_res->word_spacing.computed = wordspacing;

    style_res->line_height.normal = linespacing_normal;
    style_res->line_height.computed = linespacing;
    style_res->line_height.value = linespacing;
    style_res->line_height.unit = SP_CSS_UNIT_PERCENT;

    if (texts > 1) {
        if (different) {
            return QUERY_STYLE_MULTIPLE_AVERAGED;
        } else {
            return QUERY_STYLE_MULTIPLE_SAME;
        }
    } else {
        return QUERY_STYLE_SINGLE;
    }
}

/**
 * Write to style_res the average font style of objects.
 */
int
objects_query_fontstyle (const std::vector<SPItem*> &objects, SPStyle *style_res)
{
    bool different = false;
    bool set = false;

    int texts = 0;

    for (std::vector<SPItem*>::const_iterator i = objects.begin(); i != objects.end(); i++) {
        SPObject *obj = *i;

        if (!isTextualItem(obj)) {
            continue;
        }

        SPStyle *style = obj->style;
        if (!style) {
            continue;
        }

        texts ++;

        if (set &&
            ( ( style_res->font_weight.computed  != style->font_weight.computed  ) ||
              ( style_res->font_style.computed   != style->font_style.computed   ) ||
              ( style_res->font_stretch.computed != style->font_stretch.computed ) ||
              ( style_res->font_variant.computed != style->font_variant.computed ) ) ) {
            different = true;  // different styles
        }

        set = TRUE;
        style_res->font_weight.value = style_res->font_weight.computed = style->font_weight.computed;
        style_res->font_style.value = style_res->font_style.computed = style->font_style.computed;
        style_res->font_stretch.value = style_res->font_stretch.computed = style->font_stretch.computed;
        style_res->font_variant.value = style_res->font_variant.computed = style->font_variant.computed;
        style_res->text_align.value = style_res->text_align.computed = style->text_align.computed;
        style_res->font_size.value = style->font_size.value;
        style_res->font_size.unit = style->font_size.unit;
    }

    if (texts == 0 || !set)
        return QUERY_STYLE_NOTHING;

    if (texts > 1) {
        if (different) {
            return QUERY_STYLE_MULTIPLE_DIFFERENT;
        } else {
            return QUERY_STYLE_MULTIPLE_SAME;
        }
    } else {
        return QUERY_STYLE_SINGLE;
    }
}

int
objects_query_fontvariants (const std::vector<SPItem*> &objects, SPStyle *style_res)
{
    bool set = false;

    int texts = 0;

    SPILigatures* ligatures_res = &(style_res->font_variant_ligatures);
    SPIEnum* position_res       = &(style_res->font_variant_position);
    SPIEnum* caps_res           = &(style_res->font_variant_caps);
    SPINumeric* numeric_res     = &(style_res->font_variant_numeric);

    // Stores 'and' of all values
    ligatures_res->computed = SP_CSS_FONT_VARIANT_LIGATURES_NORMAL;
    position_res->computed  = SP_CSS_FONT_VARIANT_POSITION_NORMAL;
    caps_res->computed      = SP_CSS_FONT_VARIANT_CAPS_NORMAL;
    numeric_res->computed   = SP_CSS_FONT_VARIANT_NUMERIC_NORMAL;

    // Stores only differences
    ligatures_res->value = 0;
    position_res->value  = 0;
    caps_res->value      = 0;
    numeric_res->value   = 0;
    
    for (std::vector<SPItem*>::const_iterator i = objects.begin(); i != objects.end(); i++) {
        SPObject *obj = *i;

        if (!isTextualItem(obj)) {
            continue;
        }

        SPStyle *style = obj->style;
        if (!style) {
            continue;
        }

        texts ++;

        SPILigatures* ligatures_in = &(style->font_variant_ligatures);
        SPIEnum*      position_in  = &(style->font_variant_position);
        SPIEnum*      caps_in      = &(style->font_variant_caps);
        SPINumeric*   numeric_in   = &(style->font_variant_numeric);
        // computed stores which bits are on/off, only valid if same between all selected objects.
        // value stores which bits are different between objects. This is a bit of an abuse of
        // the values but then we don't need to add new variables to class.
        if (set) {
            ligatures_res->value  |= (ligatures_res->computed ^ ligatures_in->computed );
            ligatures_res->computed &= ligatures_in->computed;

            position_res->value  |= (position_res->computed ^ position_in->computed );
            position_res->computed &= position_in->computed;

            caps_res->value  |= (caps_res->computed ^ caps_in->computed );
            caps_res->computed &= caps_in->computed;

            numeric_res->value  |= (numeric_res->computed ^ numeric_in->computed );
            numeric_res->computed &= numeric_in->computed;

        } else {
            ligatures_res->computed  = ligatures_in->computed;
            position_res->computed   = position_in->computed;
            caps_res->computed       = caps_in->computed;
            numeric_res->computed    = numeric_in->computed;
        }

        set = true;
    }

    bool different = (style_res->font_variant_ligatures.value != 0 || 
                      style_res->font_variant_position.value  != 0 ||
                      style_res->font_variant_caps.value      != 0 ||
                      style_res->font_variant_numeric.value   != 0 );

    if (texts == 0 || !set)
        return QUERY_STYLE_NOTHING;

    if (texts > 1) {
        if (different) {
            return QUERY_STYLE_MULTIPLE_DIFFERENT;
        } else {
            return QUERY_STYLE_MULTIPLE_SAME;
        }
    } else {
        return QUERY_STYLE_SINGLE;
    }
}


int
objects_query_fontfeaturesettings (const std::vector<SPItem*> &objects, SPStyle *style_res)
{
    bool different = false;
    int texts = 0;

    if (style_res->font_feature_settings.value) {
        g_free(style_res->font_feature_settings.value);
        style_res->font_feature_settings.value = NULL;
    }
    style_res->font_feature_settings.set = FALSE;

    for (std::vector<SPItem*>::const_iterator i = objects.begin(); i != objects.end(); i++) {
        SPObject *obj = *i;

        // std::cout << "  " << reinterpret_cast<SPObject*>(i->data)->getId() << std::endl;
        if (!isTextualItem(obj)) {
            continue;
        }

        SPStyle *style = obj->style;
        if (!style) {
            continue;
        }

        texts ++;

        if (style_res->font_feature_settings.value && style->font_feature_settings.value &&
            strcmp (style_res->font_feature_settings.value, style->font_feature_settings.value)) {
            different = true;  // different fonts
        }

        if (style_res->font_feature_settings.value) {
            g_free(style_res->font_feature_settings.value);
            style_res->font_feature_settings.value = NULL;
        }

        style_res->font_feature_settings.set = TRUE;
        style_res->font_feature_settings.value = g_strdup(style->font_feature_settings.value);
    }

    if (texts == 0 || !style_res->font_feature_settings.set) {
        return QUERY_STYLE_NOTHING;
    }

    if (texts > 1) {
        if (different) {
            return QUERY_STYLE_MULTIPLE_DIFFERENT;
        } else {
            return QUERY_STYLE_MULTIPLE_SAME;
        }
    } else {
        return QUERY_STYLE_SINGLE;
    }
}


/**
 * Write to style_res the baseline numbers.
 */
static int
objects_query_baselines (const std::vector<SPItem*> &objects, SPStyle *style_res)
{
    bool different = false;

    // Only baseline-shift at the moment
    // We will return:
    //   If baseline-shift is same for all objects:
    //     The full baseline-shift data (used for subscripts and superscripts)
    //   If baseline-shift is different:
    //     The average baseline-shift (not implemented at the moment as this is complicated June 2010)
    SPIBaselineShift old;
    old.value = 0.0;
    old.computed = 0.0;

    // double baselineshift = 0.0;
    bool set = false;

    int texts = 0;

    for (std::vector<SPItem*>::const_iterator i = objects.begin(); i != objects.end(); i++) {
        SPObject *obj = *i;

        if (!isTextualItem(obj)) {
            continue;
        }

        SPStyle *style = obj->style;
        if (!style) {
            continue;
        }

        texts ++;

        SPIBaselineShift current;
        if(style->baseline_shift.set) {

            current.set      = style->baseline_shift.set;
            current.inherit  = style->baseline_shift.inherit;
            current.type     = style->baseline_shift.type;
            current.literal  = style->baseline_shift.literal;
            current.value    = style->baseline_shift.value;
            current.computed = style->baseline_shift.computed;

            if( set ) {
                if( current.set      != old.set ||
                    current.inherit  != old.inherit ||
                    current.type     != old.type ||
                    current.literal  != old.literal ||
                    current.value    != old.value ||
                    current.computed != old.computed ) {
                    // Maybe this needs to be better thought out.
                    different = true;
                }
            }

            set = true;

            old.set      = current.set;
            old.inherit  = current.inherit;
            old.type     = current.type;
            old.literal  = current.literal;
            old.value    = current.value;
            old.computed = current.computed;
        }
    }

    if (different || !set ) {
        style_res->baseline_shift.set = false;
        style_res->baseline_shift.computed = 0.0;
    } else {
        style_res->baseline_shift.set      = old.set;
        style_res->baseline_shift.inherit  = old.inherit;
        style_res->baseline_shift.type     = old.type;
        style_res->baseline_shift.literal  = old.literal;
        style_res->baseline_shift.value    = old.value;
        style_res->baseline_shift.computed = old.computed;
    }

    if (texts == 0 || !set)
        return QUERY_STYLE_NOTHING;

    if (texts > 1) {
        if (different) {
            return QUERY_STYLE_MULTIPLE_DIFFERENT;
        } else {
            return QUERY_STYLE_MULTIPLE_SAME;
        }
    } else {
        return QUERY_STYLE_SINGLE;
    }
}

/**
 * Write to style_res the average font family of objects.
 */
int
objects_query_fontfamily (const std::vector<SPItem*> &objects, SPStyle *style_res)
{
    bool different = false;
    int texts = 0;

    if (style_res->font_family.value) {
        g_free(style_res->font_family.value);
        style_res->font_family.value = NULL;
    }
    style_res->font_family.set = FALSE;

    for (std::vector<SPItem*>::const_iterator i = objects.begin(); i != objects.end(); i++) {
        SPObject *obj = *i;

        // std::cout << "  " << reinterpret_cast<SPObject*>(i->data)->getId() << std::endl;
        if (!isTextualItem(obj)) {
            continue;
        }

        SPStyle *style = obj->style;
        if (!style) {
            continue;
        }

        texts ++;

        if (style_res->font_family.value && style->font_family.value &&
            strcmp (style_res->font_family.value, style->font_family.value)) {
            different = true;  // different fonts
        }

        if (style_res->font_family.value) {
            g_free(style_res->font_family.value);
            style_res->font_family.value = NULL;
        }

        style_res->font_family.set = TRUE;
        style_res->font_family.value = g_strdup(style->font_family.value);
    }

    if (texts == 0 || !style_res->font_family.set) {
        return QUERY_STYLE_NOTHING;
    }

    if (texts > 1) {
        if (different) {
            return QUERY_STYLE_MULTIPLE_DIFFERENT;
        } else {
            return QUERY_STYLE_MULTIPLE_SAME;
        }
    } else {
        return QUERY_STYLE_SINGLE;
    }
}

static int
objects_query_fontspecification (const std::vector<SPItem*> &objects, SPStyle *style_res)
{
    bool different = false;
    int texts = 0;

    if (style_res->font_specification.value) {
        g_free(style_res->font_specification.value);
        style_res->font_specification.value = NULL;
    }
    style_res->font_specification.set = FALSE;

    for (std::vector<SPItem*>::const_iterator i = objects.begin(); i != objects.end(); i++) {
        SPObject *obj = *i;

        // std::cout << "  " << reinterpret_cast<SPObject*>(i->data)->getId() << std::endl;
        if (!isTextualItem(obj)) {
            continue;
        }

        SPStyle *style = obj->style;
        if (!style) {
            continue;
        }

        texts ++;

        if (style_res->font_specification.value && style_res->font_specification.set &&
            style->font_specification.value && style->font_specification.set &&
            strcmp (style_res->font_specification.value, style->font_specification.value)) {
            different = true;  // different fonts
        }

        if (style->font_specification.set) {

            if (style_res->font_specification.value) {
                g_free(style_res->font_specification.value);
                style_res->font_specification.value = NULL;
            }

            style_res->font_specification.set = TRUE;
            style_res->font_specification.value = g_strdup(style->font_specification.value);
        }
    }

    if (texts == 0) {
        return QUERY_STYLE_NOTHING;
    }

    if (texts > 1) {
        if (different) {
            return QUERY_STYLE_MULTIPLE_DIFFERENT;
        } else {
            return QUERY_STYLE_MULTIPLE_SAME;
        }
    } else {
        return QUERY_STYLE_SINGLE;
    }
}

static int
objects_query_blend (const std::vector<SPItem*> &objects, SPStyle *style_res)
{
    const int empty_prev = -2;
    const int complex_filter = 5;
    int blend = 0;
    float blend_prev = empty_prev;
    bool same_blend = true;
    guint items = 0;

    for (std::vector<SPItem*>::const_iterator i = objects.begin(); i != objects.end(); i++) {
        SPObject *obj = *i;
        if (!obj) {
            continue;
        }
        SPStyle *style = obj->style;
        if (!style || !dynamic_cast<SPItem *>(obj)) {
            continue;
        }

        items++;

        //if object has a filter
        if (style->filter.set && style->getFilter()) {
            int blurcount = 0;
            int blendcount = 0;

            // determine whether filter is simple (blend and/or blur) or complex
            for(SPObject *primitive_obj = style->getFilter()->children;
                primitive_obj && dynamic_cast<SPFilterPrimitive *>(primitive_obj);
                primitive_obj = primitive_obj->next) {
                SPFilterPrimitive *primitive = dynamic_cast<SPFilterPrimitive *>(primitive_obj);
                if (dynamic_cast<SPFeBlend *>(primitive)) {
                    ++blendcount;
                } else if (dynamic_cast<SPGaussianBlur *>(primitive)) {
                    ++blurcount;
                } else {
                    blurcount = complex_filter;
                    break;
                }
            }

            // simple filter
            if(blurcount == 1 || blendcount == 1) {
                for(SPObject *primitive_obj = style->getFilter()->children;
                    primitive_obj && dynamic_cast<SPFilterPrimitive *>(primitive_obj);
                    primitive_obj = primitive_obj->next) {
                    SPFeBlend *spblend = dynamic_cast<SPFeBlend *>(primitive_obj);
                    if (spblend) {
                        blend = spblend->blend_mode;
                    }
                }
            }
            else {
                blend = complex_filter;
            }
        }
        // defaults to blend mode = "normal"
        else {
            blend = 0;
        }

        if(blend_prev != empty_prev && blend_prev != blend)
            same_blend = false;
        blend_prev = blend;
    }

    if (items > 0) {
        style_res->filter_blend_mode.value = blend;
    }

    if (items == 0) {
        return QUERY_STYLE_NOTHING;
    } else if (items == 1) {
        return QUERY_STYLE_SINGLE;
    } else {
        if(same_blend)
            return QUERY_STYLE_MULTIPLE_SAME;
        else
            return QUERY_STYLE_MULTIPLE_DIFFERENT;
    }
}

/**
 * Write to style_res the average blurring of a list of objects.
 */
int
objects_query_blur (const std::vector<SPItem*> &objects, SPStyle *style_res)
{
   if (objects.empty()) {
        /* No objects, set empty */
        return QUERY_STYLE_NOTHING;
    }

    float blur_sum = 0;
    float blur_prev = -1;
    bool same_blur = true;
    guint blur_items = 0;
    guint items = 0;

    for (std::vector<SPItem*>::const_iterator i = objects.begin(); i != objects.end(); i++) {
        SPObject *obj = *i;
        if (!obj) {
            continue;
        }
        SPStyle *style = obj->style;
        if (!style) {
            continue;
        }
        SPItem *item = dynamic_cast<SPItem *>(obj);
        if (!item) {
            continue;
        }

        Geom::Affine i2d = item->i2dt_affine();

        items ++;

        //if object has a filter
        if (style->filter.set && style->getFilter()) {
            //cycle through filter primitives
            SPObject *primitive_obj = style->getFilter()->children;
            while (primitive_obj) {
                SPFilterPrimitive *primitive = dynamic_cast<SPFilterPrimitive *>(primitive_obj);
                if (primitive) {

                    //if primitive is gaussianblur
                    SPGaussianBlur * spblur = dynamic_cast<SPGaussianBlur *>(primitive);
                    if (spblur) {
                        float num = spblur->stdDeviation.getNumber();
                        float dummy = num * i2d.descrim();
                        if (!IS_NAN(dummy)) {
                            blur_sum += dummy;
                            if (blur_prev != -1 && fabs (num - blur_prev) > 1e-2) // rather low tolerance because difference in blur radii is much harder to notice than e.g. difference in sizes
                                same_blur = false;
                            blur_prev = num;
                            //TODO: deal with opt number, for the moment it's not necessary to the ui.
                            blur_items ++;
                        }
                    }
                }
                primitive_obj = primitive_obj->next;
            }
        }
    }

    if (items > 0) {
        if (blur_items > 0)
            blur_sum /= blur_items;
        style_res->filter_gaussianBlur_deviation.value = blur_sum;
    }

    if (items == 0) {
        return QUERY_STYLE_NOTHING;
    } else if (items == 1) {
        return QUERY_STYLE_SINGLE;
    } else {
        if (same_blur)
            return QUERY_STYLE_MULTIPLE_SAME;
        else
            return QUERY_STYLE_MULTIPLE_AVERAGED;
    }
}

/**
 * Query the given list of objects for the given property, write
 * the result to style, return appropriate flag.
 */
int
sp_desktop_query_style_from_list (const std::vector<SPItem*> &list, SPStyle *style, int property)
{
    if (property == QUERY_STYLE_PROPERTY_FILL) {
        return objects_query_fillstroke (list, style, true);
    } else if (property == QUERY_STYLE_PROPERTY_STROKE) {
        return objects_query_fillstroke (list, style, false);

    } else if (property == QUERY_STYLE_PROPERTY_STROKEWIDTH) {
        return objects_query_strokewidth (list, style);
    } else if (property == QUERY_STYLE_PROPERTY_STROKEMITERLIMIT) {
        return objects_query_miterlimit (list, style);
    } else if (property == QUERY_STYLE_PROPERTY_STROKECAP) {
        return objects_query_strokecap (list, style);
    } else if (property == QUERY_STYLE_PROPERTY_STROKEJOIN) {
        return objects_query_strokejoin (list, style);

    } else if (property == QUERY_STYLE_PROPERTY_MASTEROPACITY) {
        return objects_query_opacity (list, style);

    } else if (property == QUERY_STYLE_PROPERTY_FONT_SPECIFICATION) {
        return objects_query_fontspecification (list, style);
    } else if (property == QUERY_STYLE_PROPERTY_FONTFAMILY) {
        return objects_query_fontfamily (list, style);
    } else if (property == QUERY_STYLE_PROPERTY_FONTSTYLE) {
        return objects_query_fontstyle (list, style);
    } else if (property == QUERY_STYLE_PROPERTY_FONTVARIANTS) {
        return objects_query_fontvariants (list, style);
    } else if (property == QUERY_STYLE_PROPERTY_FONTFEATURESETTINGS) {
        return objects_query_fontfeaturesettings (list, style);
    } else if (property == QUERY_STYLE_PROPERTY_FONTNUMBERS) {
        return objects_query_fontnumbers (list, style);
    } else if (property == QUERY_STYLE_PROPERTY_BASELINES) {
        return objects_query_baselines (list, style);

    } else if (property == QUERY_STYLE_PROPERTY_BLEND) {
        return objects_query_blend (list, style);
    } else if (property == QUERY_STYLE_PROPERTY_BLUR) {
        return objects_query_blur (list, style);
    }
    return QUERY_STYLE_NOTHING;
}


/**
 * Query the subselection (if any) or selection on the given desktop for the given property, write
 * the result to style, return appropriate flag.
 */
int
sp_desktop_query_style(SPDesktop *desktop, SPStyle *style, int property)
{
    // Used by text tool and in gradient dragging
    int ret = desktop->_query_style_signal.emit(style, property);

    if (ret != QUERY_STYLE_NOTHING)
        return ret; // subselection returned a style, pass it on

    // otherwise, do querying and averaging over selection
    if (desktop->selection != NULL) {
        return sp_desktop_query_style_from_list (desktop->selection->itemList(), style, property);
    }

    return QUERY_STYLE_NOTHING;
}

/**
 * Do the same as sp_desktop_query_style for all (defined) style properties, return true if at
 * least one of the properties did not return QUERY_STYLE_NOTHING.
 */
bool
sp_desktop_query_style_all (SPDesktop *desktop, SPStyle *query)
{
        int result_family = sp_desktop_query_style (desktop, query, QUERY_STYLE_PROPERTY_FONTFAMILY);
        int result_fstyle = sp_desktop_query_style (desktop, query, QUERY_STYLE_PROPERTY_FONTSTYLE);
        int result_fnumbers = sp_desktop_query_style (desktop, query, QUERY_STYLE_PROPERTY_FONTNUMBERS);
        int result_fill = sp_desktop_query_style (desktop, query, QUERY_STYLE_PROPERTY_FILL);
        int result_stroke = sp_desktop_query_style (desktop, query, QUERY_STYLE_PROPERTY_STROKE);
        int result_strokewidth = sp_desktop_query_style (desktop, query, QUERY_STYLE_PROPERTY_STROKEWIDTH);
        int result_strokemiterlimit = sp_desktop_query_style (desktop, query, QUERY_STYLE_PROPERTY_STROKEMITERLIMIT);
        int result_strokecap = sp_desktop_query_style (desktop, query, QUERY_STYLE_PROPERTY_STROKECAP);
        int result_strokejoin = sp_desktop_query_style (desktop, query, QUERY_STYLE_PROPERTY_STROKEJOIN);
        int result_opacity = sp_desktop_query_style (desktop, query, QUERY_STYLE_PROPERTY_MASTEROPACITY);
        int result_blur = sp_desktop_query_style (desktop, query, QUERY_STYLE_PROPERTY_BLUR);

        return (result_family != QUERY_STYLE_NOTHING ||
                result_fstyle != QUERY_STYLE_NOTHING ||
                result_fnumbers != QUERY_STYLE_NOTHING ||
                result_fill != QUERY_STYLE_NOTHING ||
                result_stroke != QUERY_STYLE_NOTHING ||
                result_opacity != QUERY_STYLE_NOTHING ||
                result_strokewidth != QUERY_STYLE_NOTHING ||
                result_strokemiterlimit != QUERY_STYLE_NOTHING ||
                result_strokecap != QUERY_STYLE_NOTHING ||
                result_strokejoin != QUERY_STYLE_NOTHING ||
                result_blur != QUERY_STYLE_NOTHING);
}


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
