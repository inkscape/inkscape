/**
 * @file
 * SVG stylesheets implementation.
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Peter Moulder <pmoulder@mail.csse.monash.edu.au>
 *   bulia byak <buliabyak@users.sf.net>
 *   Abhishek Sharma
 *   Tavmjong Bah <tavmjong@free.fr>
 *   Kris De Gussem <Kris.DeGussem@gmail.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 * Copyright (C) 2005 Monash University
 * Copyright (C) 2012 Kris De Gussem
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <cstring>
#include <string>

#include "libcroco/cr-sel-eng.h"
#include "xml/croco-node-iface.h"

#include "svg/svg.h"
#include "svg/svg-color.h"
#include "svg/svg-icc-color.h"

#include "display/canvas-bpath.h"
#include "attributes.h"
#include "document.h"
#include "extract-uri.h"
#include "uri-references.h"
#include "uri.h"
#include "sp-paint-server.h"
#include "streq.h"
#include "strneq.h"
#include "style.h"
#include "svg/css-ostringstream.h"
#include "xml/repr.h"
#include "xml/simple-document.h"
#include "util/units.h"
#include "macros.h"
#include "preferences.h"

#include "sp-filter-reference.h"

#include <sigc++/functors/ptr_fun.h>
#include <sigc++/adaptors/bind.h>

#include <2geom/math-utils.h>

using Inkscape::CSSOStringStream;
using std::vector;

#define BMAX 8192

#define SP_CSS_FONT_SIZE_DEFAULT 12.0;

struct SPStyleEnum;

/*#########################
## FORWARD DECLARATIONS
#########################*/
static void sp_style_clear(SPStyle *style);

static void sp_style_merge_property(SPStyle *style, gint id, gchar const *val);

static void sp_style_merge_ipaint(SPStyle *style, SPIPaint *paint, SPIPaint const *parent);
static void sp_style_merge_ifilter(SPStyle *style, SPIFilter const *parent);
static void sp_style_read_dash(SPStyle *style, gchar const *str);

static SPTextStyle *sp_text_style_new(void);
static void sp_text_style_clear(SPTextStyle *ts);
static SPTextStyle *sp_text_style_unref(SPTextStyle *st);
static SPTextStyle *sp_text_style_duplicate_unset(SPTextStyle *st);
static guint sp_text_style_write(gchar *p, guint len, SPTextStyle const *st, guint flags = SP_STYLE_FLAG_IFSET);
static void sp_style_privatize_text(SPStyle *style);

static void sp_style_read_ifloat(SPIFloat *val, gchar const *str);
static void sp_style_read_iscale24(SPIScale24 *val, gchar const *str);
static void sp_style_read_ienum(SPIEnum *val, gchar const *str, SPStyleEnum const *dict, bool can_explicitly_inherit);
static void sp_style_read_istring(SPIString *val, gchar const *str);
static void sp_style_read_ilength(SPILength *val, gchar const *str);
static void sp_style_read_ilengthornormal(SPILengthOrNormal *val, gchar const *str);

static void sp_style_read_ipaintorder(SPIPaintOrder *val, gchar const *str);

static void sp_style_read_itextdecoration(SPITextDecorationLine *line, SPITextDecorationStyle *style, SPIPaint *color, gchar const *str);
static void sp_style_read_itextdecorationLine(SPITextDecorationLine *line, gchar const *str);
static void sp_style_read_itextdecorationStyle(SPITextDecorationStyle *style, gchar const *str);
static void sp_style_read_itextdecorationColor(SPIPaint *color, gchar const *str);

static void sp_style_read_icolor(SPIPaint *paint, gchar const *str, SPStyle *style, SPDocument *document);
static void sp_style_read_ifontsize(SPIFontSize *val, gchar const *str);
static void sp_style_read_ibaselineshift(SPIBaselineShift *val, gchar const *str);
static void sp_style_read_ifilter(gchar const *str, SPStyle *style, SPDocument *document);

static void sp_style_read_penum(SPIEnum *val, Inkscape::XML::Node *repr, gchar const *key, SPStyleEnum const *dict, bool can_explicitly_inherit);
static void sp_style_read_plength(SPILength *val, Inkscape::XML::Node *repr, gchar const *key);
static void sp_style_read_pfontsize(SPIFontSize *val, Inkscape::XML::Node *repr, gchar const *key);
static void sp_style_read_pbaselineshift(SPIBaselineShift *val, Inkscape::XML::Node *repr, gchar const *key);
static void sp_style_read_pfloat(SPIFloat *val, Inkscape::XML::Node *repr, gchar const *key);

static gint sp_style_write_ifloat(gchar *p, gint len, gchar const *key, SPIFloat const *val, SPIFloat const *base, guint flags);
static gint sp_style_write_iscale24(gchar *p, gint len, gchar const *key, SPIScale24 const *val, SPIScale24 const *base, guint flags);
static gint sp_style_write_ienum(gchar *p, gint len, gchar const *key, SPStyleEnum const *dict, SPIEnum const *val, SPIEnum const *base, guint flags);
static gint sp_style_write_istring(gchar *p, gint len, gchar const *key, SPIString const *val, SPIString const *base, guint flags);
static gint sp_style_write_ilength(gchar *p, gint len, gchar const *key, SPILength const *val, SPILength const *base, guint flags);
static gint sp_style_write_ipaint(gchar *b, gint len, gchar const *key, SPIPaint const *paint, SPIPaint const *base, guint flags);
static gint sp_style_write_ipaintorder(gchar *p, gint len, gchar const *key, SPIPaintOrder const *paint_order, SPIPaintOrder const *base, guint flags);
static gint sp_style_write_idasharray(gchar *p, gint const len, gchar const *const key, SPIDashArray const *const val, SPIDashArray const *const base, guint const flags);

static gint sp_style_write_ifontsize(gchar *p, gint len, gchar const *key, SPIFontSize const *val, SPIFontSize const *base, guint flags);
static gint sp_style_write_ibaselineshift(gchar *p, gint len, gchar const *key, SPIBaselineShift const *val, SPIBaselineShift const *base, guint flags);
static gint sp_style_write_ilengthornormal(gchar *p, gint const len, gchar const *const key, SPILengthOrNormal const *const val, SPILengthOrNormal const *const base, guint const flags);
static gint sp_style_write_itextdecoration(gchar *p, gint const len, gchar const *const key,
                               SPITextDecorationLine  const *const line,
                               SPITextDecorationStyle const *const style,
                               SPIPaint               const *const color,
                               SPITextDecorationLine  const *const baseLine,
                               SPITextDecorationStyle const *const baseStyle,
                               SPIPaint               const *const baseColor,
                               guint const flags);
static gint sp_style_write_ifilter(gchar *b, gint len, gchar const *key, SPIFilter const *filter, SPIFilter const *base, guint flags);

static void sp_style_filter_clear(SPStyle *style);

#define SPS_READ_IENUM_IF_UNSET(v,s,d,i) if (!(v)->set) {sp_style_read_ienum((v), (s), (d), (i));}
#define SPS_READ_PENUM_IF_UNSET(v,r,k,d,i) if (!(v)->set) {sp_style_read_penum((v), (r), (k), (d), (i));}

#define SPS_READ_ILENGTH_IF_UNSET(v,s) if (!(v)->set) {sp_style_read_ilength((v), (s));}
#define SPS_READ_PLENGTH_IF_UNSET(v,r,k) if (!(v)->set) {sp_style_read_plength((v), (r), (k));}

#define SPS_READ_PFLOAT_IF_UNSET(v,r,k) if (!(v)->set) {sp_style_read_pfloat((v), (r), (k));}

#define SPS_READ_IFONTSIZE_IF_UNSET(v,s) if (!(v)->set) {sp_style_read_ifontsize((v), (s));}
#define SPS_READ_PFONTSIZE_IF_UNSET(v,r,k) if (!(v)->set) {sp_style_read_pfontsize((v), (r), (k));}

#define SPS_READ_IBASELINE_SHIFT_IF_UNSET(v,s) if (!(v)->set) {sp_style_read_ibaselineshift((v), (s));}
#define SPS_READ_PBASELINE_SHIFT_IF_UNSET(v,r,k) if (!(v)->set) {sp_style_read_pbaselineshift((v), (r), (k));}

static void sp_style_merge_from_object_stylesheet(SPStyle *, SPObject const *);

struct SPStyleEnum {
    gchar const *key;
    gint value;
};

static SPStyleEnum const enum_fill_rule[] = {
    {"nonzero", SP_WIND_RULE_NONZERO},
    {"evenodd", SP_WIND_RULE_EVENODD},
    {NULL, -1}
};

static SPStyleEnum const enum_stroke_linecap[] = {
    {"butt", SP_STROKE_LINECAP_BUTT},
    {"round", SP_STROKE_LINECAP_ROUND},
    {"square", SP_STROKE_LINECAP_SQUARE},
    {NULL, -1}
};

static SPStyleEnum const enum_stroke_linejoin[] = {
    {"miter", SP_STROKE_LINEJOIN_MITER},
    {"round", SP_STROKE_LINEJOIN_ROUND},
    {"bevel", SP_STROKE_LINEJOIN_BEVEL},
    {NULL, -1}
};

static SPStyleEnum const enum_font_style[] = {
    {"normal", SP_CSS_FONT_STYLE_NORMAL},
    {"italic", SP_CSS_FONT_STYLE_ITALIC},
    {"oblique", SP_CSS_FONT_STYLE_OBLIQUE},
    {NULL, -1}
};

static SPStyleEnum const enum_font_size[] = {
    {"xx-small", SP_CSS_FONT_SIZE_XX_SMALL},
    {"x-small", SP_CSS_FONT_SIZE_X_SMALL},
    {"small", SP_CSS_FONT_SIZE_SMALL},
    {"medium", SP_CSS_FONT_SIZE_MEDIUM},
    {"large", SP_CSS_FONT_SIZE_LARGE},
    {"x-large", SP_CSS_FONT_SIZE_X_LARGE},
    {"xx-large", SP_CSS_FONT_SIZE_XX_LARGE},
    {"smaller", SP_CSS_FONT_SIZE_SMALLER},
    {"larger", SP_CSS_FONT_SIZE_LARGER},
    {NULL, -1}
};

static SPStyleEnum const enum_font_variant[] = {
    {"normal", SP_CSS_FONT_VARIANT_NORMAL},
    {"small-caps", SP_CSS_FONT_VARIANT_SMALL_CAPS},
    {NULL, -1}
};

static SPStyleEnum const enum_font_weight[] = {
    {"100", SP_CSS_FONT_WEIGHT_100},
    {"200", SP_CSS_FONT_WEIGHT_200},
    {"300", SP_CSS_FONT_WEIGHT_300},
    {"400", SP_CSS_FONT_WEIGHT_400},
    {"500", SP_CSS_FONT_WEIGHT_500},
    {"600", SP_CSS_FONT_WEIGHT_600},
    {"700", SP_CSS_FONT_WEIGHT_700},
    {"800", SP_CSS_FONT_WEIGHT_800},
    {"900", SP_CSS_FONT_WEIGHT_900},
    {"normal", SP_CSS_FONT_WEIGHT_NORMAL},
    {"bold", SP_CSS_FONT_WEIGHT_BOLD},
    {"lighter", SP_CSS_FONT_WEIGHT_LIGHTER},
    {"bolder", SP_CSS_FONT_WEIGHT_BOLDER},
    {NULL, -1}
};

static SPStyleEnum const enum_font_stretch[] = {
    {"ultra-condensed", SP_CSS_FONT_STRETCH_ULTRA_CONDENSED},
    {"extra-condensed", SP_CSS_FONT_STRETCH_EXTRA_CONDENSED},
    {"condensed", SP_CSS_FONT_STRETCH_CONDENSED},
    {"semi-condensed", SP_CSS_FONT_STRETCH_SEMI_CONDENSED},
    {"normal", SP_CSS_FONT_STRETCH_NORMAL},
    {"semi-expanded", SP_CSS_FONT_STRETCH_SEMI_EXPANDED},
    {"expanded", SP_CSS_FONT_STRETCH_EXPANDED},
    {"extra-expanded", SP_CSS_FONT_STRETCH_EXTRA_EXPANDED},
    {"ultra-expanded", SP_CSS_FONT_STRETCH_ULTRA_EXPANDED},
    {"narrower", SP_CSS_FONT_STRETCH_NARROWER},
    {"wider", SP_CSS_FONT_STRETCH_WIDER},
    {NULL, -1}
};

static SPStyleEnum const enum_text_align[] = {
    {"start", SP_CSS_TEXT_ALIGN_START},
    {"end", SP_CSS_TEXT_ALIGN_END},
    {"left", SP_CSS_TEXT_ALIGN_LEFT},
    {"right", SP_CSS_TEXT_ALIGN_RIGHT},
    {"center", SP_CSS_TEXT_ALIGN_CENTER},
    {"justify", SP_CSS_TEXT_ALIGN_JUSTIFY},
    {NULL, -1}
};

static SPStyleEnum const enum_text_transform[] = {
    {"capitalize", SP_CSS_TEXT_TRANSFORM_CAPITALIZE},
    {"uppercase", SP_CSS_TEXT_TRANSFORM_UPPERCASE},
    {"lowercase", SP_CSS_TEXT_TRANSFORM_LOWERCASE},
    {"none", SP_CSS_TEXT_TRANSFORM_NONE},
    {NULL, -1}
};

static SPStyleEnum const enum_text_anchor[] = {
    {"start", SP_CSS_TEXT_ANCHOR_START},
    {"middle", SP_CSS_TEXT_ANCHOR_MIDDLE},
    {"end", SP_CSS_TEXT_ANCHOR_END},
    {NULL, -1}
};

static SPStyleEnum const enum_direction[] = {
    {"ltr", SP_CSS_DIRECTION_LTR},
    {"rtl", SP_CSS_DIRECTION_RTL},
    {NULL, -1}
};

static SPStyleEnum const enum_block_progression[] = {
    {"tb", SP_CSS_BLOCK_PROGRESSION_TB},
    {"rl", SP_CSS_BLOCK_PROGRESSION_RL},
    {"lr", SP_CSS_BLOCK_PROGRESSION_LR},
    {NULL, -1}
};

static SPStyleEnum const enum_writing_mode[] = {
    /* Note that using the same enumerator for lr as lr-tb means we write as lr-tb even if the
     * input file said lr.  We prefer writing lr-tb on the grounds that the spec says the initial
     * value is lr-tb rather than lr.
     *
     * ECMA scripts may be surprised to find tb-rl in DOM if they set the attribute to rl, so
     * sharing enumerators for different strings may be a bug (once we support ecma script).
     */
    {"lr-tb", SP_CSS_WRITING_MODE_LR_TB},
    {"rl-tb", SP_CSS_WRITING_MODE_RL_TB},
    {"tb-rl", SP_CSS_WRITING_MODE_TB_RL},
    {"lr", SP_CSS_WRITING_MODE_LR_TB},
    {"rl", SP_CSS_WRITING_MODE_RL_TB},
    {"tb", SP_CSS_WRITING_MODE_TB_RL},
    {NULL, -1}
};

static SPStyleEnum const enum_baseline_shift[] = {
    {"baseline", SP_CSS_BASELINE_SHIFT_BASELINE},
    {"sub",      SP_CSS_BASELINE_SHIFT_SUB},
    {"super",    SP_CSS_BASELINE_SHIFT_SUPER},
    {NULL, -1}
};

static SPStyleEnum const enum_visibility[] = {
    {"hidden", SP_CSS_VISIBILITY_HIDDEN},
    {"collapse", SP_CSS_VISIBILITY_COLLAPSE},
    {"visible", SP_CSS_VISIBILITY_VISIBLE},
    {NULL, -1}
};

static SPStyleEnum const enum_overflow[] = {
    {"visible", SP_CSS_OVERFLOW_VISIBLE},
    {"hidden", SP_CSS_OVERFLOW_HIDDEN},
    {"scroll", SP_CSS_OVERFLOW_SCROLL},
    {"auto", SP_CSS_OVERFLOW_AUTO},
    {NULL, -1}
};

// CSS Compositing and Blending Level 1
static SPStyleEnum const enum_isolation[] = {
    {"auto",             SP_CSS_ISOLATION_AUTO},
    {"isolate",          SP_CSS_ISOLATION_ISOLATE},
    {NULL, -1}
};

static SPStyleEnum const enum_blend_mode[] = {
    {"normal",           SP_CSS_BLEND_NORMAL},
    {"multiply",         SP_CSS_BLEND_MULTIPLY},
    {"screen",           SP_CSS_BLEND_SCREEN},
    {"darken",           SP_CSS_BLEND_DARKEN},
    {"lighten",          SP_CSS_BLEND_LIGHTEN},
    {"overlay",          SP_CSS_BLEND_OVERLAY},
    {"color-dodge",      SP_CSS_BLEND_COLORDODGE},
    {"color-burn",       SP_CSS_BLEND_COLORBURN},
    {"hard-light",       SP_CSS_BLEND_HARDLIGHT},
    {"soft-light",       SP_CSS_BLEND_SOFTLIGHT},
    {"difference",       SP_CSS_BLEND_DIFFERENCE},
    {"exclusion",        SP_CSS_BLEND_EXCLUSION},
    {"hue",              SP_CSS_BLEND_HUE},
    {"saturation",       SP_CSS_BLEND_SATURATION},
    {"color",            SP_CSS_BLEND_COLOR},
    {"luminosity",       SP_CSS_BLEND_LUMINOSITY},
    {NULL, -1}
};

static SPStyleEnum const enum_display[] = {
    {"none",      SP_CSS_DISPLAY_NONE},
    {"inline",    SP_CSS_DISPLAY_INLINE},
    {"block",     SP_CSS_DISPLAY_BLOCK},
    {"list-item", SP_CSS_DISPLAY_LIST_ITEM},
    {"run-in",    SP_CSS_DISPLAY_RUN_IN},
    {"compact",   SP_CSS_DISPLAY_COMPACT},
    {"marker",    SP_CSS_DISPLAY_MARKER},
    {"table",     SP_CSS_DISPLAY_TABLE},
    {"inline-table",  SP_CSS_DISPLAY_INLINE_TABLE},
    {"table-row-group",    SP_CSS_DISPLAY_TABLE_ROW_GROUP},
    {"table-header-group", SP_CSS_DISPLAY_TABLE_HEADER_GROUP},
    {"table-footer-group", SP_CSS_DISPLAY_TABLE_FOOTER_GROUP},
    {"table-row",     SP_CSS_DISPLAY_TABLE_ROW},
    {"table-column-group", SP_CSS_DISPLAY_TABLE_COLUMN_GROUP},
    {"table-column",  SP_CSS_DISPLAY_TABLE_COLUMN},
    {"table-cell",    SP_CSS_DISPLAY_TABLE_CELL},
    {"table-caption", SP_CSS_DISPLAY_TABLE_CAPTION},
    {NULL, -1}
};

static SPStyleEnum const enum_shape_rendering[] = {
    {"auto",                SP_CSS_SHAPE_RENDERING_AUTO},
    {"optimizeSpeed",       SP_CSS_SHAPE_RENDERING_OPTIMIZESPEED},
    {"crispEdges",          SP_CSS_SHAPE_RENDERING_CRISPEDGES},
    {"geometricPrecision",  SP_CSS_SHAPE_RENDERING_GEOMETRICPRECISION},
    {NULL, -1}
};

static SPStyleEnum const enum_color_rendering[] = {
    {"auto",            SP_CSS_COLOR_RENDERING_AUTO},
    {"optimizeSpeed",   SP_CSS_COLOR_RENDERING_OPTIMIZESPEED},
    {"optimizeQuality", SP_CSS_COLOR_RENDERING_OPTIMIZEQUALITY},
    {NULL, -1}
};

static SPStyleEnum const enum_image_rendering[] = {
    {"auto",                  SP_CSS_IMAGE_RENDERING_AUTO},
    {"optimizeSpeed",         SP_CSS_IMAGE_RENDERING_OPTIMIZESPEED},
    {"optimizeQuality",       SP_CSS_IMAGE_RENDERING_OPTIMIZEQUALITY},
    {"-inkscape-crisp-edges", SP_CSS_IMAGE_RENDERING_CRISPEDGES},
    {"-inkscape-pixelated",   SP_CSS_IMAGE_RENDERING_PIXELATED},
    {NULL, -1}
};

static SPStyleEnum const enum_text_rendering[] = {
    {"auto",               SP_CSS_TEXT_RENDERING_AUTO},
    {"optimizeSpeed",      SP_CSS_TEXT_RENDERING_OPTIMIZESPEED},
    {"optimizeLegibility", SP_CSS_TEXT_RENDERING_OPTIMIZELEGIBILITY},
    {"geometricPrecision", SP_CSS_TEXT_RENDERING_GEOMETRICPRECISION},
    {NULL, -1}
};

static SPStyleEnum const enum_enable_background[] = {
    {"accumulate", SP_CSS_BACKGROUND_ACCUMULATE},
    {"new", SP_CSS_BACKGROUND_NEW},
    {NULL, -1}
};

static SPStyleEnum const enum_clip_rule[] = {
    {"nonzero", SP_WIND_RULE_NONZERO},
    {"evenodd", SP_WIND_RULE_EVENODD},
    {NULL, -1}
};

static SPStyleEnum const enum_color_interpolation[] = {
    {"auto", SP_CSS_COLOR_INTERPOLATION_AUTO},
    {"sRGB", SP_CSS_COLOR_INTERPOLATION_SRGB},
    {"linearRGB", SP_CSS_COLOR_INTERPOLATION_LINEARRGB},
    {NULL, -1}
};

/**
 * Release callback.
 */
static void
sp_style_object_release(SPObject *object, SPStyle *style)
{
    (void)object; // TODO
    style->object = NULL;
}

/**
 * Emit style modified signal on style's object if the filter changed.
 */
