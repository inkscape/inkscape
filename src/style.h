#ifndef SEEN_SP_STYLE_H
#define SEEN_SP_STYLE_H

/** \file
 * SPStyle - a style object for SPItem objects
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2010 Jon A. Cruz
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "style-enums.h"
#include "style-internal.h"

#include <stddef.h>
#include <sigc++/connection.h>

namespace Inkscape {
namespace XML {
class Node;
}
}

/// An SVG style object.
struct SPStyle {
    int refcount;

    /** Object we are attached to */
    SPObject *object;
    /** Document we are associated with */
    SPDocument *document;

    /** Our text style component */
    SPTextStyle *text;
    unsigned text_private : 1;

    /* CSS2 */
    /* Font */
    /** Size of the font */
    SPIFontSize font_size;
    /** Style of the font */
    SPIEnum font_style;
    /** Which substyle of the font */
    SPIEnum font_variant;
    /** Weight of the font */
    SPIEnum font_weight;
    /** Stretch of the font */
    SPIEnum font_stretch;

    /** First line indent of paragraphs (css2 16.1) */
    SPILength text_indent;
    /** text alignment (css2 16.2) (not to be confused with text-anchor) */
    SPIEnum text_align;
    /** text decoration (css2 16.3.1) is now handled as a subset of css3 2.4 */
    //    SPITextDecoration      text_decoration; 
    
    /** CSS 3 2.1, 2.2, 2.3 */
    /** Not done yet, test_decoration3        = css3 2.4*/
    SPITextDecorationLine  text_decoration_line;
    SPIPaint               text_decoration_color;
    SPITextDecorationStyle text_decoration_style;

    // used to implement text_decoration, not saved to or read from SVG file
    SPITextDecorationData  text_decoration_data;

    // 16.3.2 is text-shadow. That's complicated.
    /** Line spacing (css2 10.8.1) */
    SPILengthOrNormal line_height;
    /** letter spacing (css2 16.4) */
    SPILengthOrNormal letter_spacing;
    /** word spacing (also css2 16.4) */
    SPILengthOrNormal word_spacing;
    /** capitalization (css2 16.5) */
    SPIEnum text_transform;

    /* CSS3 Text */
    /** text direction (css3 text 3.2) */
    SPIEnum direction;
    /** block progression (css3 text 3.2) */
    SPIEnum block_progression;
    /** Writing mode (css3 text 3.2 and svg1.1 10.7.2) */
    SPIEnum writing_mode;
    /** Baseline shift (svg1.1 10.9.2) */
    SPIBaselineShift baseline_shift;

    /* SVG */
    /** Anchor of the text (svg1.1 10.9.1) */
    SPIEnum text_anchor;

    /* Misc attributes */
    unsigned clip_set : 1;
    unsigned color_set : 1;
    unsigned cursor_set : 1;
    unsigned overflow_set : 1;
    unsigned clip_path_set : 1;
    unsigned mask_set : 1;

    /** clip-rule: 0 nonzero, 1 evenodd */
    SPIEnum clip_rule;

    /** display */
    SPIEnum display;

    /** overflow */
    SPIEnum overflow;

    /** visibility */
    SPIEnum visibility;

    /** opacity */
    SPIScale24 opacity;

    /** mix-blend-mode:  CSS Compositing and Blending Level 1 */
    SPIEnum isolation;
    // Could be shared with Filter blending mode
    SPIEnum blend_mode;

    /** color */
    SPIPaint color;
    /** color-interpolation */
    SPIEnum color_interpolation;
    /** color-interpolation-filters */
    SPIEnum color_interpolation_filters;

    /** fill */
    SPIPaint fill;
    /** fill-opacity */
    SPIScale24 fill_opacity;
    /** fill-rule: 0 nonzero, 1 evenodd */
    SPIEnum fill_rule;

    /** stroke */
    SPIPaint stroke;
    /** stroke-width */
    SPILength stroke_width;
    /** stroke-linecap */
    SPIEnum stroke_linecap;
    /** stroke-linejoin */
    SPIEnum stroke_linejoin;
    /** stroke-miterlimit */
    SPIFloat stroke_miterlimit;
    /** stroke-dasharray */
    SPIDashArray stroke_dasharray;
    /** stroke-dashoffset */
    SPILength stroke_dashoffset;
    /** stroke-opacity */
    SPIScale24 stroke_opacity;

    /** Marker list */
    SPIString marker[SP_MARKER_LOC_QTY];

    SPIPaintOrder paint_order;

    /** Filter effect */
    SPIFilter filter;

    SPIEnum filter_blend_mode;

   /** normally not used, but duplicates the Gaussian blur deviation (if any) from the attached
        filter when the style is used for querying */
    SPILength filter_gaussianBlur_deviation;

    /** hints on how to render: e.g. speed vs. accuracy.
     * As of April, 2013, only image_rendering used. */
    SPIEnum color_rendering;
    SPIEnum image_rendering;
    SPIEnum shape_rendering;
    SPIEnum text_rendering;

    /** enable-background, used for defining where filter effects get
     * their background image */
    SPIEnum enable_background;

    /// style belongs to a cloned object
    bool cloned;

    sigc::connection release_connection;

    sigc::connection filter_modified_connection;
    sigc::connection fill_ps_modified_connection;
    sigc::connection stroke_ps_modified_connection;

    SPObject *getFilter() { return (filter.href) ? filter.href->getObject() : NULL; }
    SPObject const *getFilter() const { return (filter.href) ? filter.href->getObject() : NULL; }
    gchar const *getFilterURI() const { return (filter.href) ? filter.href->getURI()->toString() : NULL; }

    SPPaintServer *getFillPaintServer() { return (fill.value.href) ? fill.value.href->getObject() : NULL; }
    SPPaintServer const *getFillPaintServer() const { return (fill.value.href) ? fill.value.href->getObject() : NULL; }
    gchar const *getFillURI() const { return (fill.value.href) ? fill.value.href->getURI()->toString() : NULL; }

    SPPaintServer *getStrokePaintServer() { return (stroke.value.href) ? stroke.value.href->getObject() : NULL; }
    SPPaintServer const *getStrokePaintServer() const { return (stroke.value.href) ? stroke.value.href->getObject() : NULL; }
    gchar const  *getStrokeURI() const { return (stroke.value.href) ? stroke.value.href->getURI()->toString() : NULL; }
};