static void
sp_style_filter_ref_modified(SPObject *obj, guint flags, SPStyle *style)
{
    (void)flags; // TODO
    SPFilter *filter=static_cast<SPFilter *>(obj);
    if (style->getFilter() == filter)
    {
        if (style->object) {
            style->object->requestModified(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
        }
    }
}

/**
 * Gets called when the filter is (re)attached to the style
 */
static void
sp_style_filter_ref_changed(SPObject *old_ref, SPObject *ref, SPStyle *style)
{
    if (old_ref) {
        style->filter_modified_connection.disconnect();
    }
    if ( SP_IS_FILTER(ref))
    {
        style->filter_modified_connection =
           ref->connectModified(sigc::bind(sigc::ptr_fun(&sp_style_filter_ref_modified), style));
    }

    sp_style_filter_ref_modified(ref, 0, style);
}

/**
 * Emit style modified signal on style's object if server is style's fill
 * or stroke paint server.
 */
static void
sp_style_paint_server_ref_modified(SPObject *obj, guint flags, SPStyle *style)
{
    (void)flags; // TODO
    SPPaintServer *server = static_cast<SPPaintServer *>(obj);

    if ((style->fill.isPaintserver())
        && style->getFillPaintServer() == server)
    {
        if (style->object) {
            /** \todo
             * fixme: I do not know, whether it is optimal - we are
             * forcing reread of everything (Lauris)
             */
            /** \todo
             * fixme: We have to use object_modified flag, because parent
             * flag is only available downstreams.
             */
            style->object->requestModified(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
        }
    } else if ((style->stroke.isPaintserver())
        && style->getStrokePaintServer() == server)
    {
        if (style->object) {
            /// \todo fixme:
            style->object->requestModified(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
        }
    } else if (server) {
        g_assert_not_reached();
    }
}

/**
 * Gets called when the paintserver is (re)attached to the style
 */
static void
sp_style_fill_paint_server_ref_changed(SPObject *old_ref, SPObject *ref, SPStyle *style)
{
    if (old_ref) {
        style->fill_ps_modified_connection.disconnect();
    }
    if (SP_IS_PAINT_SERVER(ref)) {
        style->fill_ps_modified_connection =
           ref->connectModified(sigc::bind(sigc::ptr_fun(&sp_style_paint_server_ref_modified), style));
    }

    sp_style_paint_server_ref_modified(ref, 0, style);
}

/**
 * Gets called when the paintserver is (re)attached to the style
 */
static void
sp_style_stroke_paint_server_ref_changed(SPObject *old_ref, SPObject *ref, SPStyle *style)
{
    if (old_ref) {
        style->stroke_ps_modified_connection.disconnect();
    }
    if (SP_IS_PAINT_SERVER(ref)) {
        style->stroke_ps_modified_connection =
          ref->connectModified(sigc::bind(sigc::ptr_fun(&sp_style_paint_server_ref_modified), style));
    }

    sp_style_paint_server_ref_modified(ref, 0, style);
}

/**
 * Returns a new SPStyle object with settings as per sp_style_clear().
 */
SPStyle *
sp_style_new(SPDocument *document)
{
    SPStyle *const style = g_new0(SPStyle, 1);

    style->refcount = 1;
    style->object = NULL;
    style->document = document;
    style->text = sp_text_style_new();
    style->text_private = TRUE;

    sp_style_clear(style);

    style->cloned = false;

    new (&style->release_connection) sigc::connection();
    new (&style->filter_modified_connection) sigc::connection();
    new (&style->fill_ps_modified_connection) sigc::connection();
    new (&style->stroke_ps_modified_connection) sigc::connection();

    return style;
}


/**
 * Creates a new SPStyle object, and attaches it to the specified SPObject.
 */
SPStyle *
sp_style_new_from_object(SPObject *object)
{
    g_return_val_if_fail(object != NULL, NULL);
    g_return_val_if_fail(SP_IS_OBJECT(object), NULL);

    SPStyle *style = sp_style_new( object->document );
    style->object = object;
    style->release_connection = object->connectRelease(sigc::bind<1>(sigc::ptr_fun(&sp_style_object_release), style));

    if (object->cloned) {
        style->cloned = true;
    }

    return style;
}


/**
 * Increase refcount of style.
 */
SPStyle *
sp_style_ref(SPStyle *style)
{
    g_return_val_if_fail(style != NULL, NULL);
    g_return_val_if_fail(style->refcount > 0, NULL);

    style->refcount += 1;

    return style;
}


/**
 * Decrease refcount of style with possible destruction.
 */
SPStyle *
sp_style_unref(SPStyle *style)
{
    g_return_val_if_fail(style != NULL, NULL);
    g_return_val_if_fail(style->refcount > 0, NULL);

    style->refcount -= 1;

    if (style->refcount < 1) {
        style->release_connection.disconnect();
        style->release_connection.~connection();
        if (style->text) sp_text_style_unref(style->text);

         if (style->fill.value.href) {
             style->fill_ps_modified_connection.disconnect();
             delete style->fill.value.href;
             style->fill.value.href = NULL;
         }
         if (style->stroke.value.href) {
             style->stroke_ps_modified_connection.disconnect();
             delete style->stroke.value.href;
             style->stroke.value.href = NULL;
         }
         if (style->filter.href) {
             style->filter_modified_connection.disconnect();
             delete style->filter.href;
             style->filter.href = NULL;
         }

        style->filter_modified_connection.~connection();
        style->fill_ps_modified_connection.~connection();
        style->stroke_ps_modified_connection.~connection();

        style->fill.clear();
        style->stroke.clear();
        sp_style_filter_clear(style);

        style->stroke_dasharray.values.clear();

        for (unsigned i = SP_MARKER_LOC; i < SP_MARKER_LOC_QTY; i++) {
            if (style->marker[i].value) {
                g_free(style->marker[i].value);
                style->marker[i].value = NULL;
            }
        }

        g_free(style);
        return NULL;
    }
    return style;
}

/**
 *  Reads the various style parameters for an object from repr.
 */
static void
sp_style_read(SPStyle *style, SPObject *object, Inkscape::XML::Node *repr)
{
    g_assert(style != NULL);
    g_assert(repr != NULL);
    g_assert(!object || (object->getRepr() == repr));

    sp_style_clear(style);

    if (object && object->cloned) {
        style->cloned = true;
    }

    /* 1. Style attribute */
    gchar const *val = repr->attribute("style");
    if (val != NULL && *val) {
        sp_style_merge_from_style_string(style, val);
    }

    if (object) {
        sp_style_merge_from_object_stylesheet(style, object);
    } else {
        /** \todo No stylesheet information. Find out under what circumstances
         * this occurs, and handle accordingly.  (If we really wanted to, we
         * could probably get stylesheets by going through repr->doc.)
         */
    }

    /* 2. Presentation attributes */
    /* Attributes are only read in if not already set in a style sheet or style attribute above. */

    /* CSS2 */
    SPS_READ_PENUM_IF_UNSET(&style->visibility, repr, "visibility", enum_visibility, true);
    SPS_READ_PENUM_IF_UNSET(&style->display, repr, "display", enum_display, true);
    SPS_READ_PENUM_IF_UNSET(&style->overflow, repr, "overflow", enum_overflow, true);

    /* CSS Compositing and Blending Level 1 */
    SPS_READ_PENUM_IF_UNSET(&style->isolation,  repr, "isolation",      enum_isolation,  true);
    SPS_READ_PENUM_IF_UNSET(&style->blend_mode, repr, "mix_blend_mode", enum_blend_mode, true);

    /* Font */
    SPS_READ_PFONTSIZE_IF_UNSET(&style->font_size, repr, "font-size");
    SPS_READ_PENUM_IF_UNSET(&style->font_style, repr, "font-style", enum_font_style, true);
    SPS_READ_PENUM_IF_UNSET(&style->font_variant, repr, "font-variant", enum_font_variant, true);
    SPS_READ_PENUM_IF_UNSET(&style->font_weight, repr, "font-weight", enum_font_weight, true);
    SPS_READ_PENUM_IF_UNSET(&style->font_stretch, repr, "font-stretch", enum_font_stretch, true);
    /* Text (css2 chapter 16) */
    SPS_READ_PLENGTH_IF_UNSET(&style->text_indent, repr, "text-indent");
    SPS_READ_PENUM_IF_UNSET(&style->text_align, repr, "text-align", enum_text_align, true);
    if (!style->text_decoration_line.set) {
        // assume it uses either text-decoration or text-decoration-line, but not both
        if ((val = repr->attribute("text-decoration")) || (val = repr->attribute("text-decoration-line"))) {
            sp_style_read_itextdecoration(&style->text_decoration_line, &style->text_decoration_style, &style->text_decoration_color, val);
        }
    }
    if (!style->line_height.set) {
        val = repr->attribute("line-height");
        if (val) {
            sp_style_read_ilengthornormal(&style->line_height, val);
        }
    }
    if (!style->letter_spacing.set) {
        val = repr->attribute("letter-spacing");
        if (val) {
            sp_style_read_ilengthornormal(&style->letter_spacing, val);
        }
    }
    if (!style->word_spacing.set) {
        val = repr->attribute("word-spacing");
        if (val) {
            sp_style_read_ilengthornormal(&style->word_spacing, val);
        }
    }
    SPS_READ_PENUM_IF_UNSET(&style->text_transform, repr, "text-transform", enum_text_transform, true);
    SPS_READ_PENUM_IF_UNSET(&style->direction, repr, "direction", enum_direction, true);
    SPS_READ_PENUM_IF_UNSET(&style->block_progression, repr, "block_progression", enum_block_progression, true);

    /* SVG */
    SPS_READ_PENUM_IF_UNSET(&style->writing_mode, repr, "writing-mode",
                            enum_writing_mode, true);
    SPS_READ_PENUM_IF_UNSET(&style->text_anchor, repr, "text-anchor",
                            enum_text_anchor, true);
    SPS_READ_PBASELINE_SHIFT_IF_UNSET(&style->baseline_shift, repr, "baseline-shift");

    /* opacity */
    if (!style->opacity.set) {
        val = repr->attribute("opacity");
        if (val) {
            sp_style_read_iscale24(&style->opacity, val);
        }
    }
    /* color */
    if (!style->color.set) {
        val = repr->attribute("color");
        if (val) {
            sp_style_read_icolor(&style->color, val, style, ( object
                                                              ? object->document
                                                              : NULL ));
        }
    }
    /* color interpolation */
    SPS_READ_PENUM_IF_UNSET(&style->color_interpolation, repr, "color-interpolation", enum_color_interpolation, true);
    /* color interpolation filters*/
    SPS_READ_PENUM_IF_UNSET(&style->color_interpolation_filters, repr, "color-interpolation-filters", enum_color_interpolation, true);
    /* fill */
    if (!style->fill.set) {
        val = repr->attribute("fill");
        if (val) {
            style->fill.read( val, *style, (object) ? object->document : NULL );
        }
    }
    /* fill-opacity */
    if (!style->fill_opacity.set) {
        val = repr->attribute("fill-opacity");
        if (val) {
            sp_style_read_iscale24(&style->fill_opacity, val);
        }
    }
    /* fill-rule */
    SPS_READ_PENUM_IF_UNSET(&style->fill_rule, repr, "fill-rule", enum_fill_rule, true);
    /* stroke */
    if (!style->stroke.set) {
        val = repr->attribute("stroke");
        if (val) {
            style->stroke.read( val, *style, (object) ? object->document : NULL );
        }
    }
    SPS_READ_PLENGTH_IF_UNSET(&style->stroke_width, repr, "stroke-width");
    SPS_READ_PENUM_IF_UNSET(&style->stroke_linecap, repr, "stroke-linecap", enum_stroke_linecap, true);
    SPS_READ_PENUM_IF_UNSET(&style->stroke_linejoin, repr, "stroke-linejoin", enum_stroke_linejoin, true);
    SPS_READ_PFLOAT_IF_UNSET(&style->stroke_miterlimit, repr, "stroke-miterlimit");

    /* markers */
    if (!style->marker[SP_MARKER_LOC].set) {
        val = repr->attribute("marker");
        if (val) {
            sp_style_read_istring(&style->marker[SP_MARKER_LOC], val);
        }
    }
    if (!style->marker[SP_MARKER_LOC_START].set) {
        val = repr->attribute("marker-start");
        if (val) {
            sp_style_read_istring(&style->marker[SP_MARKER_LOC_START], val);
        }
    }
    if (!style->marker[SP_MARKER_LOC_MID].set) {
        val = repr->attribute("marker-mid");
        if (val) {
            sp_style_read_istring(&style->marker[SP_MARKER_LOC_MID], val);
        }
    }
    if (!style->marker[SP_MARKER_LOC_END].set) {
        val = repr->attribute("marker-end");
        if (val) {
            sp_style_read_istring(&style->marker[SP_MARKER_LOC_END], val);
        }
    }

    /* stroke-opacity */
    if (!style->stroke_opacity.set) {
        val = repr->attribute("stroke-opacity");
        if (val) {
            sp_style_read_iscale24(&style->stroke_opacity, val);
        }
    }
    if (!style->stroke_dasharray.set) {
        val = repr->attribute("stroke-dasharray");
        if (val) {
            sp_style_read_dash(style, val);
        }
    }
    SPS_READ_PLENGTH_IF_UNSET(&style->stroke_width, repr, "stroke-dashoffset");

    /* paint-order */
    if (!style->paint_order.set) {
        val = repr->attribute("paint-order");
        if (val) {
            sp_style_read_ipaintorder(&style->paint_order, val);
        } else {
            style->paint_order.layer[0] = SP_CSS_PAINT_ORDER_NORMAL;
        }
    }

    /* -inkscape-font-specification */
    if (!style->text_private || !style->text->font_specification.set) {
        val = repr->attribute("-inkscape-font-specification");
        if (val) {
            if (!style->text_private) sp_style_privatize_text(style);
            gchar *val_unquoted = attribute_unquote(val);
            sp_style_read_istring(&style->text->font_specification, val_unquoted);
            if (val_unquoted) g_free (val_unquoted);
        }
    }

    /* font-family */
    if (!style->text_private || !style->text->font_family.set) {
        val = repr->attribute("font-family");
        if (val) {
            if (!style->text_private) sp_style_privatize_text(style);
            gchar *val_unquoted = attribute_unquote(val);
            sp_style_read_istring(&style->text->font_family, val_unquoted);
            if (val_unquoted) g_free (val_unquoted);
        }
    }

    /* filter effects */
    if (!style->filter.set) {
        val = repr->attribute("filter");
        if (val) {
            sp_style_read_ifilter(val, style, (object) ? object->document : NULL);
        }
    }
    SPS_READ_PENUM_IF_UNSET(&style->enable_background, repr,
                            "enable-background", enum_enable_background, true);

    /* clip-rule */
    SPS_READ_PENUM_IF_UNSET(&style->clip_rule, repr, "clip-rule", enum_clip_rule, true);

    /* color_rendering, image_rendering, shape_rendering, text_rendering */
    SPS_READ_PENUM_IF_UNSET(&style->color_rendering, repr, "color-rendering", enum_color_rendering, true);
    SPS_READ_PENUM_IF_UNSET(&style->image_rendering, repr, "image-rendering", enum_image_rendering, true);
    SPS_READ_PENUM_IF_UNSET(&style->shape_rendering, repr, "shape-rendering", enum_shape_rendering, true);
    SPS_READ_PENUM_IF_UNSET(&style->text_rendering,  repr, "text-rendering",  enum_text_rendering,  true);

    /* 3. Merge from parent */
    if (object) {
        if (object->parent) {
            sp_style_merge_from_parent(style, object->parent->style);
        }
    } else {
        if (repr->parent()) {
            /// \todo fixme: This is not the prettiest thing (Lauris)
            SPStyle *parent = sp_style_new(NULL);
            sp_style_read(parent, NULL, repr->parent());
            sp_style_merge_from_parent(style, parent);
            sp_style_unref(parent);
        }
    }
}


/**
 * Read style properties from object's repr.
 *
 * 1. Reset existing object style
 * 2. Load current effective object style
 * 3. Load i attributes from immediate parent (which has to be up-to-date)
 */
void
sp_style_read_from_object(SPStyle *style, SPObject *object)
{
    g_return_if_fail(style != NULL);
    g_return_if_fail(object != NULL);
    g_return_if_fail(SP_IS_OBJECT(object));

    Inkscape::XML::Node *repr = object->getRepr();
    g_return_if_fail(repr != NULL);

    sp_style_read(style, object, repr);
}


/**
 * Read style properties from preferences.
 * @param style The style to write to
 * @param path Preferences directory from which the style should be read
 */
void
sp_style_read_from_prefs(SPStyle *style, Glib::ustring const &path)
{
    g_return_if_fail(style != NULL);
    g_return_if_fail(path != "");

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    // not optimal: we reconstruct the node based on the prefs, then pass it to
    // sp_style_read for actual processing.
    Inkscape::XML::SimpleDocument *tempdoc = new Inkscape::XML::SimpleDocument;
    Inkscape::XML::Node *tempnode = tempdoc->createElement("temp");

    std::vector<Inkscape::Preferences::Entry> attrs = prefs->getAllEntries(path);
    for (std::vector<Inkscape::Preferences::Entry>::iterator i = attrs.begin(); i != attrs.end(); ++i) {
        tempnode->setAttribute(i->getEntryName().data(), i->getString().data());
    }

    sp_style_read(style, NULL, tempnode);

    Inkscape::GC::release(tempnode);
    Inkscape::GC::release(tempdoc);
    delete tempdoc;
}



/**
 *
 */
static void
sp_style_privatize_text(SPStyle *style)
{
    SPTextStyle *text = style->text;
    style->text = sp_text_style_duplicate_unset(style->text);
    sp_text_style_unref(text);
    style->text_private = TRUE;
}


/**
 * Merge property into style.
 *
 * Should be called in order of highest to lowest precedence.
 * E.g. for a single style string, call from the last declaration to the first,
 * as CSS says that later declarations override earlier ones.
 *
 * \pre val != NULL.
 */
static void
sp_style_merge_property(SPStyle *style, gint id, gchar const *val)
{
    g_return_if_fail(val != NULL);

    switch (id) {
        case SP_PROP_INKSCAPE_FONT_SPEC:
            if (!style->text_private) sp_style_privatize_text(style);
            if (!style->text->font_specification.set) {
                gchar *val_unquoted = attribute_unquote(val);
                sp_style_read_istring(&style->text->font_specification, val_unquoted);
                if (val_unquoted) g_free (val_unquoted);
            }
            break;
        /* CSS2 */
        /* Font */
        case SP_PROP_FONT_FAMILY:
            if (!style->text_private) sp_style_privatize_text(style);
            if (!style->text->font_family.set) {
                gchar *val_unquoted = attribute_unquote(val);
                sp_style_read_istring(&style->text->font_family, val_unquoted);
                if (val_unquoted) g_free (val_unquoted);
            }
            break;
        case SP_PROP_FONT_SIZE:
            SPS_READ_IFONTSIZE_IF_UNSET(&style->font_size, val);
            break;
        case SP_PROP_FONT_SIZE_ADJUST:
            if (strcmp(val, "none") != 0) {
                g_warning("Unimplemented style property id SP_PROP_FONT_SIZE_ADJUST: value: %s", val);
            }
            break;
        case SP_PROP_FONT_STYLE:
            SPS_READ_IENUM_IF_UNSET(&style->font_style, val, enum_font_style, true);
            break;
        case SP_PROP_FONT_VARIANT:
            SPS_READ_IENUM_IF_UNSET(&style->font_variant, val, enum_font_variant, true);
            break;
        case SP_PROP_FONT_WEIGHT:
            SPS_READ_IENUM_IF_UNSET(&style->font_weight, val, enum_font_weight, true);
            break;
        case SP_PROP_FONT_STRETCH:
            SPS_READ_IENUM_IF_UNSET(&style->font_stretch, val, enum_font_stretch, true);
            break;
        case SP_PROP_FONT:
            if (!style->text_private) sp_style_privatize_text(style);
            if (!style->text->font.set) {
                g_free(style->text->font.value);
                style->text->font.value = g_strdup(val);
                style->text->font.set = TRUE;
                style->text->font.inherit = (val && !strcmp(val, "inherit"));

                // Break string into white space separated tokens
                std::stringstream os( val );
                Glib::ustring param;

                while (os >> param) {

                    // CSS is case insensitive but we're comparing against lowercase strings
                    Glib::ustring lparam = param.lowercase();

                    if (lparam == "/") {

                        os >> param;
                        // Eat the line-height for the moment as it is not an SVG property.
                        // lparam = param.lowercase();
                        // sp_style_read_ilengthornormal(&style->line_height, lparam);

                    } else {

                        // Skip if "normal" as that is the default (and we don't know which attribute it applies to).
                        if (lparam == "normal") continue;

                        // Check each property in turn

                        // font-style
                        SPIEnum test_style;
                        test_style.set = FALSE;

                        // Read once to see if param is valid style. If valid, .set will be TRUE.
                        sp_style_read_ienum(&test_style, lparam.c_str(), enum_font_style, true);

                        // If valid style parameter
                        if (test_style.set) {

                            // If not previously set
                            if (!style->font_style.set) {
                                style->font_style.set      = TRUE;
                                style->font_style.inherit  = test_style.inherit;
                                style->font_style.value    = test_style.value;
                                style->font_style.computed = test_style.computed;
                            }
                            continue; // Next parameter.
                        }

                        // font-variant (small-caps)
                        SPIEnum test_variant;
                        test_variant.set = FALSE;
                        sp_style_read_ienum(&test_variant, lparam.c_str(), enum_font_variant, true);

                        // If valid variant parameter
                        if (test_variant.set) {

                            // If not previously set
                            if (!style->font_variant.set) {
                                style->font_variant.set      = TRUE;
                                style->font_variant.inherit  = test_variant.inherit;
                                style->font_variant.value    = test_variant.value;
                                style->font_variant.computed = test_variant.computed;
                            }
                            continue; // Next parameter.
                        }

                        // font-weight
                        SPIEnum test_weight;
                        test_weight.set = FALSE;
                        sp_style_read_ienum(&test_weight, lparam.c_str(), enum_font_weight, true);

                        // If valid weight parameter
                        if (test_weight.set) {

                            // If not previously set
                            if (!style->font_weight.set) {
                                style->font_weight.set      = TRUE;
                                style->font_weight.inherit  = test_weight.inherit;
                                style->font_weight.value    = test_weight.value;
                                style->font_weight.computed = test_weight.computed;
                            }
                            continue; // Next parameter
                        }

                        // Font-size
                        SPIFontSize test_size;
                        test_size.set = FALSE;

                        // Read once to see if param is valid size.
                        sp_style_read_ifontsize( &test_size, lparam.c_str() );

                        // If valid size parameter
                        if (test_size.set) {

                            // If not previously set
                            if (!style->font_size.set) {
                                style->font_size.set      = TRUE;
                                style->font_size.inherit  = test_size.inherit;
                                style->font_size.unit     = test_size.unit;
                                style->font_size.value    = test_size.value;
                                style->font_size.computed = test_size.computed;
                                style->font_size.type     = test_size.type;
                                style->font_size.literal  = test_size.literal;
                            }
                            continue;
                        }

                        // No valid property value found.
                        break;
                    }
                } // params

                // The rest must be font-family...
                std::string val_s = val;
                std::string family = val_s.substr( val_s.find( param ) );

                if (!style->text_private) sp_style_privatize_text(style);
                if (!style->text->font_family.set) {
                    gchar *val_unquoted = attribute_unquote( family.c_str() );
                    sp_style_read_istring(&style->text->font_family, val_unquoted);
                    if (val_unquoted) g_free (val_unquoted);
                }

                // Set all properties to their default values per CSS 2.1 spec if not already set
                SPS_READ_IFONTSIZE_IF_UNSET(&style->font_size, "medium" );
                SPS_READ_IENUM_IF_UNSET(&style->font_style,   "normal", enum_font_style,   true);
                SPS_READ_IENUM_IF_UNSET(&style->font_variant, "normal", enum_font_variant, true);
                SPS_READ_IENUM_IF_UNSET(&style->font_weight,  "normal", enum_font_weight,  true);
                // Line height is not an SVG property but Inkscape uses it for multi-line text.
                // sp_style_read_ilengthornormal(&style->line_height, "normal");

            }

            break;

            /* Text */
        case SP_PROP_TEXT_INDENT:
            SPS_READ_ILENGTH_IF_UNSET(&style->text_indent, val);
            break;
        case SP_PROP_TEXT_ALIGN:
            SPS_READ_IENUM_IF_UNSET(&style->text_align, val, enum_text_align, true);
            break;
        case SP_PROP_TEXT_DECORATION:
            if (!style->text_decoration_line.set) {
                sp_style_read_itextdecoration(&style->text_decoration_line, &style->text_decoration_style, &style->text_decoration_color, val);
            }
            break;
        case SP_PROP_TEXT_DECORATION_LINE:
            if (!style->text_decoration_line.set) {
                sp_style_read_itextdecorationLine(&style->text_decoration_line, val);
            }
            break;
        case SP_PROP_TEXT_DECORATION_STYLE:
            if (!style->text_decoration_style.set) {
                sp_style_read_itextdecorationStyle(&style->text_decoration_style, val);
            }
            break;
        case SP_PROP_TEXT_DECORATION_COLOR:
            if (!style->text_decoration_color.set) {
                sp_style_read_itextdecorationColor(&style->text_decoration_color, val);
            }
            break;
        case SP_PROP_LINE_HEIGHT:
            if (!style->line_height.set) {
                sp_style_read_ilengthornormal(&style->line_height, val);
            }
            break;
        case SP_PROP_LETTER_SPACING:
            if (!style->letter_spacing.set) {
                sp_style_read_ilengthornormal(&style->letter_spacing, val);
            }
            break;
        case SP_PROP_WORD_SPACING:
            if (!style->word_spacing.set) {
                sp_style_read_ilengthornormal(&style->word_spacing, val);
            }
            break;
        case SP_PROP_TEXT_TRANSFORM:
            SPS_READ_IENUM_IF_UNSET(&style->text_transform, val, enum_text_transform, true);
            break;
            /* Text (css3) */
        case SP_PROP_DIRECTION:
            SPS_READ_IENUM_IF_UNSET(&style->direction, val, enum_direction, true);
            break;
        case SP_PROP_BLOCK_PROGRESSION:
            SPS_READ_IENUM_IF_UNSET(&style->block_progression, val, enum_block_progression, true);
            break;
        case SP_PROP_WRITING_MODE:
            SPS_READ_IENUM_IF_UNSET(&style->writing_mode, val, enum_writing_mode, true);
            break;
        case SP_PROP_TEXT_ANCHOR:
            SPS_READ_IENUM_IF_UNSET(&style->text_anchor, val, enum_text_anchor, true);
            break;
        case SP_PROP_BASELINE_SHIFT:
            SPS_READ_IBASELINE_SHIFT_IF_UNSET(&style->baseline_shift, val);
            break;
        case SP_PROP_TEXT_RENDERING: {
            SPS_READ_IENUM_IF_UNSET(&style->text_rendering, val, enum_text_rendering, true);
            break;
        }
        case SP_PROP_ALIGNMENT_BASELINE:
            g_warning("Unimplemented style property SP_PROP_ALIGNMENT_BASELINE: value: %s", val);
            break;
        case SP_PROP_DOMINANT_BASELINE:
            g_warning("Unimplemented style property SP_PROP_DOMINANT_BASELINE: value: %s", val);
            break;
        case SP_PROP_GLYPH_ORIENTATION_HORIZONTAL:
            g_warning("Unimplemented style property SP_PROP_ORIENTATION_HORIZONTAL: value: %s", val);
            break;
        case SP_PROP_GLYPH_ORIENTATION_VERTICAL:
            g_warning("Unimplemented style property SP_PROP_ORIENTATION_VERTICAL: value: %s", val);
            break;
        case SP_PROP_KERNING:
            g_warning("Unimplemented style property SP_PROP_KERNING: value: %s", val);
            break;
            /* Misc */
        case SP_PROP_CLIP:
            g_warning("Unimplemented style property SP_PROP_CLIP: value: %s", val);
            break;
        case SP_PROP_COLOR:
            if (!style->color.set) {
                sp_style_read_icolor(&style->color, val, style, (style->object) ? style->object->document : NULL);
            }
            break;
        case SP_PROP_CURSOR:
            g_warning("Unimplemented style property SP_PROP_CURSOR: value: %s", val);
            break;
        case SP_PROP_DISPLAY:
            SPS_READ_IENUM_IF_UNSET(&style->display, val, enum_display, true);
            break;
        case SP_PROP_OVERFLOW:
            /** \todo
             * FIXME: not supported properly yet, we just read and write it,
             * but act as if it is always "display".
             */
            SPS_READ_IENUM_IF_UNSET(&style->overflow, val, enum_overflow, true);
            break;
        case SP_PROP_VISIBILITY:
            SPS_READ_IENUM_IF_UNSET(&style->visibility, val, enum_visibility, true);
            break;
        case SP_PROP_ISOLATION:
            SPS_READ_IENUM_IF_UNSET(&style->isolation,  val, enum_isolation,  true);
            break;
        case SP_PROP_BLEND_MODE:
            SPS_READ_IENUM_IF_UNSET(&style->blend_mode, val, enum_blend_mode, true);
            break;

            /* SVG */
            /* Clip/Mask */
        case SP_PROP_CLIP_PATH:
            /** \todo
             * This is a workaround. Inkscape only supports 'clip-path' as SVG attribute, not as
             * style property. By having both CSS and SVG attribute set, editing of clip-path
             * will fail, since CSS always overwrites SVG attributes.
             * Fixes Bug #324849
             */
            g_warning("attribute 'clip-path' given as CSS");

            //XML Tree being directly used here.
            style->object->getRepr()->setAttribute("clip-path", val);
            break;
        case SP_PROP_CLIP_RULE:
            if (!style->clip_rule.set) {
                sp_style_read_ienum(&style->clip_rule, val, enum_clip_rule, true);
            }
            break;
        case SP_PROP_MASK:
            /** \todo
             * See comment for SP_PROP_CLIP_PATH
             */
            g_warning("attribute 'mask' given as CSS");
            
            //XML Tree being directly used here.
            style->object->getRepr()->setAttribute("mask", val);
            break;
        case SP_PROP_OPACITY:
            if (!style->opacity.set) {
                sp_style_read_iscale24(&style->opacity, val);
            }
            break;
        case SP_PROP_ENABLE_BACKGROUND:
            SPS_READ_IENUM_IF_UNSET(&style->enable_background, val,
                                    enum_enable_background, true);
            break;
            /* Filter */
        case SP_PROP_FILTER:
            if (!style->filter.set && !style->filter.inherit) {
                sp_style_read_ifilter(val, style, (style->object) ? style->object->document : NULL);
            }
            break;
        case SP_PROP_FLOOD_COLOR:
            g_warning("Unimplemented style property SP_PROP_FLOOD_COLOR: value: %s", val);
            break;
        case SP_PROP_FLOOD_OPACITY:
            g_warning("Unimplemented style property SP_PROP_FLOOD_OPACITY: value: %s", val);
            break;
        case SP_PROP_LIGHTING_COLOR:
            g_warning("Unimplemented style property SP_PROP_LIGHTING_COLOR: value: %s", val);
            break;
            /* Gradient */
        case SP_PROP_STOP_COLOR:
            g_warning("Unimplemented style property SP_PROP_STOP_COLOR: value: %s", val);
            break;
        case SP_PROP_STOP_OPACITY:
            g_warning("Unimplemented style property SP_PROP_STOP_OPACITY: value: %s", val);
            break;
            /* Interactivity */
        case SP_PROP_POINTER_EVENTS:
            g_warning("Unimplemented style property SP_PROP_POINTER_EVENTS: value: %s", val);
            break;
            /* Paint */
        case SP_PROP_COLOR_INTERPOLATION:
            // We read it but issue warning
            SPS_READ_IENUM_IF_UNSET(&style->color_interpolation, val, enum_color_interpolation, true);
            if( style->color_interpolation.value != SP_CSS_COLOR_INTERPOLATION_SRGB ) {
                g_warning("Inkscape currently only supports color-interpolation = sRGB");
            }
            break;
        case SP_PROP_COLOR_INTERPOLATION_FILTERS:
            SPS_READ_IENUM_IF_UNSET(&style->color_interpolation_filters, val, enum_color_interpolation, true);
            break;
        case SP_PROP_COLOR_PROFILE:
            g_warning("Unimplemented style property SP_PROP_COLOR_PROFILE: value: %s", val);
            break;
        case SP_PROP_COLOR_RENDERING: {
            SPS_READ_IENUM_IF_UNSET(&style->color_rendering, val, enum_color_rendering, true);
            break;
        }
        case SP_PROP_FILL:
            if (!style->fill.set) {
                style->fill.read( val, *style, (style->object) ? style->object->document : NULL );
            }
            break;
        case SP_PROP_FILL_OPACITY:
            if (!style->fill_opacity.set) {
                sp_style_read_iscale24(&style->fill_opacity, val);
            }
            break;
        case SP_PROP_FILL_RULE:
            if (!style->fill_rule.set) {
                sp_style_read_ienum(&style->fill_rule, val, enum_fill_rule, true);
            }
            break;
        case SP_PROP_IMAGE_RENDERING: {
            SPS_READ_IENUM_IF_UNSET(&style->image_rendering, val, enum_image_rendering, true);
            break;
        }
        case SP_PROP_MARKER:
            /* TODO:  Call sp_uri_reference_resolve(SPDocument *document, guchar const *uri) */
            /* style->marker[SP_MARKER_LOC] = g_quark_from_string(val); */
            if (!style->marker[SP_MARKER_LOC].set) {
                g_free(style->marker[SP_MARKER_LOC].value);
                style->marker[SP_MARKER_LOC].value = g_strdup(val);
                style->marker[SP_MARKER_LOC].set = TRUE;
                style->marker[SP_MARKER_LOC].inherit = (val && !strcmp(val, "inherit"));
            }
            break;

        case SP_PROP_MARKER_START:
            /* TODO:  Call sp_uri_reference_resolve(SPDocument *document, guchar const *uri) */
            if (!style->marker[SP_MARKER_LOC_START].set) {
                g_free(style->marker[SP_MARKER_LOC_START].value);
                style->marker[SP_MARKER_LOC_START].value = g_strdup(val);
                style->marker[SP_MARKER_LOC_START].set = TRUE;
                style->marker[SP_MARKER_LOC_START].inherit = (val && !strcmp(val, "inherit"));
            }
            break;
        case SP_PROP_MARKER_MID:
            /* TODO:  Call sp_uri_reference_resolve(SPDocument *document, guchar const *uri) */
            if (!style->marker[SP_MARKER_LOC_MID].set) {
                g_free(style->marker[SP_MARKER_LOC_MID].value);
                style->marker[SP_MARKER_LOC_MID].value = g_strdup(val);
                style->marker[SP_MARKER_LOC_MID].set = TRUE;
                style->marker[SP_MARKER_LOC_MID].inherit = (val && !strcmp(val, "inherit"));
            }
            break;
        case SP_PROP_MARKER_END:
            /* TODO:  Call sp_uri_reference_resolve(SPDocument *document, guchar const *uri) */
            if (!style->marker[SP_MARKER_LOC_END].set) {
                g_free(style->marker[SP_MARKER_LOC_END].value);
                style->marker[SP_MARKER_LOC_END].value = g_strdup(val);
                style->marker[SP_MARKER_LOC_END].set = TRUE;
                style->marker[SP_MARKER_LOC_END].inherit = (val && !strcmp(val, "inherit"));
            }
            break;

        case SP_PROP_SHAPE_RENDERING: {
            SPS_READ_IENUM_IF_UNSET(&style->shape_rendering, val, enum_shape_rendering, true);
            break;
        }

        case SP_PROP_STROKE:
            if (!style->stroke.set) {
                style->stroke.read( val, *style, (style->object) ? style->object->document : NULL );
            }
            break;
        case SP_PROP_STROKE_WIDTH:
            SPS_READ_ILENGTH_IF_UNSET(&style->stroke_width, val);
            break;
        case SP_PROP_STROKE_DASHARRAY:
            if (!style->stroke_dasharray.set) {
                sp_style_read_dash(style, val);
            }
            break;
        case SP_PROP_STROKE_DASHOFFSET:
            SPS_READ_ILENGTH_IF_UNSET(&style->stroke_dashoffset, val);
            break;
        case SP_PROP_STROKE_LINECAP:
            if (!style->stroke_linecap.set) {
                sp_style_read_ienum(&style->stroke_linecap, val, enum_stroke_linecap, true);
            }
            break;
        case SP_PROP_STROKE_LINEJOIN:
            if (!style->stroke_linejoin.set) {
                sp_style_read_ienum(&style->stroke_linejoin, val, enum_stroke_linejoin, true);
            }
            break;
        case SP_PROP_STROKE_MITERLIMIT:
            if (!style->stroke_miterlimit.set) {
                sp_style_read_ifloat(&style->stroke_miterlimit, val);
            }
            break;
        case SP_PROP_STROKE_OPACITY:
            if (!style->stroke_opacity.set) {
                sp_style_read_iscale24(&style->stroke_opacity, val);
            }
            break;
        case SP_PROP_PAINT_ORDER:
            if (!style->paint_order.set) {
                sp_style_read_ipaintorder(&style->paint_order, val);
            }
            break;

        default:
            g_warning("Invalid style property id: %d value: %s", id, val);
            break;
    }
}

static void
sp_style_merge_style_from_decl(SPStyle *const style, CRDeclaration const *const decl)
{
    /** \todo Ensure that property is lcased, as per
     * http://www.w3.org/TR/REC-CSS2/syndata.html#q4.
     * Should probably be done in libcroco.
     */
    unsigned const prop_idx = sp_attribute_lookup(decl->property->stryng->str);
    if (prop_idx != SP_ATTR_INVALID) {
        /** \todo
         * effic: Test whether the property is already set before trying to
         * convert to string. Alternatively, set from CRTerm directly rather
         * than converting to string.
         */
        guchar *const str_value_unsigned = cr_term_to_string(decl->value);
        gchar *const str_value = reinterpret_cast<gchar *>(str_value_unsigned);
        sp_style_merge_property(style, prop_idx, str_value);
        g_free(str_value);
    }
}

static void
sp_style_merge_from_props(SPStyle *const style, CRPropList *const props)
{
#if 0 /* forwards */
    for (CRPropList const *cur = props; cur; cur = cr_prop_list_get_next(cur)) {
        CRDeclaration *decl = NULL;
        cr_prop_list_get_decl(cur, &decl);
        sp_style_merge_style_from_decl(style, decl);
    }
#else /* in reverse order, as we need later declarations to take precedence over earlier ones. */
    if (props) {
        sp_style_merge_from_props(style, cr_prop_list_get_next(props));
        CRDeclaration *decl = NULL;
        cr_prop_list_get_decl(props, &decl);
        sp_style_merge_style_from_decl(style, decl);
    }
#endif
}

/**
 * \pre decl_list != NULL
 */
static void
sp_style_merge_from_decl_list(SPStyle *const style, CRDeclaration const *const decl_list)
{
    // read the decls from end to start, using head recursion, so that latter declarations override
    // (Ref: http://www.w3.org/TR/REC-CSS2/cascade.html#cascading-order point 4.)
    // because sp_style_merge_style_from_decl only sets properties that are unset
    if (decl_list->next) {
        sp_style_merge_from_decl_list(style, decl_list->next);
    }
    sp_style_merge_style_from_decl(style, decl_list);
}

static CRSelEng *
sp_repr_sel_eng()
{
    CRSelEng *const ret = cr_sel_eng_new();
    cr_sel_eng_set_node_iface(ret, &Inkscape::XML::croco_node_iface);

    /** \todo
     * Check whether we need to register any pseudo-class handlers.
     * libcroco has its own default handlers for first-child and lang.
     *
     * We probably want handlers for link and arguably visited (though
     * inkscape can't visit links at the time of writing).  hover etc.
     * more useful in inkview than the editor inkscape.
     *
     * http://www.w3.org/TR/SVG11/styling.html#StylingWithCSS says that
     * the following should be honoured, at least by inkview:
     * :hover, :active, :focus, :visited, :link.
     */

    g_assert(ret);
    return ret;
}

static void
sp_style_merge_from_object_stylesheet(SPStyle *const style, SPObject const *const object)
{
    static CRSelEng *sel_eng = NULL;
    if (!sel_eng) {
        sel_eng = sp_repr_sel_eng();
    }

    CRPropList *props = NULL;

    //XML Tree being directly used here while it shouldn't be.
    CRStatus status = cr_sel_eng_get_matched_properties_from_cascade(sel_eng,
                                                                     object->document->style_cascade,
                                                                     object->getRepr(),
                                                                     &props);
    g_return_if_fail(status == CR_OK);
    /// \todo Check what errors can occur, and handle them properly.
    if (props) {
        sp_style_merge_from_props(style, props);
        cr_prop_list_destroy(props);
    }
}

/**
 * Parses a style="..." string and merges it with an existing SPStyle.
 */
void
sp_style_merge_from_style_string(SPStyle *const style, gchar const *const p)
{
    /*
     * Reference: http://www.w3.org/TR/SVG11/styling.html#StyleAttribute:
     * ``When CSS styling is used, CSS inline style is specified by including
     * semicolon-separated property declarations of the form "name : value"
     * within the style attribute''.
     *
     * That's fairly ambiguous.  Is a `value' allowed to contain semicolons?
     * Why does it say "including", what else is allowed in the style
     * attribute value?
     */

    CRDeclaration *const decl_list
        = cr_declaration_parse_list_from_buf(reinterpret_cast<guchar const *>(p), CR_UTF_8);
    if (decl_list) {
        sp_style_merge_from_decl_list(style, decl_list);
        cr_declaration_destroy(decl_list);
    }
}

/** Indexed by SP_CSS_FONT_SIZE_blah. */
static float const font_size_table[] = {6.0, 8.0, 10.0, 12.0, 14.0, 18.0, 24.0};

static void
sp_style_merge_font_size_from_parent(SPIFontSize &child, SPIFontSize const &parent)
{
    /* 'font-size' */
    if (!child.set || child.inherit) {
        /* Inherit the computed value.  Reference: http://www.w3.org/TR/SVG11/styling.html#Inheritance */
        child.computed = parent.computed;
    } else if (child.type == SP_FONT_SIZE_LITERAL) {
        /** \todo
         * fixme: SVG and CSS do not specify clearly, whether we should use
         * user or screen coordinates (Lauris)
         */
        if (child.literal < SP_CSS_FONT_SIZE_SMALLER) {
            child.computed = font_size_table[child.literal];
        } else if (child.literal == SP_CSS_FONT_SIZE_SMALLER) {
            child.computed = parent.computed / 1.2;
        } else if (child.literal == SP_CSS_FONT_SIZE_LARGER) {
            child.computed = parent.computed * 1.2;
        } else {
            /* Illegal value */
        }
    } else if (child.type == SP_FONT_SIZE_PERCENTAGE) {
        /* Unlike most other lengths, percentage for font size is relative to parent computed value
         * rather than viewport. */
        child.computed = parent.computed * child.value;
    } else if (child.type == SP_FONT_SIZE_LENGTH) {
        switch (child.unit) {
            case SP_CSS_UNIT_EM:
                /* Relative to parent font size */
                child.computed = parent.computed * child.value;
                break;
            case SP_CSS_UNIT_EX:
                /* Relative to parent font size */
                child.computed = parent.computed * child.value * 0.5; /* Hack */
                break;
            default:
                /* No change */
                break;
        }
    }
}

// Some shifts are defined relative to parent.
static void
sp_style_merge_baseline_shift_from_parent(SPIBaselineShift &child, SPIBaselineShift const &parent,
                                          SPIFontSize const &pfont_size)
{
    /* 'baseline-shift' */
    if (!child.set || child.inherit) {
        /* Inherit the computed value.  Reference: http://www.w3.org/TR/SVG11/styling.html#Inheritance */
        child.computed = parent.computed;  // Does this make sense (applying a shift a second time)?
    } else if (child.type == SP_BASELINE_SHIFT_LITERAL) {
        if( child.literal == SP_CSS_BASELINE_SHIFT_BASELINE ) {
            child.computed = 0; // No change
        } else if (child.literal == SP_CSS_BASELINE_SHIFT_SUB ) {
            // Should use subscript position from font relative to alphabetic baseline
            // OpenOffice, Adobe: -0.33, Word -0.14, LaTex about -0.2.
            child.computed = -0.2 * pfont_size.computed; 
        } else if (child.literal == SP_CSS_BASELINE_SHIFT_SUPER ) {
            // Should use superscript position from font relative to alphabetic baseline
            // OpenOffice, Adobe: 0.33, Word 0.35, LaTex about 0.45.
            child.computed =  0.4 * pfont_size.computed; 
        } else {
            /* Illegal value */
        }
    } else if (child.type == SP_BASELINE_SHIFT_PERCENTAGE) {
        // Percentage for baseline shift is relative to computed "line-height"
        // which is just font-size (see SVG1.1 'font').
        child.computed = pfont_size.computed * child.value;
    } else if (child.type == SP_BASELINE_SHIFT_LENGTH) {
        switch (child.unit) {
            case SP_CSS_UNIT_EM:
                child.computed = child.value * pfont_size.computed; 
                break;
            case SP_CSS_UNIT_EX:
                child.computed = child.value * 0.5 * pfont_size.computed; 
                break;
            default:
                /* No change */
                break;
        }
    }
    // baseline-shifts are relative to parent baseline
    child.computed += parent.computed;
}

/**
 * Sets computed values in \a style, which may involve inheriting from (or in some other way
 * calculating from) corresponding computed values of \a parent.
 *
 * References: http://www.w3.org/TR/SVG11/propidx.html shows what properties inherit by default.
 * http://www.w3.org/TR/SVG11/styling.html#Inheritance gives general rules as to what it means to
 * inherit a value.  http://www.w3.org/TR/REC-CSS2/cascade.html#computed-value is more precise
 * about what the computed value is (not obvious for lengths).
 *
 * \pre \a parent's computed values are already up-to-date.
 */
void
sp_style_merge_from_parent(SPStyle *const style, SPStyle const *const parent)
{
    g_return_if_fail(style != NULL);

    /** \todo
     * fixme: Check for existing callers that might pass null parent.
     * This should probably be g_return_if_fail, or else we should make a
     * best attempt to set computed values correctly without having a parent
     * (i.e., by assuming parent has initial values).
     */
    if (!parent)
        return;

    /* CSS2 */
    /* Font */
    sp_style_merge_font_size_from_parent(style->font_size, parent->font_size);

    /* 'font-style' */
    if (!style->font_style.set || style->font_style.inherit) {
        style->font_style.computed = parent->font_style.computed;
    }

    /* 'font-variant' */
    if (!style->font_variant.set || style->font_variant.inherit) {
        style->font_variant.computed = parent->font_variant.computed;
    }

    /* 'font-weight' */
    if (!style->font_weight.set || style->font_weight.inherit) {
        style->font_weight.computed = parent->font_weight.computed;
    } else if (style->font_weight.value == SP_CSS_FONT_WEIGHT_NORMAL) {
        /** \todo
         * fixme: This is unconditional, i.e., happens even if parent not
         * present.
         */
        style->font_weight.computed = SP_CSS_FONT_WEIGHT_400;
    } else if (style->font_weight.value == SP_CSS_FONT_WEIGHT_BOLD) {
        style->font_weight.computed = SP_CSS_FONT_WEIGHT_700;
    } else if (style->font_weight.value == SP_CSS_FONT_WEIGHT_LIGHTER) {
        unsigned const parent_val = parent->font_weight.computed;
        g_assert(SP_CSS_FONT_WEIGHT_100 == 0);
        // strictly, 'bolder' and 'lighter' should go to the next weight
        // expressible in the current font family, but that's difficult to
        // find out, so jumping by 3 seems an appropriate approximation
        style->font_weight.computed = (parent_val <= SP_CSS_FONT_WEIGHT_100 + 3
                                       ? (unsigned)SP_CSS_FONT_WEIGHT_100
                                       : parent_val - 3);
        g_assert(style->font_weight.computed <= (unsigned) SP_CSS_FONT_WEIGHT_900);
    } else if (style->font_weight.value == SP_CSS_FONT_WEIGHT_BOLDER) {
        unsigned const parent_val = parent->font_weight.computed;
        g_assert(parent_val <= SP_CSS_FONT_WEIGHT_900);
        style->font_weight.computed = (parent_val >= SP_CSS_FONT_WEIGHT_900 - 3
                                       ? (unsigned)SP_CSS_FONT_WEIGHT_900
                                       : parent_val + 3);
        g_assert(style->font_weight.computed <= (unsigned) SP_CSS_FONT_WEIGHT_900);
    }

    /* 'font-stretch' */
    if (!style->font_stretch.set || style->font_stretch.inherit) {
        style->font_stretch.computed = parent->font_stretch.computed;
    } else if (style->font_stretch.value == SP_CSS_FONT_STRETCH_NARROWER) {
        unsigned const parent_val = parent->font_stretch.computed;
        style->font_stretch.computed = (parent_val == SP_CSS_FONT_STRETCH_ULTRA_CONDENSED
                                        ? parent_val
                                        : parent_val - 1);
        g_assert(style->font_stretch.computed <= (unsigned) SP_CSS_FONT_STRETCH_ULTRA_EXPANDED);
    } else if (style->font_stretch.value == SP_CSS_FONT_STRETCH_WIDER) {
        unsigned const parent_val = parent->font_stretch.computed;
        g_assert(parent_val <= SP_CSS_FONT_STRETCH_ULTRA_EXPANDED);
        style->font_stretch.computed = (parent_val == SP_CSS_FONT_STRETCH_ULTRA_EXPANDED
                                        ? parent_val
                                        : parent_val + 1);
        g_assert(style->font_stretch.computed <= (unsigned) SP_CSS_FONT_STRETCH_ULTRA_EXPANDED);
    }

    /* text (css2) */
    if (!style->text_indent.set || style->text_indent.inherit) {
        style->text_indent.computed = parent->text_indent.computed;
    }

    if (!style->text_align.set || style->text_align.inherit) {
        style->text_align.computed = parent->text_align.computed;
    }

    if (!style->text_decoration_line.set || style->text_decoration_line.inherit) {
        style->text_decoration_line.underline    = parent->text_decoration_line.underline;
        style->text_decoration_line.overline     = parent->text_decoration_line.overline;
        style->text_decoration_line.line_through = parent->text_decoration_line.line_through;
        style->text_decoration_line.blink        = parent->text_decoration_line.blink;
    }

    if (!style->text_decoration_style.set || style->text_decoration_style.inherit) {
        style->text_decoration_style.solid    = parent->text_decoration_style.solid;
        style->text_decoration_style.isdouble = parent->text_decoration_style.isdouble;
        style->text_decoration_style.dotted   = parent->text_decoration_style.dotted;
        style->text_decoration_style.dashed   = parent->text_decoration_style.dashed;
        style->text_decoration_style.wavy     = parent->text_decoration_style.wavy;
    }

    if (!style->text_decoration_color.set || style->text_decoration_color.inherit) {
        sp_style_merge_ipaint(style, &style->text_decoration_color, &parent->text_decoration_color);
    }

    if (!style->line_height.set || style->line_height.inherit) {
        style->line_height.value = parent->line_height.value;
        style->line_height.computed = parent->line_height.computed;
        style->line_height.normal = parent->line_height.normal;
    }

    if (!style->letter_spacing.set || style->letter_spacing.inherit) {
        style->letter_spacing.value = parent->letter_spacing.value;
        style->letter_spacing.computed = parent->letter_spacing.computed;
        style->letter_spacing.normal = parent->letter_spacing.normal;
    }

    if (!style->word_spacing.set || style->word_spacing.inherit) {
        style->word_spacing.value = parent->word_spacing.value;
        style->word_spacing.computed = parent->word_spacing.computed;
        style->word_spacing.normal = parent->word_spacing.normal;
    }

    if (!style->text_transform.set || style->text_transform.inherit) {
        style->text_transform.computed = parent->text_transform.computed;
    }

    if (!style->direction.set || style->direction.inherit) {
        style->direction.computed = parent->direction.computed;
    }

    if (!style->block_progression.set || style->block_progression.inherit) {
        style->block_progression.computed = parent->block_progression.computed;
    }

    if (!style->writing_mode.set || style->writing_mode.inherit) {
        style->writing_mode.computed = parent->writing_mode.computed;
    }

    if (!style->text_anchor.set || style->text_anchor.inherit) {
        style->text_anchor.computed = parent->text_anchor.computed;
    }

    /* Baseline Shift... Some shifts are relative to parent. */
    sp_style_merge_baseline_shift_from_parent(style->baseline_shift, parent->baseline_shift,
                                              parent->font_size);

    if (style->opacity.inherit) {
        style->opacity.value = parent->opacity.value;
    }

    /* Color */
    if (!style->color.set || style->color.inherit) {
        sp_style_merge_ipaint(style, &style->color, &parent->color);
    }
    if (!style->color_interpolation.set || style->color_interpolation.inherit) {
        style->color_interpolation.computed = parent->color_interpolation.computed;
    }
    if (!style->color_interpolation_filters.set || style->color_interpolation_filters.inherit) {
        style->color_interpolation_filters.computed = parent->color_interpolation_filters.computed;
    }


    /* Fill */
    if (!style->fill.set || style->fill.inherit || style->fill.currentcolor) {
        sp_style_merge_ipaint(style, &style->fill, &parent->fill);
    }

    if (!style->fill_opacity.set || style->fill_opacity.inherit) {
        style->fill_opacity.value = parent->fill_opacity.value;
    }

    if (!style->fill_rule.set || style->fill_rule.inherit) {
        style->fill_rule.computed = parent->fill_rule.computed;
    }

    /* Stroke */
    if (!style->stroke.set || style->stroke.inherit || style->stroke.currentcolor) {
        sp_style_merge_ipaint(style, &style->stroke, &parent->stroke);
    }

    if (!style->stroke_width.set || style->stroke_width.inherit) {
        style->stroke_width.computed = parent->stroke_width.computed;
    } else {
        /* Update computed value for any change in font inherited from parent. */
        double const em = style->font_size.computed;
        if (style->stroke_width.unit == SP_CSS_UNIT_EM) {
            style->stroke_width.computed = style->stroke_width.value * em;
        } else if (style->stroke_width.unit == SP_CSS_UNIT_EX) {
            double const ex = em * 0.5;  // fixme: Get x height from libnrtype or pango.
            style->stroke_width.computed = style->stroke_width.value * ex;
        }
    }

    if (!style->stroke_linecap.set || style->stroke_linecap.inherit) {
        style->stroke_linecap.computed = parent->stroke_linecap.computed;
    }

    if (!style->stroke_linejoin.set || style->stroke_linejoin.inherit) {
        style->stroke_linejoin.computed = parent->stroke_linejoin.computed;
    }

    if (!style->stroke_miterlimit.set || style->stroke_miterlimit.inherit) {
        style->stroke_miterlimit.value = parent->stroke_miterlimit.value;
    }

    if (!style->stroke_dasharray.set || style->stroke_dasharray.inherit) {
        style->stroke_dasharray.values = parent->stroke_dasharray.values;
    }

    if (!style->stroke_dashoffset.set || style->stroke_dashoffset.inherit) {
        style->stroke_dashoffset.value = parent->stroke_dashoffset.value;
    }

    if (!style->stroke_opacity.set || style->stroke_opacity.inherit) {
        style->stroke_opacity.value = parent->stroke_opacity.value;
    }

    if (!style->paint_order.set || style->paint_order.inherit) {
        g_free(style->paint_order.value);
        style->paint_order.value = g_strdup(parent->paint_order.value);
        for (unsigned i = 0; i < PAINT_ORDER_LAYERS; ++i) {
            style->paint_order.layer[i]     = parent->paint_order.layer[i];
            style->paint_order.layer_set[i] = parent->paint_order.layer_set[i];
        }
    }

    if (style->text && parent->text) {
        if (!style->text->font_family.set || style->text->font_family.inherit) {
            g_free(style->text->font_family.value);
            style->text->font_family.value = g_strdup(parent->text->font_family.value);
        }
    }

    if (style->text && parent->text) {
        if (!style->text->font_specification.set || style->text->font_specification.inherit) {
            g_free(style->text->font_specification.value);
            style->text->font_specification.value = g_strdup(parent->text->font_specification.value);
        }
    }

    /* Markers - Free the old value and make copy of the new */
    for (unsigned i = SP_MARKER_LOC; i < SP_MARKER_LOC_QTY; i++) {
        if (!style->marker[i].set || style->marker[i].inherit) {
            g_free(style->marker[i].value);
            style->marker[i].value = g_strdup(parent->marker[i].value);
        }
    }

    /* Filter effects */
    if (style->filter.inherit) {
        sp_style_merge_ifilter(style, &parent->filter);
    }

    if(style->enable_background.inherit) {
        style->enable_background.value = parent->enable_background.value;
    }

    /* Clipping */
    if (!style->clip_rule.set || style->clip_rule.inherit) {
        style->clip_rule.computed = parent->clip_rule.computed;
    }

    /* Rendering */
    if (!style->color_rendering.set || style->color_rendering.inherit) {
        style->color_rendering.computed = parent->color_rendering.computed;
    }
    if (!style->image_rendering.set || style->image_rendering.inherit) {
        style->image_rendering.computed = parent->image_rendering.computed;
    }
    if (!style->shape_rendering.set || style->shape_rendering.inherit) {
        style->shape_rendering.computed = parent->shape_rendering.computed;
    }
    if (!style->text_rendering.set || style->text_rendering.inherit) {
        style->text_rendering.computed = parent->text_rendering.computed;
    }
}

template <typename T>
static void
sp_style_merge_prop_from_dying_parent(T &child, T const &parent)
{
    if ( ( !(child.set) || child.inherit )
         && parent.set && !(parent.inherit) )
    {
        child = parent;
    }
}

/**
 * Copy SPIString from parent to child.
 */
static void
sp_style_merge_string_prop_from_dying_parent(SPIString &child, SPIString const &parent)
{
    if ( ( !(child.set) || child.inherit )
         && parent.set && !(parent.inherit) )
    {
        g_free(child.value);
        child.value = g_strdup(parent.value);
        child.set = parent.set;
        child.inherit = parent.inherit;
    }
}

static void
sp_style_merge_paint_prop_from_dying_parent(SPStyle *style,
                                            SPIPaint &child, SPIPaint const &parent)
{
    /** \todo
     * I haven't given this much attention.  See comments below about
     * currentColor, colorProfile, and relative URIs.
     */
    if (!child.set || child.inherit) {
        sp_style_merge_ipaint(style, &child, &parent);
        child.set = parent.set;
        child.inherit = parent.inherit;
    }
}

static void
sp_style_merge_rel_enum_prop_from_dying_parent(SPIEnum &child, SPIEnum const &parent,
                                               unsigned const max_computed_val,
                                               unsigned const smaller_val)
{
    /* We assume that min computed val is 0, contiguous up to max_computed_val,
       then zero or more other absolute values, then smaller_val then larger_val. */
    unsigned const min_computed_val = 0;
    unsigned const larger_val = smaller_val + 1;
    g_return_if_fail(min_computed_val < max_computed_val);
    g_return_if_fail(max_computed_val < smaller_val);

    if (parent.set && !parent.inherit) {
        if (!child.set || child.inherit) {
            child.value = parent.value;
            child.set = parent.set;  // i.e. true
            child.inherit = parent.inherit;  // i.e. false
        } else if (child.value < smaller_val) {
            /* Child has absolute value: leave as is. */
        } else if ( ( child.value == smaller_val
                      && parent.value == larger_val )
                    || ( parent.value == smaller_val
                         && child.value == larger_val ) )
        {
            child.set = false;
            /*
             * Note that this can result in a change in computed value in the
             * rare case that the parent's setting was a no-op (i.e. if the
             * parent's parent's computed value was already ultra-condensed or
             * ultra-expanded).  However, I'd guess that the change is for the
             * better: I'd guess that if the properties were specified
             * relatively, then the intent is to counteract parent's effect.
             * I.e. I believe this is the best code even in that case.
             */
        } else if (child.value == parent.value) {
            /* Leave as is. */
            /** \todo
             * It's unclear what to do if style and parent specify the same
             * relative directive (narrower or wider).  We can either convert
             * to absolute specification or coalesce to a single relative
             * request (of half the strength of the original pair).
             *
             * Converting to a single level of relative specification is a
             * better choice if the newly-unlinked clone is itself cloned to
             * other contexts (inheriting different font stretchiness): it
             * would preserve the effect that it will be narrower than
             * the inherited context in each case.  The disadvantage is that
             * it will ~certainly affect the computed value of the
             * newly-unlinked clone.
             */
        } else {
            unsigned const parent_val = parent.computed;
            child.value = ( child.value == smaller_val
                            ? ( parent_val == min_computed_val
                                ? parent_val
                                : parent_val - 1 )
                            : ( parent_val == max_computed_val
                                ? parent_val
                                : parent_val + 1 ) );
            g_assert(child.value <= max_computed_val);
            child.inherit = false;
            g_assert(child.set);
        }
    }
}

template <typename LengthT>
static void
sp_style_merge_length_prop_from_dying_parent(LengthT &child, LengthT const &parent,
                                             double const parent_child_em_ratio)
{
    if ( ( !(child.set) || child.inherit )
         && parent.set && !(parent.inherit) )
    {
        child = parent;
        switch (parent.unit) {
            case SP_CSS_UNIT_EM:
            case SP_CSS_UNIT_EX:
                child.value *= parent_child_em_ratio;
                /** \todo
                 * fixme: Have separate ex ratio parameter.
                 * Get x height from libnrtype or pango.
                 */
                if (!IS_FINITE(child.value)) {
                    child.value = child.computed;
                    child.unit = SP_CSS_UNIT_NONE;
                }
                break;

            default:
                break;
        }
    }
}

static double
get_relative_font_size_frac(SPIFontSize const &font_size)
{
    switch (font_size.type) {
        case SP_FONT_SIZE_LITERAL: {
            switch (font_size.literal) {
                case SP_CSS_FONT_SIZE_SMALLER:
                    return 5.0 / 6.0;

                case SP_CSS_FONT_SIZE_LARGER:
                    return 6.0 / 5.0;

                default:
                    g_assert_not_reached();
            }
        }

        case SP_FONT_SIZE_PERCENTAGE:
            return font_size.value;

        case SP_FONT_SIZE_LENGTH: {
            switch (font_size.unit ) {
                case SP_CSS_UNIT_EM:
                    return font_size.value;

                case SP_CSS_UNIT_EX:
                    return font_size.value * 0.5;

                default:
                    g_assert_not_reached();
            }
        }
    }
    g_assert_not_reached();
}

/**
 * Combine \a style and \a parent style specifications into a single style specification that
 * preserves (as much as possible) the effect of the existing \a style being a child of \a parent.
 *
 * Called when the parent repr is to be removed (e.g. the parent is a \<use\> element that is being
 * unlinked), in which case we copy/adapt property values that are explicitly set in \a parent,
 * trying to retain the same visual appearance once the parent is removed.  Interesting cases are
 * when there is unusual interaction with the parent's value (opacity, display) or when the value
 * can be specified as relative to the parent computed value (font-size, font-weight etc.).
 *
 * Doesn't update computed values of \a style.  For correctness, you should subsequently call
 * sp_style_merge_from_parent against the new parent (presumably \a parent's parent) even if \a
 * style was previously up-to-date wrt \a parent.
 *
 * \pre \a parent's computed values are already up-to-date.
 *   (\a style's computed values needn't be up-to-date.)
 */
void
sp_style_merge_from_dying_parent(SPStyle *const style, SPStyle const *const parent)
{
    /** \note
     * The general rule for each property is as follows:
     *
     *   If style is set to an absolute value, then leave it as is.
     *
     *   Otherwise (i.e. if style has a relative value):
     *
     *     If parent is set to an absolute value, then set style to the computed value.
     *
     *     Otherwise, calculate the combined relative value (e.g. multiplying the two percentages).
     */

    /* We do font-size first, to ensure that em size is up-to-date. */
    /** \todo
     * fixme: We'll need to have more font-related things up the top once
     * we're getting x-height from pango or libnrtype.
     */

    /* Some things that allow relative specifications. */
    {
        /* font-size.  Note that we update the computed font-size of style,
           to assist in em calculations later in this function. */

        if (parent->font_size.set && !parent->font_size.inherit) {
            /* Parent has defined font-size */

            if (!style->font_size.set || style->font_size.inherit) {
                /* font_size inherits the computed value, so we can use the parent value
                 * verbatim. */
                style->font_size = parent->font_size;

            } else if ( style->font_size.type == SP_FONT_SIZE_LENGTH  && 
                        style->font_size.unit != SP_CSS_UNIT_EM &&
                        style->font_size.unit != SP_CSS_UNIT_EX ) {

                /* Child already has absolute size (stored in computed value), so do nothing. */

            } else if ( style->font_size.type == SP_FONT_SIZE_LITERAL
                        && style->font_size.literal < SP_CSS_FONT_SIZE_SMALLER ) {
                /* Child already has absolute size, but we ensure that the computed value
                   is up-to-date. */
                unsigned const ix = style->font_size.literal;
                g_assert(ix < G_N_ELEMENTS(font_size_table));
                style->font_size.computed = font_size_table[ix];

            } else {
                /* Child has relative size. */
                double const child_frac(get_relative_font_size_frac(style->font_size));
                style->font_size.set = true;
                style->font_size.inherit = false;
                style->font_size.computed = parent->font_size.computed * child_frac;

                if ( ( parent->font_size.type == SP_FONT_SIZE_LITERAL
                       && parent->font_size.literal < SP_CSS_FONT_SIZE_SMALLER ) ||
                     ( parent->font_size.type == SP_FONT_SIZE_LENGTH &&
                       parent->font_size.unit != SP_CSS_UNIT_EM &&
                       parent->font_size.unit != SP_CSS_UNIT_EX ) ) {

                    /* Absolute value. */
                    style->font_size.type = SP_FONT_SIZE_LENGTH;
                    /* .value is unused for non ex/em SP_FONT_SIZE_LENGTH. */

                } else {
                    /* Relative value. */
                        
                    double const parent_frac(get_relative_font_size_frac(parent->font_size));
                    if( style->font_size.type == SP_FONT_SIZE_LENGTH ) {
                        /* Value in terms of ex/em */
                        style->font_size.value *= parent_frac;
                    } else {
                        style->font_size.value = parent_frac * child_frac;
                        style->font_size.type = SP_FONT_SIZE_PERCENTAGE;
                    }
                }
            }
        }

        /* 'font-stretch' */
        sp_style_merge_rel_enum_prop_from_dying_parent(style->font_stretch,
                                                       parent->font_stretch,
                                                       SP_CSS_FONT_STRETCH_ULTRA_EXPANDED,
                                                       SP_CSS_FONT_STRETCH_NARROWER);

        /* font-weight */
        sp_style_merge_rel_enum_prop_from_dying_parent(style->font_weight,
                                                       parent->font_weight,
                                                       SP_CSS_FONT_WEIGHT_900,
                                                       SP_CSS_FONT_WEIGHT_LIGHTER);
    }


    /* Enum values that don't have any relative settings (other than `inherit'). */
    {
        SPIEnum SPStyle::*const fields[] = {
            &SPStyle::blend_mode,
            &SPStyle::clip_rule,
            &SPStyle::color_interpolation,
            &SPStyle::color_interpolation_filters,
            &SPStyle::color_rendering,
            &SPStyle::direction,
            &SPStyle::fill_rule,
            &SPStyle::font_style,
            &SPStyle::font_variant,
            &SPStyle::image_rendering,
            &SPStyle::isolation,
            //nyi: SPStyle::pointer_events,
            &SPStyle::shape_rendering,
            &SPStyle::stroke_linecap,
            &SPStyle::stroke_linejoin,
            &SPStyle::text_anchor,
            &SPStyle::text_rendering,
            &SPStyle::visibility,
            &SPStyle::writing_mode
        };

        for (unsigned i = 0; i < G_N_ELEMENTS(fields); ++i) {
            SPIEnum SPStyle::*const fld = fields[i];
            sp_style_merge_prop_from_dying_parent<SPIEnum>(style->*fld, parent->*fld);
        }
    }

    /* A few other simple inheritance properties. */
    {
        sp_style_merge_prop_from_dying_parent<SPIScale24>(style->fill_opacity, parent->fill_opacity);
        sp_style_merge_prop_from_dying_parent<SPIScale24>(style->stroke_opacity, parent->stroke_opacity);
        sp_style_merge_prop_from_dying_parent<SPIFloat>(style->stroke_miterlimit, parent->stroke_miterlimit);

        /** \todo
         * We currently treat text-decoration as if it were a simple inherited
         * property (fixme). This code may need changing once we do the
         * special fill/stroke inheritance mentioned by the spec.
         */
        sp_style_merge_prop_from_dying_parent<SPITextDecorationLine>( style->text_decoration_line,  parent->text_decoration_line);
        sp_style_merge_prop_from_dying_parent<SPITextDecorationStyle>(style->text_decoration_style, parent->text_decoration_style);
        sp_style_merge_paint_prop_from_dying_parent(style,style->text_decoration_color, parent->text_decoration_color);

        //nyi: font-size-adjust,  // <number> | none | inherit
        //nyi: glyph-orientation-horizontal,
        //nyi: glyph-orientation-vertical,
    }

    /* Properties that involve length but are easy in other respects. */
    {
        /* The difficulty with lengths is that font-relative units need adjusting if the font
         * varies between parent & child.
         *
         * Lengths specified in the existing child can stay as they are: its computed font
         * specification should stay unchanged, so em & ex lengths should continue to mean the same
         * size.
         *
         * Lengths specified in the dying parent in em or ex need to be scaled according to the
         * ratio of em or ex size between parent & child.
         */
        double const parent_child_em_ratio = parent->font_size.computed / style->font_size.computed;

        SPILength SPStyle::*const lfields[] = {
            &SPStyle::stroke_width,
            &SPStyle::text_indent
        };
        for (unsigned i = 0; i < G_N_ELEMENTS(lfields); ++i) {
            SPILength SPStyle::*fld = lfields[i];
            sp_style_merge_length_prop_from_dying_parent<SPILength>(style->*fld,
                                                                    parent->*fld,
                                                                    parent_child_em_ratio);
        }

        SPILengthOrNormal SPStyle::*const nfields[] = {
            &SPStyle::letter_spacing,
            &SPStyle::line_height,
            &SPStyle::word_spacing
        };
        for (unsigned i = 0; i < G_N_ELEMENTS(nfields); ++i) {
            SPILengthOrNormal SPStyle::*fld = nfields[i];
            sp_style_merge_length_prop_from_dying_parent<SPILengthOrNormal>(style->*fld,
                                                                            parent->*fld,
                                                                            parent_child_em_ratio);
        }

        //nyi: &SPStyle::kerning: length or `auto'

        /* fixme: Move stroke-dash and stroke-dash-offset here once they
           can accept units. */
    }

    /* Properties that involve a URI but are easy in other respects. */
    {
        /** \todo
         * Could cause problems if original object was in another document
         * and it used a relative URL.  (At the time of writing, we don't
         * allow hrefs to other documents, so this isn't a problem yet.)
         * Paint properties also allow URIs.
         */
        //nyi: cursor,   // may involve change in effect, but we can't do much better
        //nyi: color-profile,

        // Markers (marker-start, marker-mid, marker-end).
        for (unsigned i = SP_MARKER_LOC; i < SP_MARKER_LOC_QTY; i++) {
            sp_style_merge_string_prop_from_dying_parent(style->marker[i], parent->marker[i]);
        }
    }

    /* CSS2 */
    /* Font */

    if (style->text && parent->text) {
        sp_style_merge_string_prop_from_dying_parent(style->text->font_specification,
                                                     parent->text->font_specification);

        sp_style_merge_string_prop_from_dying_parent(style->text->font_family,
                                                     parent->text->font_family);
    }


    /* Properties that don't inherit by default.  Most of these need special handling. */
    {
        /*
         * opacity's effect is cumulative; we set the new value to the combined effect.  The
         * default value for opacity is 1.0, not inherit.  (Note that stroke-opacity and
         * fill-opacity are quite different from opacity, and don't need any special handling.)
         *
         * Cases:
         * - parent & child were each previously unset, in which case the effective
         *   opacity value is 1.0, and style should remain unset.
         * - parent was previously unset (so computed opacity value of 1.0)
         *   and child was set to inherit.  The merged child should
         *   get a value of 1.0, and shouldn't inherit (lest the new parent
         *   has a different opacity value).  Given that opacity's default
         *   value is 1.0 (rather than inherit), we might as well have the
         *   merged child's opacity be unset.
         * - parent was previously unset (so opacity 1.0), and child was set to a number.
         *   The merged child should retain its existing settings (though it doesn't matter
         *   if we make it unset if that number was 1.0).
         * - parent was inherit and child was unset.  Merged child should be set to inherit.
         * - parent was inherit and child was inherit.  (We can't in general reproduce this
         *   effect (short of introducing a new group), but setting opacity to inherit is rare.)
         *   If the inherited value was strictly between 0.0 and 1.0 (exclusive) then the merged
         *   child's value should be set to the product of the two, i.e. the square of the
         *   inherited value, and should not be marked as inherit.  (This decision assumes that it
         *   is more important to retain the effective opacity than to retain the inheriting
         *   effect, and assumes that the inheriting effect either isn't important enough to create
         *   a group or isn't common enough to bother maintaining the code to create a group.)  If
         *   the inherited value was 0.0 or 1.0, then marking the merged child as inherit comes
         *   closer to maintaining the effect.
         * - parent was inherit and child was set to a numerical value.  If the child's value
         *   was 1.0, then the merged child should have the same settings as the parent.
         *   If the child's value was 0, then the merged child should also be set to 0.
         *   If the child's value was anything else, then we do the same as for the inherit/inherit
         *   case above: have the merged child set to the product of the two opacities and not
         *   marked as inherit, for the same reasons as for that case.
         * - parent was set to a value, and child was unset.  The merged child should have
         *   parent's settings.
         * - parent was set to a value, and child was inherit.  The merged child should
         *   be set to the product, i.e. the square of the parent's value.
         * - parent & child are each set to a value.  The merged child should be set to the
         *   product.
         */
        if ( !style->opacity.set
             || ( !style->opacity.inherit
                  && style->opacity.value == SP_SCALE24_MAX ) )
        {
            style->opacity = parent->opacity;
        } else {
            /* Ensure that style's computed value is up-to-date. */
            if (style->opacity.inherit) {
                style->opacity.value = parent->opacity.value;
            }

            /* Multiplication of opacities occurs even if a child's opacity is set to inherit. */
            style->opacity.value = SP_SCALE24_MUL(style->opacity.value,
                                                  parent->opacity.value);

            style->opacity.inherit = (parent->opacity.inherit
                                      && style->opacity.inherit
                                      && (parent->opacity.value == 0 ||
                                          parent->opacity.value == SP_SCALE24_MAX));
            style->opacity.set = ( style->opacity.inherit
                                   || style->opacity.value < SP_SCALE24_MAX );
        }

        /* display is in principle similar to opacity, but implementation is easier. */
        if ( parent->display.set && !parent->display.inherit
             && parent->display.value == SP_CSS_DISPLAY_NONE ) {
            style->display.value = SP_CSS_DISPLAY_NONE;
            style->display.set = true;
            style->display.inherit = false;
        } else if (style->display.inherit) {
            style->display.value = parent->display.value;
            style->display.set = parent->display.set;
            style->display.inherit = parent->display.inherit;
        } else {
            /* Leave as is.  (display doesn't inherit by default.) */
        }

        /* enable-background - this is rather complicated, because
         * it is valid only when applied to container elements.
         * Let's check a simple case anyhow. */
        if (parent->enable_background.set
            && !parent->enable_background.inherit
            && style->enable_background.inherit)
        {
            style->enable_background.set = true;
            style->enable_background.inherit = false;
            style->enable_background.value = parent->enable_background.value;
        }

        if (!style->filter.set || style->filter.inherit)
        {
            sp_style_merge_ifilter(style, &parent->filter);
        
}
        /** \todo
         * fixme: Check that we correctly handle all properties that don't
         * inherit by default (as shown in
         * http://www.w3.org/TR/SVG11/propidx.html for most SVG 1.1 properties).
         */
    }

    /* SPIPaint properties (including color). */
    {
        /** \todo
         * Think about the issues involved if specified as currentColor or
         * if specified relative to colorProfile, and if the currentColor or
         * colorProfile differs between parent \& child.  See also comments
         * elsewhere in this function about URIs.
         */
        SPIPaint SPStyle::*const fields[] = {
            &SPStyle::color,
            &SPStyle::fill,
            &SPStyle::stroke
        };
        for (unsigned i = 0; i < G_N_ELEMENTS(fields); ++i) {
            SPIPaint SPStyle::*const fld = fields[i];
            sp_style_merge_paint_prop_from_dying_parent(style, style->*fld, parent->*fld);
        }
    }

    /* Things from SVG 1.2 or CSS3. */
    {
        /* Note: If we ever support setting string values for text-align then we'd need strdup
         * handling here. */
        sp_style_merge_prop_from_dying_parent<SPIEnum>(style->text_align, parent->text_align);

        sp_style_merge_prop_from_dying_parent<SPIEnum>(style->text_transform, parent->text_transform);
        sp_style_merge_prop_from_dying_parent<SPIEnum>(style->block_progression, parent->block_progression);
    }

    /* Note: this will need length handling once dasharray supports units. */
    if ( ( !style->stroke_dasharray.set || style->stroke_dasharray.inherit )
         && parent->stroke_dasharray.set && !parent->stroke_dasharray.inherit )
    {
        style->stroke_dasharray.values = parent->stroke_dasharray.values;
        style->stroke_dasharray.set = parent->stroke_dasharray.set;
        style->stroke_dasharray.inherit = parent->stroke_dasharray.inherit;
    }

    {
        sp_style_merge_prop_from_dying_parent<SPILength>(style->stroke_dashoffset, parent->stroke_dashoffset);
    }
}


static void
sp_style_set_ipaint_to_uri(SPStyle *style, SPIPaint *paint, const Inkscape::URI *uri, SPDocument *document)
{
    // it may be that this style's SPIPaint has not yet created its URIReference;
    // now that we have a document, we can create it here
    if (!paint->value.href && document) {
        paint->value.href = new SPPaintServerReference(document);
        paint->value.href->changedSignal().connect(sigc::bind(sigc::ptr_fun((paint == &style->fill)? sp_style_fill_paint_server_ref_changed : sp_style_stroke_paint_server_ref_changed), style));
    }

    if (paint->value.href){
        if (paint->value.href->getObject()){
            paint->value.href->detach();
        }

        try {
            paint->value.href->attach(*uri);
        } catch (Inkscape::BadURIException &e) {
            g_warning("%s", e.what());
            paint->value.href->detach();
        }
    }
}

static void
sp_style_set_ipaint_to_uri_string (SPStyle *style, SPIPaint *paint, const gchar *uri)
{
    try {
        const Inkscape::URI IURI(uri);
        sp_style_set_ipaint_to_uri(style, paint, &IURI, style->document);
    } catch (...) {
        g_warning("URI failed to parse: %s", uri);
    }
}

void
sp_style_set_to_uri_string (SPStyle *style, bool isfill, const gchar *uri)
{
    sp_style_set_ipaint_to_uri_string (style, isfill? &style->fill : &style->stroke, uri);
}

gchar const *
sp_style_get_css_unit_string(int unit)
{
    // specify px by default, see inkscape bug 1221626, mozilla bug 234789

    switch (unit) {

        case SP_CSS_UNIT_NONE: return "px";
        case SP_CSS_UNIT_PX: return "px";
        case SP_CSS_UNIT_PT: return "pt";
        case SP_CSS_UNIT_PC: return "pc";
        case SP_CSS_UNIT_MM: return "mm";
        case SP_CSS_UNIT_CM: return "cm";
        case SP_CSS_UNIT_IN: return "in";
        case SP_CSS_UNIT_EM: return "em";
        case SP_CSS_UNIT_EX: return "ex";
        case SP_CSS_UNIT_PERCENT: return "%";
        default: return "px";
    }
    return "px";
}

/*
 * Convert a size in pixels into another CSS unit size
 */
double
sp_style_css_size_px_to_units(double size, int unit)
{
    double unit_size = size;
    switch (unit) {

        case SP_CSS_UNIT_NONE: unit_size = size; break;
        case SP_CSS_UNIT_PX: unit_size = size; break;
        case SP_CSS_UNIT_PT: unit_size = Inkscape::Util::Quantity::convert(size, "px", "pt");  break;
        case SP_CSS_UNIT_PC: unit_size = Inkscape::Util::Quantity::convert(size, "px", "pc");  break;
        case SP_CSS_UNIT_MM: unit_size = Inkscape::Util::Quantity::convert(size, "px", "mm");  break;
        case SP_CSS_UNIT_CM: unit_size = Inkscape::Util::Quantity::convert(size, "px", "cm");  break;
        case SP_CSS_UNIT_IN: unit_size = Inkscape::Util::Quantity::convert(size, "px", "in");  break;
        case SP_CSS_UNIT_EM: unit_size = size / SP_CSS_FONT_SIZE_DEFAULT; break;
        case SP_CSS_UNIT_EX: unit_size = size * 2.0 / SP_CSS_FONT_SIZE_DEFAULT ; break;
        case SP_CSS_UNIT_PERCENT: unit_size = size * 100.0 / SP_CSS_FONT_SIZE_DEFAULT; break;

        default:
            g_warning("sp_style_get_css_font_size_units conversion to %d not implemented.", unit);
            break;
    }

    return unit_size;
}

/*
 * Convert a size in a CSS unit size to pixels
 */
double
sp_style_css_size_units_to_px(double size, int unit)
{
    if (unit == SP_CSS_UNIT_PX) {
        return size;
    }
    //g_message("sp_style_css_size_units_to_px %f %d = %f px", size, unit, out);
    return size * (size / sp_style_css_size_px_to_units(size, unit));;
}
/**
 *
 */
static void
sp_style_merge_ipaint(SPStyle *style, SPIPaint *paint, SPIPaint const *parent)
{
    if ((paint->set && paint->currentcolor) || parent->currentcolor) {
        bool isset = paint->set;
        paint->clear();
        paint->set = isset;
        paint->currentcolor = TRUE;
        paint->setColor(style->color.value.color);
        return;
    }

    paint->clear();
    if ( parent->isPaintserver() ) {
        if (parent->value.href) {
            sp_style_set_ipaint_to_uri(style, paint, parent->value.href->getURI(), parent->value.href->getOwnerDocument());
        } else {
            g_warning("Expected paint server not found.");
        }
    } else if ( parent->isColor() ) {
        paint->setColor( parent->value.color );
    } else if ( parent->isNoneSet() ) {
        paint->noneSet = TRUE;
    } else if ( parent->isNone() ) {
        //
    } else {
        g_assert_not_reached();
    }
}


/**
 * Merge filter style from parent.
 * Filter effects do not inherit by default
 */
static void
sp_style_merge_ifilter(SPStyle *style, SPIFilter const *parent)
{
    // FIXME:
    // instead of just copying over, we need to _really merge_ the two filters by combining their
    // filter primitives

    sp_style_filter_clear(style);
    style->filter.set = parent->set;
    style->filter.inherit = parent->inherit;

    if (style->filter.href){
        if (style->filter.href->getObject()){
            style->filter.href->detach();
        }
    }
    else{
        // it may be that this style has not yet created its SPFilterReference
        if (style->object){
            if (style->object->document) {
                style->filter.href = new SPFilterReference(style->object->document);
                style->filter.href->changedSignal().connect(sigc::bind(sigc::ptr_fun(sp_style_filter_ref_changed), style));
            }
        }
    }

    if (style->filter.href && parent->href){
        if (parent->href->getObject()) {
            try {
                style->filter.href->attach(*parent->href->getURI());
            } catch (Inkscape::BadURIException &e) {
                g_warning("%s", e.what());
                style->filter.href->detach();
            }
        }
    }
}

/**
 * Dumps the style to a CSS string, with either SP_STYLE_FLAG_IFSET or
 * SP_STYLE_FLAG_ALWAYS flags. Used with Always for copying an object's
 * complete cascaded style to style_clipboard. When you need a CSS string
 * for an object in the document tree, you normally call
 * sp_style_write_difference instead to take into account the object's parent.
 *
 * \pre style != NULL.
 * \pre flags in {IFSET, ALWAYS}.
 * \post ret != NULL.
 */
gchar *
sp_style_write_string(SPStyle const *const style, guint const flags)
{
    /** \todo
     * Merge with write_difference, much duplicate code!
     */
    g_return_val_if_fail(style != NULL, NULL);
    g_return_val_if_fail(((flags == SP_STYLE_FLAG_IFSET) ||
                          (flags == SP_STYLE_FLAG_ALWAYS)  ),
                         NULL);

    gchar c[BMAX];
    gchar *p = c;
    *p = '\0';

    p += sp_style_write_ifontsize(p, c + BMAX - p, "font-size", &style->font_size, NULL, flags);
    p += sp_style_write_ienum(p, c + BMAX - p, "font-style", enum_font_style, &style->font_style, NULL, flags);
    p += sp_style_write_ienum(p, c + BMAX - p, "font-variant", enum_font_variant, &style->font_variant, NULL, flags);
    p += sp_style_write_ienum(p, c + BMAX - p, "font-weight", enum_font_weight, &style->font_weight, NULL, flags);
    p += sp_style_write_ienum(p, c + BMAX - p, "font-stretch", enum_font_stretch, &style->font_stretch, NULL, flags);

    /* Text */
    p += sp_style_write_ilength(p, c + BMAX - p, "text-indent", &style->text_indent, NULL, flags);
    p += sp_style_write_ienum(p, c + BMAX - p, "text-align", enum_text_align, &style->text_align, NULL, flags);
    p += sp_style_write_itextdecoration(p, c + BMAX - p, "text-decoration",
            &style->text_decoration_line, &style->text_decoration_style, &style->text_decoration_color,
            NULL, NULL, NULL, flags);
    p += sp_style_write_ilengthornormal(p, c + BMAX - p, "line-height", &style->line_height, NULL, flags);
    p += sp_style_write_ilengthornormal(p, c + BMAX - p, "letter-spacing", &style->letter_spacing, NULL, flags);
    p += sp_style_write_ilengthornormal(p, c + BMAX - p, "word-spacing", &style->word_spacing, NULL, flags);
    p += sp_style_write_ienum(p, c + BMAX - p, "text-transform", enum_text_transform, &style->text_transform, NULL, flags);
    p += sp_style_write_ienum(p, c + BMAX - p, "direction", enum_direction, &style->direction, NULL, flags);
    p += sp_style_write_ienum(p, c + BMAX - p, "block-progression", enum_block_progression, &style->block_progression, NULL, flags);
    p += sp_style_write_ienum(p, c + BMAX - p, "writing-mode", enum_writing_mode, &style->writing_mode, NULL, flags);

    p += sp_style_write_ienum(p, c + BMAX - p, "text-anchor", enum_text_anchor, &style->text_anchor, NULL, flags);
    p += sp_style_write_ibaselineshift(p, c + BMAX - p, "baseline-shift", &style->baseline_shift, NULL, flags);


    /// \todo fixme: Per type methods need default flag too (lauris)

    if (style->opacity.value != SP_SCALE24_MAX) {
        p += sp_style_write_iscale24(p, c + BMAX - p, "opacity", &style->opacity, NULL, flags);
    }

    if (!style->color.noneSet) { // CSS does not permit "none" for color
        p += sp_style_write_ipaint(p, c + BMAX - p, "color", &style->color, NULL, flags);
    }
    p += sp_style_write_ienum(p, c + BMAX - p, "color-interpolation", enum_color_interpolation, &style->color_interpolation, NULL, flags);
    p += sp_style_write_ienum(p, c + BMAX - p, "color-interpolation-filters", enum_color_interpolation, &style->color_interpolation_filters, NULL, flags);


    p += sp_style_write_ipaint(p, c + BMAX - p, "fill", &style->fill, NULL, flags);
    // if fill:none, skip writing fill properties
    if (!style->fill.noneSet) {
        p += sp_style_write_iscale24(p, c + BMAX - p, "fill-opacity", &style->fill_opacity, NULL, flags);
        p += sp_style_write_ienum(p, c + BMAX - p, "fill-rule", enum_fill_rule, &style->fill_rule, NULL, flags);
    }

    p += sp_style_write_ipaint(p, c + BMAX - p, "stroke", &style->stroke, NULL, flags);

    // stroke width affects markers, so write it if there's stroke OR any markers
    if (!style->stroke.noneSet ||
        style->marker[SP_MARKER_LOC].set ||
        style->marker[SP_MARKER_LOC_START].set ||
        style->marker[SP_MARKER_LOC_MID].set ||
        style->marker[SP_MARKER_LOC_END].set) {
        p += sp_style_write_ilength(p, c + BMAX - p, "stroke-width", &style->stroke_width, NULL, flags);
    }

    // if stroke:none, skip writing stroke properties
    if (!style->stroke.noneSet) {
        p += sp_style_write_ienum(p, c + BMAX - p, "stroke-linecap", enum_stroke_linecap, &style->stroke_linecap, NULL, flags);
        p += sp_style_write_ienum(p, c + BMAX - p, "stroke-linejoin", enum_stroke_linejoin, &style->stroke_linejoin, NULL, flags);
        p += sp_style_write_ifloat(p, c + BMAX - p, "stroke-miterlimit", &style->stroke_miterlimit, NULL, flags);
        p += sp_style_write_iscale24(p, c + BMAX - p, "stroke-opacity", &style->stroke_opacity, NULL, flags);
        p += sp_style_write_idasharray(p, c + BMAX - p, "stroke-dasharray", &style->stroke_dasharray, NULL, flags);
        p += sp_style_write_ilength(p, c + BMAX - p, "stroke-dashoffset", &style->stroke_dashoffset, NULL, flags);
    }

    if (style->paint_order.set) {
        p += sp_style_write_ipaintorder(p, c + BMAX - p, "paint-order", &style->paint_order, NULL, flags);
    } else if (flags == SP_STYLE_FLAG_ALWAYS) {
        p += g_snprintf(p, c + BMAX - p, "paint-order:normal;");
    }

    bool marker_none = false;
    gchar *master = style->marker[SP_MARKER_LOC].value;
    if (style->marker[SP_MARKER_LOC].set) {
        p += g_snprintf(p, c + BMAX - p, "marker:%s;", style->marker[SP_MARKER_LOC].value);
    } else if (flags == SP_STYLE_FLAG_ALWAYS) {
        p += g_snprintf(p, c + BMAX - p, "marker:none;");
        marker_none = true;
    }
    if (style->marker[SP_MARKER_LOC_START].set
       && (!master || strcmp(master, style->marker[SP_MARKER_LOC_START].value))) {
        p += g_snprintf(p, c + BMAX - p, "marker-start:%s;", style->marker[SP_MARKER_LOC_START].value);
    } else if (flags == SP_STYLE_FLAG_ALWAYS && !marker_none) {
        p += g_snprintf(p, c + BMAX - p, "marker-start:none;");
    }
    if (style->marker[SP_MARKER_LOC_MID].set
       && (!master || strcmp(master, style->marker[SP_MARKER_LOC_MID].value))) {
        p += g_snprintf(p, c + BMAX - p, "marker-mid:%s;", style->marker[SP_MARKER_LOC_MID].value);
    } else if (flags == SP_STYLE_FLAG_ALWAYS && !marker_none) {
        p += g_snprintf(p, c + BMAX - p, "marker-mid:none;");
    }
    if (style->marker[SP_MARKER_LOC_END].set
       && (!master || strcmp(master, style->marker[SP_MARKER_LOC_END].value))) {
        p += g_snprintf(p, c + BMAX - p, "marker-end:%s;", style->marker[SP_MARKER_LOC_END].value);
    } else if (flags == SP_STYLE_FLAG_ALWAYS && !marker_none) {
        p += g_snprintf(p, c + BMAX - p, "marker-end:none;");
    }

    p += sp_style_write_ienum(p, c + BMAX - p, "visibility", enum_visibility, &style->visibility, NULL, flags);
    p += sp_style_write_ienum(p, c + BMAX - p, "display", enum_display, &style->display, NULL, flags);
    p += sp_style_write_ienum(p, c + BMAX - p, "overflow", enum_overflow, &style->overflow, NULL, flags);
    p += sp_style_write_ienum(p, c + BMAX - p, "isolation",      enum_isolation,  &style->isolation,  NULL, flags);
    p += sp_style_write_ienum(p, c + BMAX - p, "mix-blend-mode", enum_blend_mode, &style->blend_mode, NULL, flags);

    /* filter: */
    p += sp_style_write_ifilter(p, c + BMAX - p, "filter", &style->filter, NULL, flags);

    p += sp_style_write_ienum(p, c + BMAX - p, "enable-background", enum_enable_background, &style->enable_background, NULL, flags);

    /* clipping */
    p += sp_style_write_ienum(p, c + BMAX - p, "clip-rule", enum_clip_rule, &style->clip_rule, NULL, flags);

    /* rendering */
    p += sp_style_write_ienum(p, c + BMAX - p, "color-rendering", enum_color_rendering, &style->color_rendering, NULL, flags);
    p += sp_style_write_ienum(p, c + BMAX - p, "image-rendering", enum_image_rendering, &style->image_rendering, NULL, flags);
    p += sp_style_write_ienum(p, c + BMAX - p, "shape-rendering", enum_shape_rendering, &style->shape_rendering, NULL, flags);
    p += sp_style_write_ienum(p, c + BMAX - p, "text-rendering",  enum_text_rendering,  &style->text_rendering,  NULL, flags);

    /* fixme: */
    p += sp_text_style_write(p, c + BMAX - p, style->text, flags);

    /* Get rid of trailing `;'. */
    if (p != c) {
        --p;
        if (*p == ';') {
            *p = '\0';
        }
    }

    return g_strdup(c);
}


#define STYLE_BUF_MAX


/**
 * Dumps style to CSS string, see sp_style_write_string()
 *
 * \pre from != NULL.
 * \pre to != NULL.
 * \post ret != NULL.
 */
gchar *
sp_style_write_difference(SPStyle const *const from, SPStyle const *const to)
{
    g_return_val_if_fail(from != NULL, NULL);
    g_return_val_if_fail(to != NULL, NULL);

    gchar c[BMAX], *p = c;
    *p = '\0';

    p += sp_style_write_ifontsize(p, c + BMAX - p, "font-size", &from->font_size, &to->font_size, SP_STYLE_FLAG_IFDIFF);
    p += sp_style_write_ienum(p, c + BMAX - p, "font-style", enum_font_style, &from->font_style, &to->font_style, SP_STYLE_FLAG_IFDIFF);
    p += sp_style_write_ienum(p, c + BMAX - p, "font-variant", enum_font_variant, &from->font_variant, &to->font_variant, SP_STYLE_FLAG_IFDIFF);
    p += sp_style_write_ienum(p, c + BMAX - p, "font-weight", enum_font_weight, &from->font_weight, &to->font_weight, SP_STYLE_FLAG_IFDIFF);
    p += sp_style_write_ienum(p, c + BMAX - p, "font-stretch", enum_font_stretch, &from->font_stretch, &to->font_stretch, SP_STYLE_FLAG_IFDIFF);

    /* Text */
    p += sp_style_write_ilength(p, c + BMAX - p, "text-indent", &from->text_indent, &to->text_indent, SP_STYLE_FLAG_IFDIFF);
    p += sp_style_write_ienum(p, c + BMAX - p, "text-align", enum_text_align, &from->text_align, &to->text_align, SP_STYLE_FLAG_IFDIFF);
    p += sp_style_write_itextdecoration(p, c + BMAX - p, "text-decoration",
            &from->text_decoration_line,  &from->text_decoration_style, &from->text_decoration_color,
            &to->text_decoration_line,    &to->text_decoration_style,   &to->text_decoration_color,
            SP_STYLE_FLAG_IFDIFF);
    p += sp_style_write_ilengthornormal(p, c + BMAX - p, "line-height", &from->line_height, &to->line_height, SP_STYLE_FLAG_IFDIFF);
    p += sp_style_write_ilengthornormal(p, c + BMAX - p, "letter-spacing", &from->letter_spacing, &to->letter_spacing, SP_STYLE_FLAG_IFDIFF);
    p += sp_style_write_ilengthornormal(p, c + BMAX - p, "word-spacing", &from->word_spacing, &to->word_spacing, SP_STYLE_FLAG_IFDIFF);
    p += sp_style_write_ienum(p, c + BMAX - p, "text-transform", enum_text_transform, &from->text_transform, &to->text_transform, SP_STYLE_FLAG_IFDIFF);
    p += sp_style_write_ienum(p, c + BMAX - p, "direction", enum_direction, &from->direction, &to->direction, SP_STYLE_FLAG_IFDIFF);
    p += sp_style_write_ienum(p, c + BMAX - p, "block-progression", enum_block_progression, &from->block_progression, &to->block_progression, SP_STYLE_FLAG_IFDIFF);
    p += sp_style_write_ienum(p, c + BMAX - p, "writing-mode", enum_writing_mode, &from->writing_mode, &to->writing_mode, SP_STYLE_FLAG_IFDIFF);

    p += sp_style_write_ienum(p, c + BMAX - p, "text-anchor", enum_text_anchor, &from->text_anchor, &to->text_anchor, SP_STYLE_FLAG_IFDIFF);
    p += sp_style_write_ibaselineshift(p, c + BMAX - p, "baseline-shift", &from->baseline_shift, &to->baseline_shift, SP_STYLE_FLAG_IFDIFF);

    /// \todo fixme: Per type methods need default flag too
    if (from->opacity.set && from->opacity.value != SP_SCALE24_MAX) {
        p += sp_style_write_iscale24(p, c + BMAX - p, "opacity", &from->opacity, &to->opacity, SP_STYLE_FLAG_IFSET);
    }

    if (!from->color.noneSet) { // CSS does not permit "none" for color
        p += sp_style_write_ipaint(p, c + BMAX - p, "color", &from->color, &to->color, SP_STYLE_FLAG_IFSET);
    }
    p += sp_style_write_ienum(p, c + BMAX - p, "color-interpolation", enum_color_interpolation, &from->color_interpolation, &to->color_interpolation, SP_STYLE_FLAG_IFDIFF);
    p += sp_style_write_ienum(p, c + BMAX - p, "color-interpolation-filters", enum_color_interpolation, &from->color_interpolation_filters, &to->color_interpolation_filters, SP_STYLE_FLAG_IFDIFF);

    p += sp_style_write_ipaint(p, c + BMAX - p, "fill", &from->fill, &to->fill, SP_STYLE_FLAG_IFDIFF);
    // if fill:none, skip writing fill properties
    if (!from->fill.noneSet) {
        p += sp_style_write_iscale24(p, c + BMAX - p, "fill-opacity", &from->fill_opacity, &to->fill_opacity, SP_STYLE_FLAG_IFDIFF);
        p += sp_style_write_ienum(p, c + BMAX - p, "fill-rule", enum_fill_rule, &from->fill_rule, &to->fill_rule, SP_STYLE_FLAG_IFDIFF);
    }

    p += sp_style_write_ipaint(p, c + BMAX - p, "stroke", &from->stroke, &to->stroke, SP_STYLE_FLAG_IFDIFF);

    // stroke width affects markers, so write it if there's stroke OR any markers
    if (!from->stroke.noneSet ||
        from->marker[SP_MARKER_LOC].set ||
        from->marker[SP_MARKER_LOC_START].set ||
        from->marker[SP_MARKER_LOC_MID].set ||
        from->marker[SP_MARKER_LOC_END].set) {
        p += sp_style_write_ilength(p, c + BMAX - p, "stroke-width", &from->stroke_width, &to->stroke_width, SP_STYLE_FLAG_IFDIFF);
    }

    // if stroke:none, skip writing stroke properties
    if (!from->stroke.noneSet) {
        p += sp_style_write_ienum(p, c + BMAX - p, "stroke-linecap", enum_stroke_linecap,
                                  &from->stroke_linecap, &to->stroke_linecap, SP_STYLE_FLAG_IFDIFF);
        p += sp_style_write_ienum(p, c + BMAX - p, "stroke-linejoin", enum_stroke_linejoin,
                                  &from->stroke_linejoin, &to->stroke_linejoin, SP_STYLE_FLAG_IFDIFF);
        p += sp_style_write_ifloat(p, c + BMAX - p, "stroke-miterlimit",
                                   &from->stroke_miterlimit, &to->stroke_miterlimit, SP_STYLE_FLAG_IFDIFF);
        p += sp_style_write_idasharray(p, c + BMAX - p, "stroke-dasharray",
                                   &from->stroke_dasharray, &to->stroke_dasharray, SP_STYLE_FLAG_IFDIFF);
        p += sp_style_write_ilength(p, c + BMAX - p, "stroke-dashoffset", &from->stroke_dashoffset, &to->stroke_dashoffset, SP_STYLE_FLAG_IFDIFF);
        p += sp_style_write_iscale24(p, c + BMAX - p, "stroke-opacity", &from->stroke_opacity, &to->stroke_opacity, SP_STYLE_FLAG_IFDIFF);
    }

    /* paint-order */
    if( from->paint_order.set) {
        p += sp_style_write_ipaintorder(p, c + BMAX - p, "paint-order", &from->paint_order, &to->paint_order, SP_STYLE_FLAG_IFDIFF);
    }

    /* markers */
    gchar *master = from->marker[SP_MARKER_LOC].value;
    if (master != NULL) {
        p += g_snprintf(p, c + BMAX - p, "marker:%s;", master);
    }
    if (from->marker[SP_MARKER_LOC_START].value != NULL && (!master || strcmp(master, from->marker[SP_MARKER_LOC_START].value))) {
        p += g_snprintf(p, c + BMAX - p, "marker-start:%s;", from->marker[SP_MARKER_LOC_START].value);
    }
    if (from->marker[SP_MARKER_LOC_MID].value != NULL && (!master || strcmp(master, from->marker[SP_MARKER_LOC_MID].value))) {
        p += g_snprintf(p, c + BMAX - p, "marker-mid:%s;",   from->marker[SP_MARKER_LOC_MID].value);
    }
    if (from->marker[SP_MARKER_LOC_END].value != NULL && (!master || strcmp(master, from->marker[SP_MARKER_LOC_END].value))) {
        p += g_snprintf(p, c + BMAX - p, "marker-end:%s;",   from->marker[SP_MARKER_LOC_END].value);
    }

    p += sp_style_write_ienum(p, c + BMAX - p, "visibility", enum_visibility, &from->visibility, &to->visibility, SP_STYLE_FLAG_IFSET);
    p += sp_style_write_ienum(p, c + BMAX - p, "display", enum_display, &from->display, &to->display, SP_STYLE_FLAG_IFSET);
    p += sp_style_write_ienum(p, c + BMAX - p, "overflow", enum_overflow, &from->overflow, &to->overflow, SP_STYLE_FLAG_IFSET);
    p += sp_style_write_ienum(p, c + BMAX - p, "isolation",      enum_isolation,  &from->isolation,  &to->isolation, SP_STYLE_FLAG_IFSET);
    p += sp_style_write_ienum(p, c + BMAX - p, "mix-blend-mode", enum_blend_mode, &from->blend_mode, &to->blend_mode, SP_STYLE_FLAG_IFSET);

    /* filter: */
    p += sp_style_write_ifilter(p, c + BMAX - p, "filter", &from->filter, &to->filter, SP_STYLE_FLAG_IFDIFF);

    p += sp_style_write_ienum(p, c + BMAX - p, "enable-background", enum_enable_background, &from->enable_background, &to->enable_background, SP_STYLE_FLAG_IFSET);

    p += sp_text_style_write(p, c + BMAX - p, from->text, SP_STYLE_FLAG_IFDIFF);

    p += sp_style_write_ienum(p, c + BMAX - p, "clip-rule", enum_clip_rule, &from->clip_rule, &to->clip_rule, SP_STYLE_FLAG_IFDIFF);

    /* rendering */
    p += sp_style_write_ienum(p, c + BMAX - p, "color-rendering", enum_color_rendering, &from->color_rendering, &to->color_rendering, SP_STYLE_FLAG_IFDIFF);
    p += sp_style_write_ienum(p, c + BMAX - p, "image-rendering", enum_image_rendering, &from->image_rendering, &to->image_rendering, SP_STYLE_FLAG_IFDIFF);
    p += sp_style_write_ienum(p, c + BMAX - p, "shape-rendering", enum_shape_rendering, &from->shape_rendering, &to->shape_rendering, SP_STYLE_FLAG_IFDIFF);
    p += sp_style_write_ienum(p, c + BMAX - p, "text-rendering",  enum_text_rendering,  &from->text_rendering,  &to->text_rendering,  SP_STYLE_FLAG_IFDIFF);

    /** \todo
     * The reason we use IFSET rather than IFDIFF is the belief that the IFDIFF
     * flag is mainly only for attributes that don't handle explicit unset well.
     * We may need to revisit the behaviour of this routine.
     */

    /* Get rid of trailing `;'. */
    if (p != c) {
        --p;
        if (*p == ';') {
            *p = '\0';
        }
    }

    return g_strdup(c);
}



/**
 * Reset all style properties.
 */
static void
sp_style_clear(SPStyle *style)
{
    g_return_if_fail(style != NULL);

    style->fill.clear();
    style->stroke.clear();
    sp_style_filter_clear(style);

    style->release_connection.disconnect();

    style->fill_ps_modified_connection.disconnect();
    if (style->fill.value.href) {
        delete style->fill.value.href;
        style->fill.value.href = NULL;
    }
    style->stroke_ps_modified_connection.disconnect();
    if (style->stroke.value.href) {
        delete style->stroke.value.href;
        style->stroke.value.href = NULL;
    }
    style->filter_modified_connection.disconnect();
    if (style->filter.href) {
        delete style->filter.href;
        style->filter.href = NULL;
    }

    style->stroke_dasharray.values.clear();
    style->stroke_dasharray.inherit = FALSE;
    style->stroke_dashoffset.inherit = FALSE;

    /** \todo fixme: Do that text manipulation via parents */
    SPObject *object = style->object;
    SPDocument *document = style->document;
    gint const refcount = style->refcount;
    SPTextStyle *text = style->text;
    unsigned const text_private = style->text_private;


    style->refcount = refcount;
    style->object = object;
    style->document = document;

    if (document) {
        style->filter.href = new SPFilterReference(document);
        style->filter.href->changedSignal().connect(sigc::bind(sigc::ptr_fun(sp_style_filter_ref_changed), style));

        style->fill.value.href = new SPPaintServerReference(document);
        style->fill.value.href->changedSignal().connect(sigc::bind(sigc::ptr_fun(sp_style_fill_paint_server_ref_changed), style));

        style->stroke.value.href = new SPPaintServerReference(document);
        style->stroke.value.href->changedSignal().connect(sigc::bind(sigc::ptr_fun(sp_style_stroke_paint_server_ref_changed), style));
    }

    style->text = text;
    style->text_private = text_private;

    style->text->font_specification.set = FALSE;
    style->text->font.set = FALSE;
    style->text->font_family.set = FALSE;

    style->font_size.set = FALSE;
    style->font_size.inherit = FALSE;
    style->font_size.type = SP_FONT_SIZE_LITERAL;
    style->font_size.unit = 0;
    style->font_size.literal = SP_CSS_FONT_SIZE_MEDIUM;
    style->font_size.value = 12.0;
    style->font_size.computed = 12.0;
    style->font_style.set = FALSE;
    style->font_style.inherit = FALSE;
    style->font_style.value = style->font_style.computed = SP_CSS_FONT_STYLE_NORMAL;
    style->font_variant.set = FALSE;
    style->font_variant.inherit = FALSE;
    style->font_variant.value = style->font_variant.computed = SP_CSS_FONT_VARIANT_NORMAL;
    style->font_weight.set = FALSE;
    style->font_weight.inherit = FALSE;
    style->font_weight.value = SP_CSS_FONT_WEIGHT_NORMAL;
    style->font_weight.computed = SP_CSS_FONT_WEIGHT_400;
    style->font_stretch.set = FALSE;
    style->font_stretch.inherit = FALSE;
    style->font_stretch.value = style->font_stretch.computed = SP_CSS_FONT_STRETCH_NORMAL;

    /* text */
    style->text_indent.set = FALSE;
    style->text_indent.inherit = FALSE;
    style->text_indent.unit = SP_CSS_UNIT_NONE;
    style->text_indent.computed = 0.0;

    style->text_align.set = FALSE;
    style->text_align.inherit = FALSE;
    style->text_align.value = style->text_align.computed = SP_CSS_TEXT_ALIGN_START;

    style->text_decoration_line.set          = FALSE;
    style->text_decoration_line.inherit      = FALSE;
    style->text_decoration_line.underline    = FALSE;
    style->text_decoration_line.overline     = FALSE;
    style->text_decoration_line.line_through = FALSE;
    style->text_decoration_line.blink        = FALSE;

    style->text_decoration_style.set      = FALSE;
    style->text_decoration_style.inherit  = FALSE;
    style->text_decoration_style.solid    = FALSE;
    style->text_decoration_style.isdouble = FALSE;
    style->text_decoration_style.dotted   = FALSE;
    style->text_decoration_style.dashed   = FALSE;
    style->text_decoration_style.wavy     = FALSE;

    style->text_decoration_color.clear();

    style->line_height.set = FALSE;
    style->line_height.inherit = FALSE;
    style->line_height.unit = SP_CSS_UNIT_PERCENT;
    style->line_height.normal = TRUE;
    style->line_height.value = style->line_height.computed = 1.0;

    style->letter_spacing.set = FALSE;
    style->letter_spacing.inherit = FALSE;
    style->letter_spacing.unit = SP_CSS_UNIT_NONE;
    style->letter_spacing.normal = TRUE;
    style->letter_spacing.value = style->letter_spacing.computed = 0.0;

    style->word_spacing.set = FALSE;
    style->word_spacing.inherit = FALSE;
    style->word_spacing.unit = SP_CSS_UNIT_NONE;
    style->word_spacing.normal = TRUE;
    style->word_spacing.value = style->word_spacing.computed = 0.0;

    style->baseline_shift.set = FALSE;
    style->baseline_shift.inherit = FALSE;
    style->baseline_shift.type = SP_BASELINE_SHIFT_LITERAL;
    style->baseline_shift.unit = SP_CSS_UNIT_NONE;
    style->baseline_shift.literal = SP_CSS_BASELINE_SHIFT_BASELINE; 
    style->baseline_shift.value = 0.0;
    style->baseline_shift.computed = 0.0;

    style->text_transform.set = FALSE;
    style->text_transform.inherit = FALSE;
    style->text_transform.value = style->text_transform.computed = SP_CSS_TEXT_TRANSFORM_NONE;

    style->direction.set = FALSE;
    style->direction.inherit = FALSE;
    style->direction.value = style->direction.computed = SP_CSS_DIRECTION_LTR;

    style->block_progression.set = FALSE;
    style->block_progression.inherit = FALSE;
    style->block_progression.value = style->block_progression.computed = SP_CSS_BLOCK_PROGRESSION_TB;

    style->writing_mode.set = FALSE;
    style->writing_mode.inherit = FALSE;
    style->writing_mode.value = style->writing_mode.computed = SP_CSS_WRITING_MODE_LR_TB;

    style->text_anchor.set = FALSE;
    style->text_anchor.inherit = FALSE;
    style->text_anchor.value = style->text_anchor.computed = SP_CSS_TEXT_ANCHOR_START;

    style->clip_set = FALSE;
    style->color_set = FALSE;
    style->cursor_set = FALSE;
    style->overflow_set  = FALSE;
    style->clip_path_set = FALSE;
    style->mask_set = FALSE;

    style->clip_rule.set = FALSE;
    style->clip_rule.inherit = FALSE;
    style->clip_rule.value = style->clip_rule.computed = SP_WIND_RULE_NONZERO;

    style->opacity.set = FALSE;
    style->opacity.inherit = FALSE;
    style->opacity.value = SP_SCALE24_MAX;
    style->visibility.set = FALSE;
    style->visibility.inherit = FALSE;
    style->visibility.value = style->visibility.computed = SP_CSS_VISIBILITY_VISIBLE;
    style->display.set = FALSE;
    style->display.inherit = FALSE;
    style->display.value = style->display.computed = SP_CSS_DISPLAY_INLINE;
    style->overflow.set = FALSE;
    style->overflow.inherit = FALSE;
    style->overflow.value = style->overflow.computed = SP_CSS_OVERFLOW_VISIBLE;
    style->isolation.set = FALSE;
    style->isolation.inherit = FALSE;
    style->isolation.value = style->isolation.computed = SP_CSS_ISOLATION_AUTO;
    style->blend_mode.set = FALSE;
    style->blend_mode.inherit = FALSE;
    style->blend_mode.value = style->blend_mode.computed = SP_CSS_BLEND_NORMAL;

    style->color.clear();
    style->color.setColor(0.0, 0.0, 0.0);
    style->color_interpolation.set = FALSE;
    style->color_interpolation.inherit = FALSE;
    style->color_interpolation.value = style->color_interpolation.computed = SP_CSS_COLOR_INTERPOLATION_SRGB;
    style->color_interpolation_filters.set = FALSE;
    style->color_interpolation_filters.inherit = FALSE;
    style->color_interpolation_filters.value = style->color_interpolation_filters.computed = SP_CSS_COLOR_INTERPOLATION_LINEARRGB;


    style->fill.clear();
    style->fill.setColor(0.0, 0.0, 0.0);
    style->fill_opacity.set = FALSE;
    style->fill_opacity.inherit = FALSE;
    style->fill_opacity.value = SP_SCALE24_MAX;
    style->fill_rule.set = FALSE;
    style->fill_rule.inherit = FALSE;
    style->fill_rule.value = style->fill_rule.computed = SP_WIND_RULE_NONZERO;

    style->stroke.clear();
    style->stroke_opacity.set = FALSE;
    style->stroke_opacity.inherit = FALSE;
    style->stroke_opacity.value = SP_SCALE24_MAX;

    style->stroke_width.set = FALSE;
    style->stroke_width.inherit = FALSE;
    style->stroke_width.unit = SP_CSS_UNIT_NONE;
    style->stroke_width.value = style->stroke_width.computed = 1.0;

    style->stroke_linecap.set = FALSE;
    style->stroke_linecap.inherit = FALSE;
    style->stroke_linecap.value = style->stroke_linecap.computed = SP_STROKE_LINECAP_BUTT;
    style->stroke_linejoin.set = FALSE;
    style->stroke_linejoin.inherit = FALSE;
    style->stroke_linejoin.value = style->stroke_linejoin.computed = SP_STROKE_LINEJOIN_MITER;

    style->stroke_miterlimit.set = FALSE;
    style->stroke_miterlimit.inherit = FALSE;
    style->stroke_miterlimit.value = 4.0;

    style->stroke_dasharray.values.clear();
    style->stroke_dasharray.set = FALSE;
    style->stroke_dasharray.inherit = FALSE;

    style->stroke_dashoffset.value = style->stroke_dashoffset.computed = 0.0;
    style->stroke_dashoffset.set = FALSE;
    style->stroke_dashoffset.inherit = FALSE;

    for (unsigned i = SP_MARKER_LOC; i < SP_MARKER_LOC_QTY; i++) {
        g_free(style->marker[i].value);
        style->marker[i].set = FALSE;
        style->marker[i].inherit = FALSE;
        style->marker[i].data = 0;
        style->marker[i].value = NULL;
    }

    /* SVG 2 */
    style->paint_order.set = FALSE;
    style->paint_order.inherit = FALSE; // For now
    for (unsigned i = 0; i < PAINT_ORDER_LAYERS; ++i) {
        style->paint_order.layer[i] = SP_CSS_PAINT_ORDER_NORMAL;
        style->paint_order.layer_set[i] = false;
    }
    style->paint_order.value = NULL;

    style->filter.set = FALSE;
    style->filter.inherit = FALSE;
    style->filter.href = NULL;

    style->enable_background.value = SP_CSS_BACKGROUND_ACCUMULATE;
    style->enable_background.set = false;
    style->enable_background.inherit = false;

    style->filter_blend_mode.set   = style->filter_blend_mode.inherit = false;
    style->filter_blend_mode.value = style->filter_blend_mode.computed = 0;
    style->filter_gaussianBlur_deviation.set   = style->filter_gaussianBlur_deviation.inherit = false;
    style->filter_gaussianBlur_deviation.value = style->filter_gaussianBlur_deviation.computed = 0;

    style->color_rendering.set   = style->color_rendering.inherit = false;
    style->color_rendering.value = style->color_rendering.computed = SP_CSS_COLOR_RENDERING_AUTO;
    style->image_rendering.set   = style->image_rendering.inherit = false;
    style->image_rendering.value = style->image_rendering.computed = SP_CSS_IMAGE_RENDERING_AUTO;
    style->shape_rendering.set   = style->shape_rendering.inherit = false;
    style->shape_rendering.value = style->shape_rendering.computed = SP_CSS_SHAPE_RENDERING_AUTO;
    style->text_rendering.set    = style->text_rendering.inherit = false;
    style->text_rendering.value  = style->text_rendering.computed  = SP_CSS_TEXT_RENDERING_AUTO;

    style->cloned = false;
}



/**
 *
 */
static void
sp_style_read_dash(SPStyle *style, gchar const *str)
{
    /* Ref: http://www.w3.org/TR/SVG11/painting.html#StrokeDasharrayProperty */
    style->stroke_dasharray.set = TRUE;

    if (strcmp(str, "inherit") == 0) {
        style->stroke_dasharray.inherit = true;
        return;
    }
    style->stroke_dasharray.inherit = false;

    style->stroke_dasharray.values.clear();

    if (strcmp(str, "none") == 0) {
        return;
    }

    gchar *e = NULL;
    bool LineSolid = true;
    while (e != str) {
        /* TODO: Should allow <length> rather than just a unitless (px) number. */
        double number = g_ascii_strtod(str, (char **) &e);
        style->stroke_dasharray.values.push_back( number );
        if (number > 0.00000001)
            LineSolid = false;
        if (e != str) {
            str = e;
        }
        while (str && *str && !isalnum(*str)) str += 1;
    }

    if (LineSolid) {
        style->stroke_dasharray.values.clear();
    }
}


/*#########################
## SPTextStyle operations
#########################*/


/**
 * Return new SPTextStyle object with default settings.
 */
static SPTextStyle *
sp_text_style_new()
{
    SPTextStyle *ts = g_new0(SPTextStyle, 1);
    ts->refcount = 1;
    sp_text_style_clear(ts);

    ts->font_specification.value = g_strdup("sans-serif");
    ts->font.value = g_strdup("sans-serif");
    ts->font_family.value = g_strdup("sans-serif");

    return ts;
}


/**
 * Clear text style settings.
 */
static void
sp_text_style_clear(SPTextStyle *ts)
{
    ts->font_specification.set = FALSE;
    ts->font.set = FALSE;
    ts->font_family.set = FALSE;
}



/**
 * Reduce refcount of text style and possibly free it.
 */
static SPTextStyle *
sp_text_style_unref(SPTextStyle *st)
{
    st->refcount -= 1;

    if (st->refcount < 1) {
        g_free(st->font_specification.value);
        g_free(st->font.value);
        g_free(st->font_family.value);
        g_free(st);
    }

    return NULL;
}



/**
 * Return duplicate of text style.
 */
static SPTextStyle *
sp_text_style_duplicate_unset(SPTextStyle *st)
{
    SPTextStyle *nt = g_new0(SPTextStyle, 1);
    nt->refcount = 1;

    nt->font_specification.value = g_strdup(st->font_specification.value);
    nt->font.value = g_strdup(st->font.value);
    nt->font_family.value = g_strdup(st->font_family.value);

    return nt;
}



/**
 * Write SPTextStyle object into string.
 */
static guint
sp_text_style_write(gchar *p, guint const len, SPTextStyle const *const st, guint flags)
{
    gint d = 0;

    // We do not do diffing for text style
    if (flags == SP_STYLE_FLAG_IFDIFF)
        flags = SP_STYLE_FLAG_IFSET;

    d += sp_style_write_istring(p + d, len - d, "font-family", &st->font_family, NULL, flags);
    d += sp_style_write_istring(p + d, len - d, "-inkscape-font-specification", &st->font_specification, NULL, flags);
    return d;
}



/* The following sp_tyle_read_* functions ignore invalid values, as per
 * http://www.w3.org/TR/REC-CSS2/syndata.html#parsing-errors.
 *
 * [However, the SVG spec is somewhat unclear as to whether the style attribute should
 * be handled as per CSS2 rules or whether it must simply be a set of PROPERTY:VALUE
 * pairs, in which case SVG's error-handling rules
 * http://www.w3.org/TR/SVG11/implnote.html#ErrorProcessing should instead be applied.]
 */


/**
 * Set SPIFloat object from string.
 */
static void
sp_style_read_ifloat(SPIFloat *val, gchar const *str)
{
    if (!strcmp(str, "inherit")) {
        val->set = TRUE;
        val->inherit = TRUE;
    } else {
        gfloat value;
        if (sp_svg_number_read_f(str, &value)) {
            val->set = TRUE;
            val->inherit = FALSE;
            val->value = value;
        }
    }
}



/**
 * Set SPIScale24 object from string.
 */
static void
sp_style_read_iscale24(SPIScale24 *val, gchar const *str)
{
    if (!strcmp(str, "inherit")) {
        val->set = TRUE;
        val->inherit = TRUE;
    } else {
        gfloat value;
        if (sp_svg_number_read_f(str, &value)) {
            val->set = TRUE;
            val->inherit = FALSE;
            value = CLAMP(value, 0.0, 1.0);
            val->value = SP_SCALE24_FROM_FLOAT(value);
        }
    }
}

/**
 * Reads a style value and performs lookup based on the given style value enumerations.
 */
static void
sp_style_read_ienum(SPIEnum *val, gchar const *str, SPStyleEnum const *dict,
                    bool const can_explicitly_inherit)
{
    if ( can_explicitly_inherit && !strcmp(str, "inherit") ) {
        val->set = TRUE;
        val->inherit = TRUE;
    } else {
        for (unsigned i = 0; dict[i].key; i++) {
            if (!strcmp(str, dict[i].key)) {
                val->set = TRUE;
                val->inherit = FALSE;
                val->value = dict[i].value;
                /* Save copying for values not needing it */
                val->computed = val->value;
                break;
            }
        }
    }
    return;
}



/**
 * Set SPIString object from string.
 */
static void
sp_style_read_istring(SPIString *val, gchar const *str)
{
    g_free(val->value);

    if (!strcmp(str, "inherit")) {
        val->set = TRUE;
        val->inherit = TRUE;
        val->value = NULL;
    } else {
        val->set = TRUE;
        val->inherit = FALSE;
        val->value = g_strdup(str);
    }
}



/**
 * Set SPILength object from string.
 */
static void
sp_style_read_ilength(SPILength *val, gchar const *str)
{
    if (!strcmp(str, "inherit")) {
        val->set = TRUE;
        val->inherit = TRUE;
    } else {
        gdouble value;
        gchar *e;
        /** \todo fixme: Move this to standard place (Lauris) */
        value = g_ascii_strtod(str, &e);
        if ( !IS_FINITE(value) ) { // fix for bug lp:935157
            return;
        }
        if ((gchar const *) e != str) {
            /** \todo
             * Allow the number of px per inch to vary (document preferences,
             * X server or whatever).  E.g. don't fill in computed here, do
             * it at the same time as percentage units are done.
             */
            val->value = value;
            if (!*e) {
                /* Userspace */
                val->unit = SP_CSS_UNIT_NONE;
                val->computed = value;
            } else if (!strcmp(e, "px")) {
                /* Userspace */
                val->unit = SP_CSS_UNIT_PX;
                val->computed = value;
            } else if (!strcmp(e, "pt")) {
                /* Userspace / DEVICESCALE */
                val->unit = SP_CSS_UNIT_PT;
                val->computed = Inkscape::Util::Quantity::convert(value, "pt", "px");
            } else if (!strcmp(e, "pc")) {
                val->unit = SP_CSS_UNIT_PC;
                val->computed = Inkscape::Util::Quantity::convert(value, "pc", "px");
            } else if (!strcmp(e, "mm")) {
                val->unit = SP_CSS_UNIT_MM;
                val->computed = Inkscape::Util::Quantity::convert(value, "mm", "px");
            } else if (!strcmp(e, "cm")) {
                val->unit = SP_CSS_UNIT_CM;
                val->computed = Inkscape::Util::Quantity::convert(value, "cm", "px");
            } else if (!strcmp(e, "in")) {
                val->unit = SP_CSS_UNIT_IN;
                val->computed = Inkscape::Util::Quantity::convert(value, "in", "px");
            } else if (!strcmp(e, "em")) {
                /* EM square */
                val->unit = SP_CSS_UNIT_EM;
                val->computed = value * SP_CSS_FONT_SIZE_DEFAULT;
            } else if (!strcmp(e, "ex")) {
                /* ex square */
                val->unit = SP_CSS_UNIT_EX;
                val->computed = value * 0.5 * SP_CSS_FONT_SIZE_DEFAULT;
            } else if (!strcmp(e, "%")) {
                /* Percentage */
                val->unit = SP_CSS_UNIT_PERCENT;
                val->value = value * 0.01;
            } else {
                /* Invalid */
                return;
            }
            val->set = TRUE;
            val->inherit = FALSE;
        }
    }
}

/**
 * Set SPILengthOrNormal object from string.
 */
static void
sp_style_read_ilengthornormal(SPILengthOrNormal *val, gchar const *str)
{
    if (!strcmp(str, "normal")) {
        val->set = TRUE;
        val->inherit = FALSE;
        val->normal = TRUE;
        val->unit = SP_CSS_UNIT_NONE;
        val->value = val->computed = 0.0;
    } else {
        SPILength length;
        sp_style_read_ilength(&length, str);
        val->set = length.set;
        val->inherit = length.inherit;
        val->normal = FALSE;
        val->unit = length.unit;
        val->value = length.value;
        val->computed = length.computed;
    }
}

/**
 * Set SPIPaintOrder object from string.
 */
static void
sp_style_read_ipaintorder(SPIPaintOrder *val, gchar const *str)
{
    g_free(val->value);

    if (!strcmp(str, "inherit")) {
        // NEED TO CHECK FINAL SPEC
        val->set = TRUE;
        val->inherit = TRUE;
        val->value = NULL;
    } else {
        val->set = TRUE;
        val->inherit = FALSE;
        val->value = g_strdup(str);

        if (!strcmp(str, "normal")) {
            val->layer[0] = SP_CSS_PAINT_ORDER_NORMAL;
            val->layer_set[0] = true;
        } else {
            // This certainly can be done more efficiently
            gchar** c = g_strsplit(str, " ", PAINT_ORDER_LAYERS + 1);
            bool used[3] = {false, false, false};
            unsigned int i = 0;
            for( ; i < PAINT_ORDER_LAYERS; ++i ) {
                if( c[i] ) {
                    val->layer_set[i] = false;
                    if( !strcmp( c[i], "fill")) {
                        val->layer[i] = SP_CSS_PAINT_ORDER_FILL;
                        val->layer_set[i] = true;
                        used[0] = true;
                    } else if( !strcmp( c[i], "stroke")) {
                        val->layer[i] = SP_CSS_PAINT_ORDER_STROKE;
                        val->layer_set[i] = true;
                        used[1] = true;
                    } else if( !strcmp( c[i], "markers")) {
                        val->layer[i] = SP_CSS_PAINT_ORDER_MARKER;
                        val->layer_set[i] = true;
                        used[2] = true;
                    } else {
                        break;
                    }
                } else {
                    break;
                }
            }
            g_strfreev(c);

            // Fill out rest of the layers using the default order
            if( !used[0] && i < PAINT_ORDER_LAYERS ) {
                val->layer[i] = SP_CSS_PAINT_ORDER_FILL;
                val->layer_set[i] = false;
                ++i;
            }
            if( !used[1] && i < PAINT_ORDER_LAYERS ) {
                val->layer[i] = SP_CSS_PAINT_ORDER_STROKE;
                val->layer_set[i] = false;
                ++i;
            }
            if( !used[2] && i < PAINT_ORDER_LAYERS ) {
                val->layer[i] = SP_CSS_PAINT_ORDER_MARKER;
                val->layer_set[i] = false;
            }
        }
    }
}



/**
 * Set SPITextDecoration object from string.
 */
static void
sp_style_read_itextdecoration(SPITextDecorationLine *line, SPITextDecorationStyle *style, SPIPaint *color, gchar const *str){
    sp_style_read_itextdecorationLine(line, str);   // scans all tokens for line types
    sp_style_read_itextdecorationStyle(style, str); // scans all tokens for style types
    // the color routine must be fed one token at a time - if multiple colors are found the LAST one is used
    const gchar *hstr = str;
    while (1) {
        if (*str == ' ' || *str == ',' || *str == '\0'){
            int slen = str - hstr;
            gchar *frag = g_strndup(hstr,slen+1); // only send one piece at a time, since keywords may be intermixed
            sp_style_read_itextdecorationColor(color, frag);
            g_free(frag);
            if(color->set)break;
            if(*str == '\0')break;
            hstr = str + 1;
        }
        str++;
    }
}

/**
 * Set SPITextDecorationLine object from string.
 * returns true if there was a match, false otherwise
 */
static void
sp_style_read_itextdecorationLine(SPITextDecorationLine *line, gchar const *str){
    if (!strcmp(str, "inherit")) {
        line->set          = true;
        line->inherit      = true;
    } else if (!strcmp(str, "none")) {
        line->set          = true;
        line->inherit      = false;
        line->underline    = false;
        line->overline     = false;
        line->line_through = false;
        line->blink        = false;
    } else {
        bool found_one          = false;
        bool hit_one            = false;

        // CSS 2 keywords
        bool found_underline    = false;
        bool found_overline     = false;
        bool found_line_through = false;
        bool found_blink        = false;

        // this method ignores inlineid keys and extra delimiters, so " ,,, blink hello" will set blink and ignore hello
        const gchar *hstr = str;
        while (1) {
            if (*str == ' ' || *str == ',' || *str == '\0'){
                int slen = str - hstr;
                // CSS 2 keywords
                while(1){ // not really a loop, used to avoid a goto
                    hit_one = true; // most likely we will
                    if ((slen ==  9) && strneq(hstr, "underline",    slen)){  found_underline    = true; break; }
                    if ((slen ==  8) && strneq(hstr, "overline",     slen)){  found_overline     = true; break; }
                    if ((slen == 12) && strneq(hstr, "line-through", slen)){  found_line_through = true; break; }
                    if ((slen ==  5) && strneq(hstr, "blink",        slen)){  found_blink        = true; break; }
                    if ((slen ==  4) && strneq(hstr, "none",         slen)){                             break; }
                    
                    hit_one = false; // whatever this thing is, we do not recognize it
                    break;
                }
                found_one |= hit_one;
                if(*str == '\0')break;
                hstr = str + 1;
            }
            str++;
        }
        if (found_one) {
            line->set          = true;
            line->inherit      = false;
            line->underline    = found_underline;
            line->overline     = found_overline;
            line->line_through = found_line_through;
            line->blink        = found_blink;
        }
        else {
            line->set          = false;
            line->inherit      = false;
        }
    }
}

/**
 * Set SPITextDecorationStyle object from string.
 * returns true if there was a match, false otherwise
*/
static void
sp_style_read_itextdecorationStyle(SPITextDecorationStyle *style, gchar const *str){
    if (!strcmp(str, "inherit")) {
        style->set         = true;
        style->inherit     = true;
    } else if (!strcmp(str, "none")) {
        style->set         = true;
        style->inherit     = false;
        style->solid       = false;
        style->isdouble    = false;
        style->dotted      = false;
        style->dashed      = false;
        style->wavy        = false;
    } else {
        // note, these are CSS 3 keywords
        bool found_solid        = false;
        bool found_double       = false;
        bool found_dotted       = false;
        bool found_dashed       = false;
        bool found_wavy         = false;
        bool found_one          = false;
        
        // this method ignores inlineid keys and extra delimiters, so " ,,, style hello" will set style and ignore hello
        // if more than one style is present, the first is used
        const gchar *hstr = str;
        while (1) {
            if (*str == ' ' || *str == ',' || *str == '\0'){
                int slen = str - hstr;
                if (     (slen ==  5) && strneq(hstr, "solid",        slen)){  found_solid  = true; found_one = true; break; }
                else if ((slen ==  6) && strneq(hstr, "double",       slen)){  found_double = true; found_one = true; break; }
                else if ((slen ==  6) && strneq(hstr, "dotted",       slen)){  found_dotted = true; found_one = true; break; }
                else if ((slen ==  6) && strneq(hstr, "dashed",       slen)){  found_dashed = true; found_one = true; break; }
                else if ((slen ==  4) && strneq(hstr, "wavy",         slen)){  found_wavy   = true; found_one = true; break; }
                if(*str == '\0')break; // nothing more to test
                hstr = str + 1;
            }
            str++;
        }
        if(found_one){
            style->set         = true;
            style->inherit     = false;
            style->solid       = found_solid;
            style->isdouble    = found_double;
            style->dotted      = found_dotted;
            style->dashed      = found_dashed;
            style->wavy        = found_wavy;
        }
        else {
            style->set         = false;
            style->inherit     = false;
        }
    }
}

/**
 * Set SPIPaint object from string.
 */
static void
sp_style_read_itextdecorationColor(SPIPaint *color, gchar const *str){
    sp_style_read_icolor(color, str, NULL,NULL);
}


/**
 * Set SPIPaint object from string containing an integer value.
 * \param document Ignored
 */
static void
sp_style_read_icolor(SPIPaint *paint, gchar const *str, SPStyle *style, SPDocument *document)
{
    (void)style; // TODO
    (void)document; // TODO
    paint->currentcolor = FALSE;  /* currentColor not a valid <color>. */
    if (!strcmp(str, "inherit")) {
        paint->set = TRUE;
        paint->inherit = TRUE;
    } else {
        paint->inherit = FALSE;
        guint32 const rgb0 = sp_svg_read_color(str, 0xff);
        if (rgb0 != 0xff) {
            paint->setColor(rgb0);
            paint->set = TRUE;
        }
    }
}


/**
 * Set SPIPaint object from string.
 *
 * \pre paint == \&style.fill || paint == \&style.stroke.
 */
void SPIPaint::read( gchar const *str, SPStyle &style, SPDocument *document )
{
    while (g_ascii_isspace(*str)) {
        ++str;
    }

    clear();

    if (streq(str, "inherit")) {
        set = TRUE;
        inherit = TRUE;
    } else {
        if ( strneq(str, "url", 3) ) {
            gchar *uri = extract_uri( str, &str );
            while ( g_ascii_isspace(*str) ) {
                ++str;
            }
            // TODO check on and comment the comparrison "paint != &style->color".
            if ( uri && *uri && (this != &style.color) ) {
                set = TRUE;

                // it may be that this style's SPIPaint has not yet created its URIReference;
                // now that we have a document, we can create it here
                if (!value.href && document) {
                    value.href = new SPPaintServerReference(document);
                    value.href->changedSignal().connect(sigc::bind(sigc::ptr_fun((this == &style.fill)? sp_style_fill_paint_server_ref_changed : sp_style_stroke_paint_server_ref_changed), &style));
                }

                // TODO check what this does in light of move away from union
                sp_style_set_ipaint_to_uri_string (&style, this, uri);
            }
            g_free( uri );
        }

        if (streq(str, "currentColor") && (this != &style.color)) {
            set = TRUE;
            currentcolor = TRUE;
        } else if (streq(str, "none") && (this != &style.color)) {
            set = TRUE;
            noneSet = TRUE;
        } else {
            guint32 const rgb0 = sp_svg_read_color(str, &str, 0xff);
            if (rgb0 != 0xff) {
                setColor( rgb0 );
                set = TRUE;

                while (g_ascii_isspace(*str)) {
                    ++str;
                }
                if (strneq(str, "icc-color(", 10)) {
                    SVGICCColor* tmp = new SVGICCColor();
                    if ( ! sp_svg_read_icc_color( str, &str, tmp ) ) {
                        delete tmp;
                        tmp = 0;
                    }
                    value.color.icc = tmp;
                }
            }
        }
    }
}



/**
 * Set SPIFontSize object from string.
 */
static void
sp_style_read_ifontsize(SPIFontSize *val, gchar const *str)
{
    if (!strcmp(str, "inherit")) {
        val->set = TRUE;
        val->inherit = TRUE;
    } else if ((*str == 'x') || (*str == 's') || (*str == 'm') || (*str == 'l')) {
        // xx-small, x-small, etc.
        for (unsigned i = 0; enum_font_size[i].key; i++) {
            if (!strcmp(str, enum_font_size[i].key)) {
                val->set = TRUE;
                val->inherit = FALSE;
                val->type = SP_FONT_SIZE_LITERAL;
                val->literal = enum_font_size[i].value;
                return;
            }
        }
        /* Invalid */
        return;
    } else {
        SPILength length;
        length.set = FALSE;
        sp_style_read_ilength(&length, str);
        if( length.set ) {
            val->set      = TRUE;
            val->inherit  = length.inherit;
            val->unit     = length.unit;
            val->value    = length.value;
            val->computed = length.computed;
            if( val->unit == SP_CSS_UNIT_PERCENT ) {
                val->type = SP_FONT_SIZE_PERCENTAGE;
            } else {
                val->type = SP_FONT_SIZE_LENGTH;
            }
        }
        return; 
    }
}


/**
 * Set SPIBaselineShift object from string.
 */
static void
sp_style_read_ibaselineshift(SPIBaselineShift *val, gchar const *str)
{
    if (!strcmp(str, "inherit")) {
        val->set = TRUE;
        val->inherit = TRUE;
    } else if ((*str == 'b') || (*str == 's')) {
        // baseline or sub or super
        for (unsigned i = 0; enum_baseline_shift[i].key; i++) {
            if (!strcmp(str, enum_baseline_shift[i].key)) {
                val->set = TRUE;
                val->inherit = FALSE;
                val->type = SP_BASELINE_SHIFT_LITERAL;
                val->literal = enum_baseline_shift[i].value;
                return;
            }
        }
        /* Invalid */
        return;
    } else {
        SPILength length;
        sp_style_read_ilength(&length, str);
        val->set      = length.set;
        val->inherit  = length.inherit;
        val->unit     = length.unit;
        val->value    = length.value;
        val->computed = length.computed;
        if( val->unit == SP_CSS_UNIT_PERCENT ) {
            val->type = SP_BASELINE_SHIFT_PERCENTAGE;
        } else {
            val->type = SP_BASELINE_SHIFT_LENGTH;
        }
        return;
    }
}


/**
 * Set SPIFilter object from string.
 */
static void
sp_style_read_ifilter(gchar const *str, SPStyle * style, SPDocument *document)
{
    SPIFilter *f = &(style->filter);
    /* Try all possible values: inherit, none, uri */
    if (streq(str, "inherit")) {
        f->set = TRUE;
        f->inherit = TRUE;
        if (f->href){
            if (f->href->getObject()){
                f->href->detach();
            }
        }
    } else if(streq(str, "none")) {
        f->set = TRUE;
        f->inherit = FALSE;
        if (f->href){
            if (f->href->getObject()){
                f->href->detach();
            }
        }
    } else if (strneq(str, "url", 3)) {
        char *uri = extract_uri(str);
        if(uri == NULL || uri[0] == '\0') {
            g_warning("Specified filter url is empty");
            f->set = TRUE;
            f->inherit = FALSE;
            return;
        }
        f->set = TRUE;
        f->inherit = FALSE;
        if (f->href){
            if (f->href->getObject()){
                f->href->detach();
            }
        }

        // it may be that this style has not yet created its SPFilterReference;
        // now that we have a document, we can create it here
        if (!f->href && document) {
            f->href = new SPFilterReference(document);
            f->href->changedSignal().connect(sigc::bind(sigc::ptr_fun(sp_style_filter_ref_changed), style));
        }

        try {
            f->href->attach(Inkscape::URI(uri));
        } catch (Inkscape::BadURIException &e) {
            g_warning("%s", e.what());
            f->href->detach();
        }
        g_free (uri);

    } else {
        /* We shouldn't reach this if SVG input is well-formed */
        f->set = FALSE;
        f->inherit = FALSE;
        if (f->href){
            if (f->href->getObject()){
                f->href->detach();
            }
        }
    }
}

/**
 * Set SPIEnum object from repr attribute.
 */
static void
sp_style_read_penum(SPIEnum *val, Inkscape::XML::Node *repr,
                    gchar const *key, SPStyleEnum const *dict,
                    bool const can_explicitly_inherit)
{
    gchar const *str = repr->attribute(key);
    if (str) {
        sp_style_read_ienum(val, str, dict, can_explicitly_inherit);
    }
}



/**
 * Set SPILength object from repr attribute.
 */
static void
sp_style_read_plength(SPILength *val, Inkscape::XML::Node *repr, gchar const *key)
{
    gchar const *str = repr->attribute(key);
    if (str) {
        sp_style_read_ilength(val, str);
    }
}

/**
 * Set SPIFontSize object from repr attribute.
 */
static void
sp_style_read_pfontsize(SPIFontSize *val, Inkscape::XML::Node *repr, gchar const *key)
{
    gchar const *str = repr->attribute(key);
    if (str) {
        sp_style_read_ifontsize(val, str);
    }
}


/**
 * Set SPIBaselineShift object from repr attribute.
 */
static void
sp_style_read_pbaselineshift(SPIBaselineShift *val, Inkscape::XML::Node *repr, gchar const *key)
{
    gchar const *str = repr->attribute(key);
    if (str) {
        sp_style_read_ibaselineshift(val, str);
    }
}


/**
 * Set SPIFloat object from repr attribute.
 */
static void
sp_style_read_pfloat(SPIFloat *val, Inkscape::XML::Node *repr, gchar const *key)
{
    gchar const *str = repr->attribute(key);
    if (str) {
        sp_style_read_ifloat(val, str);
    }
}


/**
 * Write SPIFloat object into string.
 */
static gint
sp_style_write_ifloat(gchar *p, gint const len, gchar const *const key,
                      SPIFloat const *const val, SPIFloat const *const base, guint const flags)
{
    Inkscape::CSSOStringStream os;

    if ((flags & SP_STYLE_FLAG_ALWAYS)
        || ((flags & SP_STYLE_FLAG_IFSET) && val->set)
        || ((flags & SP_STYLE_FLAG_IFDIFF) && val->set
            && (!base->set || (val->value != base->value))))
    {
        if (val->inherit) {
            return g_snprintf(p, len, "%s:inherit;", key);
        } else {
            os << key << ":" << val->value << ";";
            return g_strlcpy(p, os.str().c_str(), len);
        }
    }
    return 0;
}


/**
 * Write SPIScale24 object into string.
 */
static gint
sp_style_write_iscale24(gchar *p, gint const len, gchar const *const key,
                        SPIScale24 const *const val, SPIScale24 const *const base,
                        guint const flags)
{
    Inkscape::CSSOStringStream os;

    if ((flags & SP_STYLE_FLAG_ALWAYS)
        || ((flags & SP_STYLE_FLAG_IFSET) && val->set)
        || ((flags & SP_STYLE_FLAG_IFDIFF) && val->set
            && (!base->set || (val->value != base->value))))
    {
        if (val->inherit) {
            return g_snprintf(p, len, "%s:inherit;", key);
        } else {
            os << key << ":" << SP_SCALE24_TO_FLOAT(val->value) << ";";
            return g_strlcpy(p, os.str().c_str(), len);
        }
    }
    return 0;
}


/**
 * Write SPIEnum object into string.
 */
static gint
sp_style_write_ienum(gchar *p, gint const len, gchar const *const key,
                     SPStyleEnum const *const dict,
                     SPIEnum const *const val, SPIEnum const *const base, guint const flags)
{
    if ((flags & SP_STYLE_FLAG_ALWAYS)
        || ((flags & SP_STYLE_FLAG_IFSET) && val->set)
        || ((flags & SP_STYLE_FLAG_IFDIFF) && val->set
            && (!base->set || (val->computed != base->computed))))
    {
        if (val->inherit) {
            return g_snprintf(p, len, "%s:inherit;", key);
        }
        for (unsigned i = 0; dict[i].key; i++) {
            if (dict[i].value == static_cast< gint > (val->value) ) {
                return g_snprintf(p, len, "%s:%s;", key, dict[i].key);
            }
        }
    }
    return 0;
}



/**
 * Write SPIString object into string.
 */
static gint
sp_style_write_istring(gchar *p, gint const len, gchar const *const key,
                       SPIString const *const val, SPIString const *const base, guint const flags)
{
    gint res = 0;
    if ((flags & SP_STYLE_FLAG_ALWAYS)
        || ((flags & SP_STYLE_FLAG_IFSET) && val->set)
        || ((flags & SP_STYLE_FLAG_IFDIFF) && val->set
            && (!base->set || strcmp(val->value, base->value))))
    {
        if (val->inherit) {
            res = g_snprintf(p, len, "%s:inherit;", key);
        } else {
            Glib::ustring val_quoted = css2_escape_quote(val->value);
            if (~val_quoted.empty()) {
                res = g_snprintf(p, len, "%s:%s;", key, val_quoted.c_str());
            }
        }
    }
    return res;
}


/**
 *
 */
static bool
sp_length_differ(SPILength const *const a, SPILength const *const b)
{
    if (a->unit != b->unit) {
        if (a->unit == SP_CSS_UNIT_EM) return true;
        if (a->unit == SP_CSS_UNIT_EX) return true;
        if (a->unit == SP_CSS_UNIT_PERCENT) return true;
        if (b->unit == SP_CSS_UNIT_EM) return true;
        if (b->unit == SP_CSS_UNIT_EX) return true;
        if (b->unit == SP_CSS_UNIT_PERCENT) return true;
    }

    return (a->computed != b->computed);
}



/**
 * Write SPILength object into string.
 */
static gint
sp_style_write_ilength(gchar *p, gint const len, gchar const *const key,
                       SPILength const *const val, SPILength const *const base, guint const flags)
{
    Inkscape::CSSOStringStream os;

    if ((flags & SP_STYLE_FLAG_ALWAYS)
        || ((flags & SP_STYLE_FLAG_IFSET) && val->set)
        || ((flags & SP_STYLE_FLAG_IFDIFF) && val->set
            && (!base->set || sp_length_differ(val, base))))
    {
        if (val->inherit) {
            return g_snprintf(p, len, "%s:inherit;", key);
        } else {
            switch (val->unit) {
                case SP_CSS_UNIT_NONE:
                    os << key << ":" << val->computed << ";";
                    return g_strlcpy(p, os.str().c_str(), len);
                    break;
                case SP_CSS_UNIT_PX:
                    os << key << ":" << val->computed << "px;";
                    return g_strlcpy(p, os.str().c_str(), len);
                    break;
                case SP_CSS_UNIT_PT:
                    os << key << ":" << Inkscape::Util::Quantity::convert(val->computed, "px", "pt") << "pt;";
                    return g_strlcpy(p, os.str().c_str(), len);
                    break;
                case SP_CSS_UNIT_PC:
                    os << key << ":" << Inkscape::Util::Quantity::convert(val->computed, "px", "pc") << "pc;";
                    return g_strlcpy(p, os.str().c_str(), len);
                    break;
                case SP_CSS_UNIT_MM:
                    os << key << ":" << Inkscape::Util::Quantity::convert(val->computed, "px", "mm") << "mm;";
                    return g_strlcpy(p, os.str().c_str(), len);
                    break;
                case SP_CSS_UNIT_CM:
                    os << key << ":" << Inkscape::Util::Quantity::convert(val->computed, "px", "cm") << "cm;";
                    return g_strlcpy(p, os.str().c_str(), len);
                    break;
                case SP_CSS_UNIT_IN:
                    os << key << ":" << Inkscape::Util::Quantity::convert(val->computed, "px", "in") << "in;";
                    return g_strlcpy(p, os.str().c_str(), len);
                    break;
                case SP_CSS_UNIT_EM:
                    os << key << ":" << val->value << "em;";
                    return g_strlcpy(p, os.str().c_str(), len);
                    break;
                case SP_CSS_UNIT_EX:
                    os << key << ":" << val->value << "ex;";
                    return g_strlcpy(p, os.str().c_str(), len);
                    break;
                case SP_CSS_UNIT_PERCENT:
                    os << key << ":" << (val->value * 100.0) << "%;";
                    return g_strlcpy(p, os.str().c_str(), len);
                    break;
                default:
                    /* Invalid */
                    break;
            }
        }
    }
    return 0;
}


/**
 *
 */
static bool
sp_lengthornormal_differ(SPILengthOrNormal const *const a, SPILengthOrNormal const *const b)
{
    if (a->normal != b->normal) return true;
    if (a->normal) return false;

    if (a->unit != b->unit) {
        if (a->unit == SP_CSS_UNIT_EM) return true;
        if (a->unit == SP_CSS_UNIT_EX) return true;
        if (a->unit == SP_CSS_UNIT_PERCENT) return true;
        if (b->unit == SP_CSS_UNIT_EM) return true;
        if (b->unit == SP_CSS_UNIT_EX) return true;
        if (b->unit == SP_CSS_UNIT_PERCENT) return true;
    }

    return (a->computed != b->computed);
}

/**
 * Write SPILengthOrNormal object into string.
 */
static gint
sp_style_write_ilengthornormal(gchar *p, gint const len, gchar const *const key,
                               SPILengthOrNormal const *const val,
                               SPILengthOrNormal const *const base,
                               guint const flags)
{
    if ((flags & SP_STYLE_FLAG_ALWAYS)
        || ((flags & SP_STYLE_FLAG_IFSET) && val->set)
        || ((flags & SP_STYLE_FLAG_IFDIFF) && val->set
            && (!base->set || sp_lengthornormal_differ(val, base))))
    {
        if (val->normal) {
            return g_snprintf(p, len, "%s:normal;", key);
        } else {
            SPILength length;
            length.set = val->set;
            length.inherit = val->inherit;
            length.unit = val->unit;
            length.value = val->value;
            length.computed = val->computed;
            return sp_style_write_ilength(p, len, key, &length, NULL, SP_STYLE_FLAG_ALWAYS);
        }
    }
    return 0;
}

/**
 * Write SPIDashArray object into string.
 */
static gint
sp_style_write_idasharray(gchar *p, gint const len, gchar const *const /*key*/,
                          SPIDashArray const *const val, SPIDashArray const *const base, guint const flags)
{
    if ((flags & SP_STYLE_FLAG_ALWAYS)
        || ((flags & SP_STYLE_FLAG_IFSET) && val->set)
        || ((flags & SP_STYLE_FLAG_IFDIFF) && val->set
            && (!base->set || (val->values != base->values))))
    {
        if (val->inherit) {
            return g_snprintf(p, len, "stroke-dasharray:inherit;");
        } else if ( !val->values.empty() ) {
            Inkscape::CSSOStringStream os;
            os << "stroke-dasharray:";
            for (unsigned i = 0; i < val->values.size(); i++) {
                if (i) {
                    os << ", ";
                }
                os << val->values[i];
            }
            os << ";";
            return g_strlcpy(p, os.str().c_str(), len);
        } else {
            return g_snprintf(p, len, "stroke-dasharray:none;");
        }
    }
    return 0;
}


/**
 *
 */
static bool
sp_textdecorationLine_differ(SPITextDecorationLine const *const a, SPITextDecorationLine const *const b)
{
    return(     (a->underline    != b->underline   )
            ||  (a->overline     != b->overline    )
            ||  (a->line_through != b->line_through)
            ||  (a->blink        != b->blink       )
    );
}

/**
 *
 */
static bool
sp_textdecorationStyle_differ(SPITextDecorationStyle const *const a, SPITextDecorationStyle const *const b)
{
    return(     (a->solid    != b->solid    )
            ||  (a->isdouble != b->isdouble )
            ||  (a->dotted   != b->dotted   )
            ||  (a->dashed   != b->dashed   )
            ||  (a->wavy     != b->wavy     )
    );
}

/**
 *
 */
static bool
sp_textdecorationColor_differ(SPIPaint const *const a, SPIPaint const *const b)
{
    bool status =   (a->isPaintserver() == b->isPaintserver()) && 
                    (a->colorSet == b->colorSet) &&
                    (a->currentcolor == b->currentcolor);
    return(status);
}

/**
 * Write SPITextDecoration object into string.
 */
static gint
sp_style_write_itextdecoration(gchar *p, gint const len, gchar const *const key,
                               SPITextDecorationLine  const *const line,
                               SPITextDecorationStyle const *const style,
                               SPIPaint               const *const color,
                               SPITextDecorationLine  const *const baseLine,
                               SPITextDecorationStyle const *const baseStyle,
                               SPIPaint               const *const baseColor,
                               guint const flags)
{
    Inkscape::CSSOStringStream os;

    if (    (flags & SP_STYLE_FLAG_ALWAYS)
        || ((flags & SP_STYLE_FLAG_IFSET)  && line->set)
        || ((flags & SP_STYLE_FLAG_IFDIFF) && line->set
                && (   !baseLine->set  || sp_textdecorationLine_differ(line,   baseLine)))
        || ((flags & SP_STYLE_FLAG_IFSET)  && style->set)
        || ((flags & SP_STYLE_FLAG_IFDIFF) && style->set
                && (   !baseStyle->set  || sp_textdecorationStyle_differ(style, baseStyle)))
        || ((flags & SP_STYLE_FLAG_IFSET)  && color->set)
        || ((flags & SP_STYLE_FLAG_IFDIFF) && color->set
                && (   !baseColor->set  || sp_textdecorationColor_differ(color, baseColor)))
    ){
        os << key << ":";
        if (line->inherit || style->inherit || color->inherit) {
            os << " inherit";
        }
        else if (line->underline || line->overline || line->line_through || line->blink) {
            if (line->underline)         os << " underline";
            if (line->overline)          os << " overline";
            if (line->line_through)      os << " line-through";
            if (line->blink)             os << " blink";

            if (     style->solid)       os << " solid";
            else if (style->isdouble)    os << " double";
            else if (style->dotted)      os << " dotted";
            else if (style->dashed)      os << " dashed";
            else if (style->wavy)        os << " wavy";
            // color, if it is set, otherwise omit it 
            if(color->set){
                char color_buf[8];
                sp_svg_write_color(color_buf, sizeof(color_buf), color->value.color.toRGBA32( 0 ));
                os << " ";
                os << color_buf;
            } 
        }
        else {
            os << "none";
        }
        os << ";";
        return g_strlcpy(p, os.str().c_str(), len);
    }
    return 0;
}

/**
 *
 */
static bool
sp_paint_differ(SPIPaint const *const a, SPIPaint const *const b)
{
    if ( (a->isColor() != b->isColor())
         || (a->isPaintserver() != b->isPaintserver())
         || (a->set != b->set)
         || (a->currentcolor != b->currentcolor)
         || (a->inherit!= b->inherit) ) {
        return true;
    }

    // TODO refactor to allow for mixed paints (rgb() *and* url(), etc)

    if ( a->isPaintserver() ) {
        return (a->value.href == NULL || b->value.href == NULL || a->value.href->getObject() != b->value.href->getObject());
    }

    if ( a->isColor() ) {
        return !( (a->value.color == b->value.color)
                 && ((a->value.color.icc == b->value.color.icc)
                     || (a->value.color.icc && b->value.color.icc
                         && (a->value.color.icc->colorProfile == b->value.color.icc->colorProfile)
                         && (a->value.color.icc->colors == b->value.color.icc->colors))));
    /* todo: Allow for epsilon differences in iccColor->colors, e.g. changes small enough not to show up
     * in the string representation. */
    }

    return false;
}



/**
 * Write SPIPaint object into string.
 */
static gint
sp_style_write_ipaint(gchar *b, gint const len, gchar const *const key,
                      SPIPaint const *const paint, SPIPaint const *const base, guint const flags)
{
    int retval = 0;

    if ((flags & SP_STYLE_FLAG_ALWAYS)
        || ((flags & SP_STYLE_FLAG_IFSET) && paint->set)
        || ((flags & SP_STYLE_FLAG_IFDIFF) && paint->set
            && (!base->set || sp_paint_differ(paint, base))))
    {
        CSSOStringStream css;

        if (paint->inherit) {
            css << "inherit";
        } else {
            if ( paint->value.href && paint->value.href->getURI() ) {
                const gchar* uri = paint->value.href->getURI()->toString();
                css << "url(" << uri << ")";
                g_free((void *)uri);
            }

            if ( paint->noneSet ) {
                if ( !css.str().empty() ) {
                    css << " ";
                }
                css << "none";
            }

            if ( paint->currentcolor ) {
                if ( !css.str().empty() ) {
                    css << " ";
                }
                css << "currentColor";
            }

            if ( paint->colorSet && !paint->currentcolor ) {
                if ( !css.str().empty() ) {
                    css << " ";
                }
                char color_buf[8];
                sp_svg_write_color(color_buf, sizeof(color_buf), paint->value.color.toRGBA32( 0 ));
                css << color_buf;
            }

            if (paint->value.color.icc && !paint->currentcolor) {
                if ( !css.str().empty() ) {
                    css << " ";
                }
                css << "icc-color(" << paint->value.color.icc->colorProfile;
                for (vector<double>::const_iterator i(paint->value.color.icc->colors.begin()),
                         iEnd(paint->value.color.icc->colors.end());
                     i != iEnd; ++i) {
                    css << ", " << *i;
                }
                css << ')';
            }
        }

        if ( !css.str().empty() ) {
            retval = g_snprintf( b, len, "%s:%s;", key, css.str().c_str() );
        }
    }

    return retval;
}


/**
 *
 */
static bool
sp_paint_order_differ(SPIPaintOrder const *const a, SPIPaintOrder const *const b)
{
    if( (a->set != b->set) ||
        (a->inherit!= b->inherit) ) {
        return true;
    }

    // Check this works when paint-order value is 'normal'
    for (unsigned i = 0; i < PAINT_ORDER_LAYERS; ++i ) {
        if( (a->layer[i]     != b->layer[i]) ||
            (a->layer_set[i] != b->layer_set[i]) ) {
            return true;
        }
    }
    return false;
}



/**
 * Write SPIPaintOrder object into string.
 */
static gint
sp_style_write_ipaintorder(gchar *p, gint len, gchar const *key, SPIPaintOrder const *paint_order, SPIPaintOrder const *base, guint flags)
{
    int retval = 0;

    if ((flags & SP_STYLE_FLAG_ALWAYS)
        || ((flags & SP_STYLE_FLAG_IFSET) && paint_order->set)
        || ((flags & SP_STYLE_FLAG_IFDIFF) && paint_order->set
            && (!base->set || sp_paint_order_differ(paint_order, base))))
    {
        CSSOStringStream css;

        if (paint_order->inherit) {
            css << "inherit";
        } else {
            for( unsigned i = 0; i < PAINT_ORDER_LAYERS; ++i ) {
                if( paint_order->layer_set[i] == true ) {
                    switch (paint_order->layer[i]) {
                        case SP_CSS_PAINT_ORDER_NORMAL:
                            css << "normal";
                            assert( i == 0 );
                            break;
                        case SP_CSS_PAINT_ORDER_FILL:
                            if (i!=0) css << " ";
                            css << "fill";
                            break;
                        case SP_CSS_PAINT_ORDER_STROKE:
                            if (i!=0) css << " ";
                            css << "stroke";
                            break;
                        case SP_CSS_PAINT_ORDER_MARKER:
                            if (i!=0) css << " ";
                            css << "markers";
                            break;
                    }
                } else {
                    break;
                }
            }
        }

        if ( !css.str().empty() ) {
            retval = g_snprintf( p, len, "%s:%s;", key, css.str().c_str() );
        }
    }

    return retval;
}

/**
 *
 */
static bool
sp_fontsize_differ(SPIFontSize const *const a, SPIFontSize const *const b)
{
    if (a->type != b->type)
        return true;
    if (a->type == SP_FONT_SIZE_LENGTH) {
        if (a->computed != b->computed)
            return true;
    } else {
        if (a->value != b->value)
            return true;
    }
    return false;
}


/**
 * Write SPIFontSize object into string.
 */
static gint
sp_style_write_ifontsize(gchar *p, gint const len, gchar const *key,
                         SPIFontSize const *const val, SPIFontSize const *const base,
                         guint const flags)
{
    if ((flags & SP_STYLE_FLAG_ALWAYS)
        || ((flags & SP_STYLE_FLAG_IFSET) && val->set)
        || ((flags & SP_STYLE_FLAG_IFDIFF) && val->set
            && (!base->set || sp_fontsize_differ(val, base))))
    {
        if (val->inherit) {
            return g_snprintf(p, len, "%s:inherit;", key);
        } else if (val->type == SP_FONT_SIZE_LITERAL) {
            for (unsigned i = 0; enum_font_size[i].key; i++) {
                if (enum_font_size[i].value == static_cast< gint > (val->literal) ) {
                    return g_snprintf(p, len, "%s:%s;", key, enum_font_size[i].key);
                }
            }
        } else if (val->type == SP_FONT_SIZE_LENGTH) {
            Inkscape::CSSOStringStream os;
            Inkscape::Preferences *prefs = Inkscape::Preferences::get();
            int unit = prefs->getInt("/options/font/unitType", SP_CSS_UNIT_PT);
            if (prefs->getBool("/options/font/textOutputPx", true)) {
                unit = SP_CSS_UNIT_PX;
            }
            os << key << ":" << sp_style_css_size_px_to_units(val->computed, unit) << sp_style_get_css_unit_string(unit) << ";";
            return g_strlcpy(p, os.str().c_str(), len);
        } else if (val->type == SP_FONT_SIZE_PERCENTAGE) {
            Inkscape::CSSOStringStream os;
            os << key << ":" << (val->value * 100.0) << "%;";
            return g_strlcpy(p, os.str().c_str(), len);
        }
    }
    return 0;
}


/*
 * baseline-shift is relative to parent. The only time it should
 * not be written out is if it is zero (or not set).
 */
static bool
sp_baseline_shift_notzero(SPIBaselineShift const *const a )
{
    if( a->type == SP_BASELINE_SHIFT_LITERAL ) {
        if( a->literal == SP_CSS_BASELINE_SHIFT_BASELINE ) {
            return false;
        }
    } else {
        if( a->value == 0.0 ) {
            return false;
        }
    }
    return true;
}

/**
 * Write SPIBaselineShift object into string.
 */
static gint
sp_style_write_ibaselineshift(gchar *p, gint const len, gchar const *key,
                              SPIBaselineShift const *const val, SPIBaselineShift const *const base,
                              guint const flags)
{
    if ((flags & SP_STYLE_FLAG_ALWAYS)
        || ((flags & SP_STYLE_FLAG_IFSET) && val->set)
        || ((flags & SP_STYLE_FLAG_IFDIFF) && val->set
            && (!base->set || sp_baseline_shift_notzero(val) )))
    {
        if (val->inherit) {
            return g_snprintf(p, len, "%s:inherit;", key);
        } else if (val->type == SP_BASELINE_SHIFT_LITERAL) {
            for (unsigned i = 0; enum_baseline_shift[i].key; i++) {
                if (enum_baseline_shift[i].value == static_cast< gint > (val->literal) ) {
                    return g_snprintf(p, len, "%s:%s;", key, enum_baseline_shift[i].key);
                }
            }
        } else if (val->type == SP_BASELINE_SHIFT_LENGTH) {
            if( val->unit == SP_CSS_UNIT_EM || val->unit == SP_CSS_UNIT_EX ) {
                Inkscape::CSSOStringStream os;
                os << key << ":" << val->value << (val->unit == SP_CSS_UNIT_EM ? "em;" : "ex;");
                return g_strlcpy(p, os.str().c_str(), len);
            } else {
                Inkscape::CSSOStringStream os;
                os << key << ":" << val->computed << "px;";      // must specify px, see inkscape bug 1221626, mozilla bug 234789
                return g_strlcpy(p, os.str().c_str(), len);
            }
        } else if (val->type == SP_BASELINE_SHIFT_PERCENTAGE) {
            Inkscape::CSSOStringStream os;
            os << key << ":" << (val->value * 100.0) << "%;";
            return g_strlcpy(p, os.str().c_str(), len);
        }
    }
    return 0;
}



/**
 * Write SPIFilter object into string.
 */
static gint
sp_style_write_ifilter(gchar *p, gint const len, gchar const *key,
                         SPIFilter const *const val, SPIFilter const *const base,
                         guint const flags)
{
    (void)base; // TODO
    if ((flags & SP_STYLE_FLAG_ALWAYS)
        || ((flags & SP_STYLE_FLAG_IFSET) && val->set)
        || ((flags & SP_STYLE_FLAG_IFDIFF) && val->set))
    {
        if (val->inherit) {
            return g_snprintf(p, len, "%s:inherit;", key);
        } else if (val->href && val->href->getURI()) {
            gchar *uri = val->href->getURI()->toString();
            gint ret = g_snprintf(p, len, "%s:url(%s);", key, uri);
            g_free(uri);
            return ret;        
        }
    }


    return 0;
}

SPIPaint::SPIPaint() :
    set(false),
    inherit(0),
    currentcolor(0),
    colorSet(0),
    noneSet(0),
    value()
{
    value.color.set( 0 );
    value.href = 0;
}

void SPIPaint::clear()
{
    set = false;
    inherit = false;
    currentcolor = false;
    colorSet = false;
    noneSet = false;
    value.color.set( 0 );
    if (value.href){
        if (value.href->getObject()){
            value.href->detach();
        }
    }
}

/**
 * Clear filter object, and disconnect style from paintserver (if present).
 */
static void
sp_style_filter_clear(SPStyle *style)
{
    if (style->filter.href){
        if (style->filter.href->getObject()){
            style->filter.href->detach();
        }
    }
}


// FIXME: Everything below this line belongs in a different file - css-chemistry?

void
sp_style_set_property_url (SPObject *item, gchar const *property, SPObject *linked, bool recursive)
{
    Inkscape::XML::Node *repr = item->getRepr();

    if (repr == NULL) return;

    SPCSSAttr *css = sp_repr_css_attr_new();
    if (linked) {
        gchar *val = g_strdup_printf("url(#%s)", linked->getId());
        sp_repr_css_set_property(css, property, val);
        g_free(val);
    } else {
        sp_repr_css_unset_property(css, "filter");
    }

    if (recursive) {
        sp_repr_css_change_recursive(repr, css, "style");
    } else {
        sp_repr_css_change(repr, css, "style");
    }
    sp_repr_css_attr_unref(css);
}


/**
 * Clear all style property attributes in object.
 */
void
sp_style_unset_property_attrs(SPObject *o)
{
    if (!o) {
        return;
    }

    SPStyle *style = o->style;
    if (!style) {
        return;
    }

    Inkscape::XML::Node *repr = o->getRepr();
    if (!repr) {
        return;
    }

    if (style->opacity.set) {
        repr->setAttribute("opacity", NULL);
    }
    if (style->color.set) {
        repr->setAttribute("color", NULL);
    }
    if (style->color_interpolation.set) {
        repr->setAttribute("color-interpolation", NULL);
    }
    if (style->color_interpolation_filters.set) {
        repr->setAttribute("color-interpolation-filters", NULL);
    }
    if (style->fill.set) {
        repr->setAttribute("fill", NULL);
    }
    if (style->fill_opacity.set) {
        repr->setAttribute("fill-opacity", NULL);
    }
    if (style->fill_rule.set) {
        repr->setAttribute("fill-rule", NULL);
    }
    if (style->stroke.set) {
        repr->setAttribute("stroke", NULL);
    }
    if (style->stroke_width.set) {
        repr->setAttribute("stroke-width", NULL);
    }
    if (style->stroke_linecap.set) {
        repr->setAttribute("stroke-linecap", NULL);
    }
    if (style->stroke_linejoin.set) {
        repr->setAttribute("stroke-linejoin", NULL);
    }
    if (style->marker[SP_MARKER_LOC].set) {
        repr->setAttribute("marker", NULL);
    }
    if (style->marker[SP_MARKER_LOC_START].set) {
        repr->setAttribute("marker-start", NULL);
    }
    if (style->marker[SP_MARKER_LOC_MID].set) {
        repr->setAttribute("marker-mid", NULL);
    }
    if (style->marker[SP_MARKER_LOC_END].set) {
        repr->setAttribute("marker-end", NULL);
    }
    if (style->stroke_opacity.set) {
        repr->setAttribute("stroke-opacity", NULL);
    }
    if (style->stroke_dasharray.set) {
        repr->setAttribute("stroke-dasharray", NULL);
    }
    if (style->stroke_dashoffset.set) {
        repr->setAttribute("stroke-dashoffset", NULL);
    }
    if (style->paint_order.set) {
        repr->setAttribute("paint-order", NULL);
    }
    if (style->text_private && style->text->font_specification.set) {
        repr->setAttribute("-inkscape-font-specification", NULL);
    }
    if (style->text_private && style->text->font_family.set) {
        repr->setAttribute("font-family", NULL);
    }
    if (style->text_anchor.set) {
        repr->setAttribute("text-anchor", NULL);
    }
    if (style->writing_mode.set) {
        repr->setAttribute("writing_mode", NULL);
    }
    if (style->filter.set) {
        repr->setAttribute("filter", NULL);
    }
    if (style->enable_background.set) {
        repr->setAttribute("enable-background", NULL);
    }
    if (style->clip_rule.set) {
        repr->setAttribute("clip-rule", NULL);
    }
    if (style->color_rendering.set) {
        repr->setAttribute("color-rendering", NULL);
    }
    if (style->image_rendering.set) {
        repr->setAttribute("image-rendering", NULL);
    }
    if (style->shape_rendering.set) {
        repr->setAttribute("shape-rendering", NULL);
    }
    if (style->text_rendering.set) {
        repr->setAttribute("text-rendering", NULL);
    }
}

/**
 * \pre style != NULL.
 * \pre flags in {IFSET, ALWAYS}.
 */
SPCSSAttr *
sp_css_attr_from_style(SPStyle const *const style, guint const flags)
{
    g_return_val_if_fail(style != NULL, NULL);
    g_return_val_if_fail(((flags == SP_STYLE_FLAG_IFSET) ||
                          (flags == SP_STYLE_FLAG_ALWAYS)  ),
                         NULL);
    gchar *style_str = sp_style_write_string(style, flags);
    SPCSSAttr *css = sp_repr_css_attr_new();
    sp_repr_css_attr_add_from_string(css, style_str);
    g_free(style_str);
    return css;
}


/**
 * \pre object != NULL
 * \pre flags in {IFSET, ALWAYS}.
 */
SPCSSAttr *sp_css_attr_from_object(SPObject *object, guint const flags)
{
    g_return_val_if_fail(((flags == SP_STYLE_FLAG_IFSET) ||
                          (flags == SP_STYLE_FLAG_ALWAYS)  ),
                         NULL);
    SPCSSAttr * result = 0;    
    if (object->style) {
        result = sp_css_attr_from_style(object->style, flags);
    }
    return result;
}

/**
 * Unset any text-related properties
 */
SPCSSAttr *
sp_css_attr_unset_text(SPCSSAttr *css)
{
    sp_repr_css_set_property(css, "font", NULL); // not implemented yet
    sp_repr_css_set_property(css, "-inkscape-font-specification", NULL);
    sp_repr_css_set_property(css, "font-size", NULL);
    sp_repr_css_set_property(css, "font-size-adjust", NULL); // not implemented yet
    sp_repr_css_set_property(css, "font-style", NULL);
    sp_repr_css_set_property(css, "font-variant", NULL);
    sp_repr_css_set_property(css, "font-weight", NULL);
    sp_repr_css_set_property(css, "font-stretch", NULL);
    sp_repr_css_set_property(css, "font-family", NULL);
    sp_repr_css_set_property(css, "text-indent", NULL);
    sp_repr_css_set_property(css, "text-align", NULL);
    sp_repr_css_set_property(css, "text-decoration", NULL);
    sp_repr_css_set_property(css, "line-height", NULL);
    sp_repr_css_set_property(css, "letter-spacing", NULL);
    sp_repr_css_set_property(css, "word-spacing", NULL);
    sp_repr_css_set_property(css, "text-transform", NULL);
    sp_repr_css_set_property(css, "direction", NULL);
    sp_repr_css_set_property(css, "block-progression", NULL);
    sp_repr_css_set_property(css, "writing-mode", NULL);
    sp_repr_css_set_property(css, "text-anchor", NULL);
    sp_repr_css_set_property(css, "kerning", NULL); // not implemented yet
    sp_repr_css_set_property(css, "dominant-baseline", NULL); // not implemented yet
    sp_repr_css_set_property(css, "alignment-baseline", NULL); // not implemented yet
    sp_repr_css_set_property(css, "baseline-shift", NULL);

    return css;
}

static bool
is_url(char const *p)
{
    if (p == NULL)
        return false;
/** \todo
 * FIXME: I'm not sure if this applies to SVG as well, but CSS2 says any URIs
 * in property values must start with 'url('.
 */
    return (g_ascii_strncasecmp(p, "url(", 4) == 0);
}

/**
 * Unset any properties that contain URI values.
 *
 * Used for storing style that will be reused across documents when carrying
 * the referenced defs is impractical.
 */
SPCSSAttr *
sp_css_attr_unset_uris(SPCSSAttr *css)
{
// All properties that may hold <uri> or <paint> according to SVG 1.1
    if (is_url(sp_repr_css_property(css, "clip-path", NULL))) sp_repr_css_set_property(css, "clip-path", NULL);
    if (is_url(sp_repr_css_property(css, "color-profile", NULL))) sp_repr_css_set_property(css, "color-profile", NULL);
    if (is_url(sp_repr_css_property(css, "cursor", NULL))) sp_repr_css_set_property(css, "cursor", NULL);
    if (is_url(sp_repr_css_property(css, "filter", NULL))) sp_repr_css_set_property(css, "filter", NULL);
    if (is_url(sp_repr_css_property(css, "marker", NULL))) sp_repr_css_set_property(css, "marker", NULL);
    if (is_url(sp_repr_css_property(css, "marker-start", NULL))) sp_repr_css_set_property(css, "marker-start", NULL);
    if (is_url(sp_repr_css_property(css, "marker-mid", NULL))) sp_repr_css_set_property(css, "marker-mid", NULL);
    if (is_url(sp_repr_css_property(css, "marker-end", NULL))) sp_repr_css_set_property(css, "marker-end", NULL);
    if (is_url(sp_repr_css_property(css, "mask", NULL))) sp_repr_css_set_property(css, "mask", NULL);
    if (is_url(sp_repr_css_property(css, "fill", NULL))) sp_repr_css_set_property(css, "fill", NULL);
    if (is_url(sp_repr_css_property(css, "stroke", NULL))) sp_repr_css_set_property(css, "stroke", NULL);

    return css;
}

/**
 * Scale a single-value property.
 */
static void
sp_css_attr_scale_property_single(SPCSSAttr *css, gchar const *property,
                                  double ex, bool only_with_units = false)
{
    gchar const *w = sp_repr_css_property(css, property, NULL);
    if (w) {
        gchar *units = NULL;
        double wd = g_ascii_strtod(w, &units) * ex;
        if (w == units) {// nothing converted, non-numeric value
            return;
        }
        if (only_with_units && (units == NULL || *units == '\0' || *units == '%')) {
            // only_with_units, but no units found, so do nothing.
            return;
        }
        Inkscape::CSSOStringStream os;
        os << wd << units; // reattach units
        sp_repr_css_set_property(css, property, os.str().c_str());
    }
}

/**
 * Scale a list-of-values property.
 */
static void
sp_css_attr_scale_property_list(SPCSSAttr *css, gchar const *property, double ex)
{
    gchar const *string = sp_repr_css_property(css, property, NULL);
    if (string) {
        Inkscape::CSSOStringStream os;
        gchar **a = g_strsplit(string, ",", 10000);
        bool first = true;
        for (gchar **i = a; i != NULL; i++) {
            gchar *w = *i;
            if (w == NULL)
                break;
            gchar *units = NULL;
            double wd = g_ascii_strtod(w, &units) * ex;
            if (w == units) {// nothing converted, non-numeric value ("none" or "inherit"); do nothing
                g_strfreev(a);
                return;
            }
            if (!first) {
                os << ",";
            }
            os << wd << units; // reattach units
            first = false;
        }
        sp_repr_css_set_property(css, property, os.str().c_str());
        g_strfreev(a);
    }
}

/**
 * Scale any properties that may hold <length> by ex.
 */
SPCSSAttr *
sp_css_attr_scale(SPCSSAttr *css, double ex)
{
    sp_css_attr_scale_property_single(css, "baseline-shift", ex);
    sp_css_attr_scale_property_single(css, "stroke-width", ex);
    sp_css_attr_scale_property_list  (css, "stroke-dasharray", ex);
    sp_css_attr_scale_property_single(css, "stroke-dashoffset", ex);
    sp_css_attr_scale_property_single(css, "font-size", ex);
    sp_css_attr_scale_property_single(css, "kerning", ex);
    sp_css_attr_scale_property_single(css, "letter-spacing", ex);
    sp_css_attr_scale_property_single(css, "word-spacing", ex);
    sp_css_attr_scale_property_single(css, "line-height", ex, true);

    return css;
}


/**
 * Remove quotes and escapes from a string. Returned value must be g_free'd.
 * Note: in CSS (in style= and in stylesheets), unquoting and unescaping is done
 * by libcroco, our CSS parser, though it adds a new pair of "" quotes for the strings
 * it parsed for us. So this function is only used to remove those quotes and for
 * presentation attributes, without any unescaping. (XML unescaping
 * (&amp; etc) is done by XML parser.)
 */
gchar *
attribute_unquote(gchar const *val)
{
    if (val) {
        if (*val == '\'' || *val == '"') {
            int l = strlen(val);
            if (l >= 2) {
                if ( ( val[0] == '"' && val[l - 1] == '"' )  ||
                     ( val[0] == '\'' && val[l - 1] == '\'' )  ) {
                    return (g_strndup (val+1, l-2));
                }
            }
        }
    }

    return (val? g_strdup (val) : NULL);
}

/**
 * Quote and/or escape string for writing to CSS (style=). Returned value must be g_free'd.
 */
Glib::ustring css2_escape_quote(gchar const *val) {

    Glib::ustring t;
    bool quote = false;
    bool last_was_space = false;

    for (gchar const *i = val; *i; i++) {
        bool is_space = ( *i == ' ' );
        if (g_ascii_isalnum(*i) || *i=='-' || *i=='_') {
            // ASCII alphanumeric, - and _ don't require quotes
            t.push_back(*i);
        } else if ( is_space && !last_was_space ) {
            // non-consecutive spaces don't require quotes
            t.push_back(*i);
        } else if (*i=='\'') {
            // single quotes require escaping and quotes
            t.push_back('\\');
            t.push_back(*i);
            quote = true;
        } else {
            // everything else requires quotes
            t.push_back(*i);
            quote = true;
        }
        if (i == val && !g_ascii_isalpha(*i)) {
            // a non-ASCII/non-alpha initial character requires quotes
            quote = true;
        }
        last_was_space = is_space;
    }

    if (last_was_space) {
        // a trailing space requires quotes
        quote = true;
    }

    if (quote) {
        // we use single quotes so the result can be stored in an XML
        // attribute without incurring un-aesthetic additional quoting
        // (our XML emitter always uses double quotes)
        t.insert(t.begin(), '\'');
        t.push_back('\'');
    }

    return t;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