SPStyle *sp_style_new(SPDocument *document);

SPStyle *sp_style_new_from_object(SPObject *object);

SPStyle *sp_style_ref(SPStyle *style);

SPStyle *sp_style_unref(SPStyle *style);

void sp_style_read_from_object(SPStyle *style, SPObject *object);

void sp_style_read_from_prefs(SPStyle *style, Glib::ustring const &path);

void sp_style_merge_from_style_string(SPStyle *style, gchar const *p);

void sp_style_merge_from_parent(SPStyle *style, SPStyle const *parent);

void sp_style_merge_from_dying_parent(SPStyle *style, SPStyle const *parent);

gchar *sp_style_write_string(SPStyle const *style, guint flags = SP_STYLE_FLAG_IFSET);

gchar *sp_style_write_difference(SPStyle const *from, SPStyle const *to);

void sp_style_set_to_uri_string (SPStyle *style, bool isfill, const gchar *uri);

gchar const *sp_style_get_css_unit_string(int unit);
double sp_style_css_size_px_to_units(double size, int unit);
double sp_style_css_size_units_to_px(double size, int unit);


SPCSSAttr *sp_css_attr_from_style (SPStyle const *const style, guint flags);
SPCSSAttr *sp_css_attr_from_object(SPObject *object, guint flags = SP_STYLE_FLAG_IFSET);
SPCSSAttr *sp_css_attr_unset_text(SPCSSAttr *css);
SPCSSAttr *sp_css_attr_unset_uris(SPCSSAttr *css);
SPCSSAttr *sp_css_attr_scale(SPCSSAttr *css, double ex);

void sp_style_unset_property_attrs(SPObject *o);

void sp_style_set_property_url (SPObject *item, gchar const *property, SPObject *linked, bool recursive);

gchar *attribute_unquote(gchar const *val);
Glib::ustring css2_escape_quote(gchar const *val);

#endif // SEEN_SP_STYLE_H


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
