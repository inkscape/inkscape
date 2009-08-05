#define __SP_FLOOD_CONTEXT_C__

/** @file
 * @brief Bucket fill drawing context, works by bitmap filling an area on a rendered version
 * of the current display and then tracing the result using potrace.
 */
/* Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   John Bintz <jcoswell@coswellproductions.org>
 *
 * Copyright (C) 2006      Johan Engelen <johan@shouraizou.nl>
 * Copyright (C) 2000-2005 authors
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gdk/gdkkeysyms.h>
#include <queue>
#include <deque>

#include "macros.h"
#include "display/sp-canvas.h"
#include "document.h"
#include "sp-namedview.h"
#include "sp-object.h"
#include "sp-rect.h"
#include "selection.h"
#include "desktop-handles.h"
#include "desktop.h"
#include "desktop-style.h"
#include "message-stack.h"
#include "message-context.h"
#include "pixmaps/cursor-paintbucket.xpm"
#include "flood-context.h"
#include "sp-metrics.h"
#include <glibmm/i18n.h>
#include "object-edit.h"
#include "xml/repr.h"
#include "xml/node-event-vector.h"
#include "preferences.h"
#include "context-fns.h"
#include "rubberband.h"
#include "shape-editor.h"

#include "display/nr-arena-item.h"
#include "display/nr-arena.h"
#include "display/nr-arena-image.h"
#include "display/canvas-arena.h"
#include "libnr/nr-pixops.h"
#include "libnr/nr-matrix-translate-ops.h"
#include "libnr/nr-scale-ops.h"
#include "libnr/nr-scale-translate-ops.h"
#include "libnr/nr-translate-matrix-ops.h"
#include "libnr/nr-translate-scale-ops.h"
#include "libnr/nr-matrix-ops.h"
#include <2geom/pathvector.h>
#include "sp-item.h"
#include "sp-root.h"
#include "sp-defs.h"
#include "sp-path.h"
#include "splivarot.h"
#include "livarot/Path.h"
#include "livarot/Shape.h"
#include "svg/svg.h"
#include "color.h"

#include "trace/trace.h"
#include "trace/imagemap.h"
#include "trace/potrace/inkscape-potrace.h"

static void sp_flood_context_class_init(SPFloodContextClass *klass);
static void sp_flood_context_init(SPFloodContext *flood_context);
static void sp_flood_context_dispose(GObject *object);

static void sp_flood_context_setup(SPEventContext *ec);

static gint sp_flood_context_root_handler(SPEventContext *event_context, GdkEvent *event);
static gint sp_flood_context_item_handler(SPEventContext *event_context, SPItem *item, GdkEvent *event);

static void sp_flood_finish(SPFloodContext *rc);

static SPEventContextClass *parent_class;


GtkType sp_flood_context_get_type()
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPFloodContextClass),
            NULL, NULL,
            (GClassInitFunc) sp_flood_context_class_init,
            NULL, NULL,
            sizeof(SPFloodContext),
            4,
            (GInstanceInitFunc) sp_flood_context_init,
            NULL,    /* value_table */
        };
        type = g_type_register_static(SP_TYPE_EVENT_CONTEXT, "SPFloodContext", &info, (GTypeFlags) 0);
    }
    return type;
}

static void sp_flood_context_class_init(SPFloodContextClass *klass)
{
    GObjectClass *object_class = (GObjectClass *) klass;
    SPEventContextClass *event_context_class = (SPEventContextClass *) klass;

    parent_class = (SPEventContextClass *) g_type_class_peek_parent(klass);

    object_class->dispose = sp_flood_context_dispose;

    event_context_class->setup = sp_flood_context_setup;
    event_context_class->root_handler  = sp_flood_context_root_handler;
    event_context_class->item_handler  = sp_flood_context_item_handler;
}

static void sp_flood_context_init(SPFloodContext *flood_context)
{
    SPEventContext *event_context = SP_EVENT_CONTEXT(flood_context);

    event_context->cursor_shape = cursor_paintbucket_xpm;
    event_context->hot_x = 11;
    event_context->hot_y = 30;
    event_context->xp = 0;
    event_context->yp = 0;
    event_context->tolerance = 4;
    event_context->within_tolerance = false;
    event_context->item_to_select = NULL;

    flood_context->item = NULL;

    new (&flood_context->sel_changed_connection) sigc::connection();
}

static void sp_flood_context_dispose(GObject *object)
{
    SPFloodContext *rc = SP_FLOOD_CONTEXT(object);
    SPEventContext *ec = SP_EVENT_CONTEXT(object);

    rc->sel_changed_connection.disconnect();
    rc->sel_changed_connection.~connection();

    delete ec->shape_editor;
    ec->shape_editor = NULL;

    /* fixme: This is necessary because we do not grab */
    if (rc->item) {
        sp_flood_finish(rc);
    }

    if (rc->_message_context) {
        delete rc->_message_context;
    }

    G_OBJECT_CLASS(parent_class)->dispose(object);
}

/**
\brief  Callback that processes the "changed" signal on the selection;
destroys old and creates new knotholder
*/
void sp_flood_context_selection_changed(Inkscape::Selection *selection, gpointer data)
{
    SPFloodContext *rc = SP_FLOOD_CONTEXT(data);
    SPEventContext *ec = SP_EVENT_CONTEXT(rc);

    ec->shape_editor->unset_item(SH_KNOTHOLDER);
    SPItem *item = selection->singleItem(); 
    ec->shape_editor->set_item(item, SH_KNOTHOLDER);
}

static void sp_flood_context_setup(SPEventContext *ec)
{
    SPFloodContext *rc = SP_FLOOD_CONTEXT(ec);

    if (((SPEventContextClass *) parent_class)->setup) {
        ((SPEventContextClass *) parent_class)->setup(ec);
    }

    ec->shape_editor = new ShapeEditor(ec->desktop);

    SPItem *item = sp_desktop_selection(ec->desktop)->singleItem();
    if (item) {
        ec->shape_editor->set_item(item, SH_KNOTHOLDER);
    }

    rc->sel_changed_connection.disconnect();
    rc->sel_changed_connection = sp_desktop_selection(ec->desktop)->connectChanged(
        sigc::bind(sigc::ptr_fun(&sp_flood_context_selection_changed), (gpointer)rc)
    );

    rc->_message_context = new Inkscape::MessageContext((ec->desktop)->messageStack());

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/tools/paintbucket/selcue")) {
        rc->enableSelectionCue();
    }
}

/**
 * \brief Merge a pixel with the background color.
 * \param orig The pixel to merge with the background.
 * \param bg The background color.
 * \param base The pixel to merge the original and background into.
 */
inline static void
merge_pixel_with_background (unsigned char *orig, unsigned char *bg,
           unsigned char *base)
{
    int precalc_bg_alpha = (255 * (255 - bg[3])) / 255;
    
    for (int i = 0; i < 3; i++) {
        base[i] = precalc_bg_alpha + (bg[i] * bg[3]) / 255;
        base[i] = (base[i] * (255 - orig[3])) / 255 + (orig[i] * orig[3]) / 255;
    }
}

/**
 * \brief Get the pointer to a pixel in a pixel buffer.
 * \param px The pixel buffer.
 * \param x The X coordinate.
 * \param y The Y coordinate.
 * \param width The width of the pixel buffer.
 */
inline unsigned char * get_pixel(guchar *px, int x, int y, int width) {
    return px + (x + y * width) * 4;
}

inline unsigned char * get_trace_pixel(guchar *trace_px, int x, int y, int width) {
    return trace_px + (x + y * width);
}

/**
 * \brief Generate the list of trace channel selection entries.
 */
GList * flood_channels_dropdown_items_list() {
    GList *glist = NULL;

    glist = g_list_append (glist, _("Visible Colors"));
    glist = g_list_append (glist, _("Red"));
    glist = g_list_append (glist, _("Green"));
    glist = g_list_append (glist, _("Blue"));
    glist = g_list_append (glist, _("Hue"));
    glist = g_list_append (glist, _("Saturation"));
    glist = g_list_append (glist, _("Lightness"));
    glist = g_list_append (glist, _("Alpha"));

    return glist;
}

/**
 * \brief Generate the list of autogap selection entries.
 */
GList * flood_autogap_dropdown_items_list() {
    GList *glist = NULL;

    glist = g_list_append (glist, _("None"));
    glist = g_list_append (glist, _("Small"));
    glist = g_list_append (glist, _("Medium"));
    glist = g_list_append (glist, _("Large"));

    return glist;
}

/**
 * \brief Compare a pixel in a pixel buffer with another pixel to determine if a point should be included in the fill operation.
 * \param check The pixel in the pixel buffer to check.
 * \param orig The original selected pixel to use as the fill target color.
 * \param merged_orig_pixel The original pixel merged with the background.
 * \param dtc The desktop background color.
 * \param threshold The fill threshold.
 * \param method The fill method to use as defined in PaintBucketChannels.
 */
static bool compare_pixels(unsigned char *check, unsigned char *orig, unsigned char *merged_orig_pixel, unsigned char *dtc, int threshold, PaintBucketChannels method) {
    int diff = 0;
    float hsl_check[3], hsl_orig[3];
    
    if ((method == FLOOD_CHANNELS_H) ||
        (method == FLOOD_CHANNELS_S) ||
        (method == FLOOD_CHANNELS_L)) {
        sp_color_rgb_to_hsl_floatv(hsl_check, check[0] / 255.0, check[1] / 255.0, check[2] / 255.0);
        sp_color_rgb_to_hsl_floatv(hsl_orig, orig[0] / 255.0, orig[1] / 255.0, orig[2] / 255.0);
    }
    
    switch (method) {
        case FLOOD_CHANNELS_ALPHA:
            return ((int)abs(check[3] - orig[3]) <= threshold);
        case FLOOD_CHANNELS_R:
            return ((int)abs(check[0] - orig[0]) <= threshold);
        case FLOOD_CHANNELS_G:
            return ((int)abs(check[1] - orig[1]) <= threshold);
        case FLOOD_CHANNELS_B:
            return ((int)abs(check[2] - orig[2]) <= threshold);
        case FLOOD_CHANNELS_RGB:
            unsigned char merged_check[3];
            
            merge_pixel_with_background(check, dtc, merged_check);
            
            for (int i = 0; i < 3; i++) {
              diff += (int)abs(merged_check[i] - merged_orig_pixel[i]);
            }
            return ((diff / 3) <= ((threshold * 3) / 4));
        
        case FLOOD_CHANNELS_H:
            return ((int)(fabs(hsl_check[0] - hsl_orig[0]) * 100.0) <= threshold);
        case FLOOD_CHANNELS_S:
            return ((int)(fabs(hsl_check[1] - hsl_orig[1]) * 100.0) <= threshold);
        case FLOOD_CHANNELS_L:
            return ((int)(fabs(hsl_check[2] - hsl_orig[2]) * 100.0) <= threshold);
    }
    
    return false;
}

enum {
  PIXEL_CHECKED = 1,
  PIXEL_QUEUED  = 2,
  PIXEL_PAINTABLE = 4,
  PIXEL_NOT_PAINTABLE = 8,
  PIXEL_COLORED = 16
};

static inline bool is_pixel_checked(unsigned char *t) { return (*t & PIXEL_CHECKED) == PIXEL_CHECKED; }
static inline bool is_pixel_queued(unsigned char *t) { return (*t & PIXEL_QUEUED) == PIXEL_QUEUED; }
static inline bool is_pixel_paintability_checked(unsigned char *t) {
  return !((*t & PIXEL_PAINTABLE) == 0) && ((*t & PIXEL_NOT_PAINTABLE) == 0);
}
static inline bool is_pixel_paintable(unsigned char *t) { return (*t & PIXEL_PAINTABLE) == PIXEL_PAINTABLE; }
static inline bool is_pixel_colored(unsigned char *t) { return (*t & PIXEL_COLORED) == PIXEL_COLORED; }

static inline void mark_pixel_checked(unsigned char *t) { *t |= PIXEL_CHECKED; }
static inline void mark_pixel_unchecked(unsigned char *t) { *t ^= PIXEL_CHECKED; }
static inline void mark_pixel_queued(unsigned char *t) { *t |= PIXEL_QUEUED; }
static inline void mark_pixel_paintable(unsigned char *t) { *t |= PIXEL_PAINTABLE; *t ^= PIXEL_NOT_PAINTABLE; }
static inline void mark_pixel_not_paintable(unsigned char *t) { *t |= PIXEL_NOT_PAINTABLE; *t ^= PIXEL_PAINTABLE; }
static inline void mark_pixel_colored(unsigned char *t) { *t |= PIXEL_COLORED; }

static inline void clear_pixel_paintability(unsigned char *t) { *t ^= PIXEL_PAINTABLE; *t ^= PIXEL_NOT_PAINTABLE; }

struct bitmap_coords_info {
    bool is_left;
    unsigned int x;
    unsigned int y;
    int y_limit;
    unsigned int width;
    unsigned int height;
    unsigned int threshold;
    unsigned int radius;
    PaintBucketChannels method;
    unsigned char *dtc;
    unsigned char *merged_orig_pixel;
    Geom::Rect bbox;
    Geom::Rect screen;
    unsigned int max_queue_size;
    unsigned int current_step;
};

/**
 * \brief Check if a pixel can be included in the fill.
 * \param px The rendered pixel buffer to check.
 * \param trace_t The pixel in the trace pixel buffer to check or mark.
 * \param x The X coordinate.
 * \param y The y coordinate.
 * \param orig_color The original selected pixel to use as the fill target color.
 * \param bci The bitmap_coords_info structure.
 */
inline static bool check_if_pixel_is_paintable(guchar *px, unsigned char *trace_t, int x, int y, unsigned char *orig_color, bitmap_coords_info bci) {
    if (is_pixel_paintability_checked(trace_t)) {
        return is_pixel_paintable(trace_t);
    } else {
        unsigned char *t = get_pixel(px, x, y, bci.width);
        if (compare_pixels(t, orig_color, bci.merged_orig_pixel, bci.dtc, bci.threshold, bci.method)) {
            mark_pixel_paintable(trace_t);
            return true;
        } else {
            mark_pixel_not_paintable(trace_t);
            return false;
        }
    }
}

/**
 * \brief Perform the bitmap-to-vector tracing and place the traced path onto the document.
 * \param px The trace pixel buffer to trace to SVG.
 * \param desktop The desktop on which to place the final SVG path.
 * \param transform The transform to apply to the final SVG path.
 * \param union_with_selection If true, merge the final SVG path with the current selection.
 */
static void do_trace(bitmap_coords_info bci, guchar *trace_px, SPDesktop *desktop, Geom::Matrix transform, unsigned int min_x, unsigned int max_x, unsigned int min_y, unsigned int max_y, bool union_with_selection) {
    Document *document = sp_desktop_document(desktop);

    unsigned char *trace_t;

    GrayMap *gray_map = GrayMapCreate((max_x - min_x + 1), (max_y - min_y + 1));
    unsigned int gray_map_y = 0;
    for (unsigned int y = min_y; y <= max_y; y++) {
        unsigned long *gray_map_t = gray_map->rows[gray_map_y];

        trace_t = get_trace_pixel(trace_px, min_x, y, bci.width);
        for (unsigned int x = min_x; x <= max_x; x++) {
            *gray_map_t = is_pixel_colored(trace_t) ? GRAYMAP_BLACK : GRAYMAP_WHITE;
            gray_map_t++;
            trace_t++;
        }
        gray_map_y++;
    }

    Inkscape::Trace::Potrace::PotraceTracingEngine pte;
    pte.keepGoing = 1;
    std::vector<Inkscape::Trace::TracingEngineResult> results = pte.traceGrayMap(gray_map);
    gray_map->destroy(gray_map);

    Inkscape::XML::Node *layer_repr = SP_GROUP(desktop->currentLayer())->repr;
    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(desktop->doc());

    long totalNodeCount = 0L;

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    double offset = prefs->getDouble("/tools/paintbucket/offset", 0.0);

    for (unsigned int i=0 ; i<results.size() ; i++) {
        Inkscape::Trace::TracingEngineResult result = results[i];
        totalNodeCount += result.getNodeCount();

        Inkscape::XML::Node *pathRepr = xml_doc->createElement("svg:path");
        /* Set style */
        sp_desktop_apply_style_tool (desktop, pathRepr, "/tools/paintbucket", false);

        Geom::PathVector pathv = sp_svg_read_pathv(result.getPathData().c_str());
        Path *path = new Path;
        path->LoadPathVector(pathv);

        if (offset != 0) {
        
            Shape *path_shape = new Shape();
        
            path->ConvertWithBackData(0.03);
            path->Fill(path_shape, 0);
            delete path;
        
            Shape *expanded_path_shape = new Shape();
        
            expanded_path_shape->ConvertToShape(path_shape, fill_nonZero);
            path_shape->MakeOffset(expanded_path_shape, offset * desktop->current_zoom(), join_round, 4);
            expanded_path_shape->ConvertToShape(path_shape, fill_positive);

            Path *expanded_path = new Path();
        
            expanded_path->Reset();
            expanded_path_shape->ConvertToForme(expanded_path);
            expanded_path->ConvertEvenLines(1.0);
            expanded_path->Simplify(1.0);
        
            delete path_shape;
            delete expanded_path_shape;
        
            gchar *str = expanded_path->svg_dump_path();
            if (str && *str) {
                pathRepr->setAttribute("d", str);
                g_free(str);
            } else {
                desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("<b>Too much inset</b>, the result is empty."));
                Inkscape::GC::release(pathRepr);
                g_free(str);
                return;
            }

            delete expanded_path;

        } else {
            gchar *str = path->svg_dump_path();
            delete path;
            pathRepr->setAttribute("d", str);
            g_free(str);
        }

        layer_repr->addChild(pathRepr, NULL);

        SPObject *reprobj = document->getObjectByRepr(pathRepr);
        if (reprobj) {
            sp_item_write_transform(SP_ITEM(reprobj), pathRepr, transform, NULL);
            
            // premultiply the item transform by the accumulated parent transform in the paste layer
            Geom::Matrix local (sp_item_i2doc_affine(SP_GROUP(desktop->currentLayer())));
            if (!local.isIdentity()) {
                gchar const *t_str = pathRepr->attribute("transform");
                Geom::Matrix item_t (Geom::identity());
                if (t_str)
                    sp_svg_transform_read(t_str, &item_t);
                item_t *= local.inverse();
                // (we're dealing with unattached repr, so we write to its attr instead of using sp_item_set_transform)
                gchar *affinestr=sp_svg_transform_write(item_t);
                pathRepr->setAttribute("transform", affinestr);
                g_free(affinestr);
            }

            Inkscape::Selection *selection = sp_desktop_selection(desktop);

            pathRepr->setPosition(-1);

            if (union_with_selection) {
                desktop->messageStack()->flashF(Inkscape::WARNING_MESSAGE, ngettext("Area filled, path with <b>%d</b> node created and unioned with selection.","Area filled, path with <b>%d</b> nodes created and unioned with selection.",sp_nodes_in_path(SP_PATH(reprobj))), sp_nodes_in_path(SP_PATH(reprobj)));
                selection->add(reprobj);
                sp_selected_path_union_skip_undo(desktop);
            } else {
                desktop->messageStack()->flashF(Inkscape::WARNING_MESSAGE, ngettext("Area filled, path with <b>%d</b> node created.","Area filled, path with <b>%d</b> nodes created.",sp_nodes_in_path(SP_PATH(reprobj))), sp_nodes_in_path(SP_PATH(reprobj)));
                selection->set(reprobj);
            }

        }

        Inkscape::GC::release(pathRepr);

    }
}

/**
 * \brief The possible return states of perform_bitmap_scanline_check()
 */
enum ScanlineCheckResult {
    SCANLINE_CHECK_OK,
    SCANLINE_CHECK_ABORTED,
    SCANLINE_CHECK_BOUNDARY
};

/**
 * \brief Determine if the provided coordinates are within the pixel buffer limits.
 * \param x The X coordinate.
 * \param y The Y coordinate.
 * \param bci The bitmap_coords_info structure.
 */
inline static bool coords_in_range(unsigned int x, unsigned int y, bitmap_coords_info bci) {
    return (x < bci.width) &&
           (y < bci.height);
}

#define PAINT_DIRECTION_LEFT 1
#define PAINT_DIRECTION_RIGHT 2
#define PAINT_DIRECTION_UP 4
#define PAINT_DIRECTION_DOWN 8
#define PAINT_DIRECTION_ALL 15

/**
 * \brief Paint a pixel or a square (if autogap is enabled) on the trace pixel buffer
 * \param px The rendered pixel buffer to check.
 * \param trace_px The trace pixel buffer.
 * \param orig_color The original selected pixel to use as the fill target color.
 * \param bci The bitmap_coords_info structure.
 * \param original_point_trace_t The original pixel in the trace pixel buffer to check.
 */
inline static unsigned int paint_pixel(guchar *px, guchar *trace_px, unsigned char *orig_color, bitmap_coords_info bci, unsigned char *original_point_trace_t) {
    if (bci.radius == 0) {
        mark_pixel_colored(original_point_trace_t); 
        return PAINT_DIRECTION_ALL;
    } else {
        unsigned char *trace_t;
  
        bool can_paint_up = true;
        bool can_paint_down = true;
        bool can_paint_left = true;
        bool can_paint_right = true;
      
        for (unsigned int ty = bci.y - bci.radius; ty <= bci.y + bci.radius; ty++) {
            for (unsigned int tx = bci.x - bci.radius; tx <= bci.x + bci.radius; tx++) {
                if (coords_in_range(tx, ty, bci)) {
                    trace_t = get_trace_pixel(trace_px, tx, ty, bci.width);
                    if (!is_pixel_colored(trace_t)) {
                        if (check_if_pixel_is_paintable(px, trace_t, tx, ty, orig_color, bci)) {
                            mark_pixel_colored(trace_t); 
                        } else {
                            if (tx < bci.x) { can_paint_left = false; }
                            if (tx > bci.x) { can_paint_right = false; }
                            if (ty < bci.y) { can_paint_up = false; }
                            if (ty > bci.y) { can_paint_down = false; }
                        }
                    }
                }
            }
        }
    
        unsigned int paint_directions = 0;
        if (can_paint_left) { paint_directions += PAINT_DIRECTION_LEFT; }
        if (can_paint_right) { paint_directions += PAINT_DIRECTION_RIGHT; }
        if (can_paint_up) { paint_directions += PAINT_DIRECTION_UP; }
        if (can_paint_down) { paint_directions += PAINT_DIRECTION_DOWN; }
        
        return paint_directions;
    }
}

/**
 * \brief Push a point to be checked onto the bottom of the rendered pixel buffer check queue.
 * \param fill_queue The fill queue to add the point to.
 * \param max_queue_size The maximum size of the fill queue.
 * \param trace_t The trace pixel buffer pixel.
 * \param x The X coordinate.
 * \param y The Y coordinate.
 */
static void push_point_onto_queue(std::deque<Geom::Point> *fill_queue, unsigned int max_queue_size, unsigned char *trace_t, unsigned int x, unsigned int y) {
    if (!is_pixel_queued(trace_t)) {
        if ((fill_queue->size() < max_queue_size)) {
            fill_queue->push_back(Geom::Point(x, y));
            mark_pixel_queued(trace_t);
        }
    }
}

/**
 * \brief Shift a point to be checked onto the top of the rendered pixel buffer check queue.
 * \param fill_queue The fill queue to add the point to.
 * \param max_queue_size The maximum size of the fill queue.
 * \param trace_t The trace pixel buffer pixel.
 * \param x The X coordinate.
 * \param y The Y coordinate.
 */
static void shift_point_onto_queue(std::deque<Geom::Point> *fill_queue, unsigned int max_queue_size, unsigned char *trace_t, unsigned int x, unsigned int y) {
    if (!is_pixel_queued(trace_t)) {
        if ((fill_queue->size() < max_queue_size)) {
            fill_queue->push_front(Geom::Point(x, y));
            mark_pixel_queued(trace_t);
        }
    }
}

/**
 * \brief Scan a row in the rendered pixel buffer and add points to the fill queue as necessary.
 * \param fill_queue The fill queue to add the point to.
 * \param px The rendered pixel buffer.
 * \param trace_px The trace pixel buffer.
 * \param orig_color The original selected pixel to use as the fill target color.
 * \param bci The bitmap_coords_info structure.
 */
static ScanlineCheckResult perform_bitmap_scanline_check(std::deque<Geom::Point> *fill_queue, guchar *px, guchar *trace_px, unsigned char *orig_color, bitmap_coords_info bci, unsigned int *min_x, unsigned int *max_x) {
    bool aborted = false;
    bool reached_screen_boundary = false;
    bool ok;

    bool keep_tracing;
    bool initial_paint = true;

    unsigned char *current_trace_t = get_trace_pixel(trace_px, bci.x, bci.y, bci.width);
    unsigned int paint_directions;

    bool currently_painting_top = false;
    bool currently_painting_bottom = false;

    unsigned int top_ty = bci.y - 1;
    unsigned int bottom_ty = bci.y + 1;

    bool can_paint_top = (top_ty > 0);
    bool can_paint_bottom = (bottom_ty < bci.height);

    Geom::Point t = fill_queue->front();

    do {
        ok = false;
        if (bci.is_left) {
            keep_tracing = (bci.x != 0);
        } else {
            keep_tracing = (bci.x < bci.width);
        }

        *min_x = MIN(*min_x, bci.x);
        *max_x = MAX(*max_x, bci.x);

        if (keep_tracing) {
            if (check_if_pixel_is_paintable(px, current_trace_t, bci.x, bci.y, orig_color, bci)) {
                paint_directions = paint_pixel(px, trace_px, orig_color, bci, current_trace_t);
                if (bci.radius == 0) {
                    mark_pixel_checked(current_trace_t);
                    if ((t[Geom::X] == bci.x) && (t[Geom::Y] == bci.y)) {
                        fill_queue->pop_front(); t = fill_queue->front();
                    }
                }

                if (can_paint_top) {
                    if (paint_directions & PAINT_DIRECTION_UP) { 
                        unsigned char *trace_t = current_trace_t - bci.width;
                        if (!is_pixel_queued(trace_t)) {
                            bool ok_to_paint = check_if_pixel_is_paintable(px, trace_t, bci.x, top_ty, orig_color, bci);

                            if (initial_paint) { currently_painting_top = !ok_to_paint; }

                            if (ok_to_paint && (!currently_painting_top)) {
                                currently_painting_top = true;
                                push_point_onto_queue(fill_queue, bci.max_queue_size, trace_t, bci.x, top_ty);
                            }
                            if ((!ok_to_paint) && currently_painting_top) {
                                currently_painting_top = false;
                            }
                        }
                    }
                }

                if (can_paint_bottom) {
                    if (paint_directions & PAINT_DIRECTION_DOWN) { 
                        unsigned char *trace_t = current_trace_t + bci.width;
                        if (!is_pixel_queued(trace_t)) {
                            bool ok_to_paint = check_if_pixel_is_paintable(px, trace_t, bci.x, bottom_ty, orig_color, bci);

                            if (initial_paint) { currently_painting_bottom = !ok_to_paint; }

                            if (ok_to_paint && (!currently_painting_bottom)) {
                                currently_painting_bottom = true;
                                push_point_onto_queue(fill_queue, bci.max_queue_size, trace_t, bci.x, bottom_ty);
                            }
                            if ((!ok_to_paint) && currently_painting_bottom) {
                                currently_painting_bottom = false;
                            }
                        }
                    }
                }

                if (bci.is_left) {
                    if (paint_directions & PAINT_DIRECTION_LEFT) {
                        bci.x--; current_trace_t--;
                        ok = true;
                    }
                } else {
                    if (paint_directions & PAINT_DIRECTION_RIGHT) {
                        bci.x++; current_trace_t++;
                        ok = true;
                    }
                }

                initial_paint = false;
            }
        } else {
            if (bci.bbox.min()[Geom::X] > bci.screen.min()[Geom::X]) {
                aborted = true; break;
            } else {
                reached_screen_boundary = true;
            }
        }
    } while (ok);

    if (aborted) { return SCANLINE_CHECK_ABORTED; }
    if (reached_screen_boundary) { return SCANLINE_CHECK_BOUNDARY; }
    return SCANLINE_CHECK_OK;
}

/**
 * \brief Sort the rendered pixel buffer check queue vertically.
 */
static bool sort_fill_queue_vertical(Geom::Point a, Geom::Point b) {
    return a[Geom::Y] > b[Geom::Y];
}

/**
 * \brief Sort the rendered pixel buffer check queue horizontally.
 */
static bool sort_fill_queue_horizontal(Geom::Point a, Geom::Point b) {
    return a[Geom::X] > b[Geom::X];
}

/**
 * \brief Perform a flood fill operation.
 * \param event_context The event context for this tool.
 * \param event The details of this event.
 * \param union_with_selection If true, union the new fill with the current selection.
 * \param is_point_fill If false, use the Rubberband "touch selection" to get the initial points for the fill.
 * \param is_touch_fill If true, use only the initial contact point in the Rubberband "touch selection" as the fill target color.
 */
static void sp_flood_do_flood_fill(SPEventContext *event_context, GdkEvent *event, bool union_with_selection, bool is_point_fill, bool is_touch_fill) {
    SPDesktop *desktop = event_context->desktop;
    Document *document = sp_desktop_document(desktop);

    /* Create new arena */
    NRArena *arena = NRArena::create();
    unsigned dkey = sp_item_display_key_new(1);

    sp_document_ensure_up_to_date (document);
    
    SPItem *document_root = SP_ITEM(SP_DOCUMENT_ROOT(document));
    Geom::OptRect bbox = document_root->getBounds(Geom::identity());

    if (!bbox) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("<b>Area is not bounded</b>, cannot fill."));
        return;
    }
    
    double zoom_scale = desktop->current_zoom();
    
    // Render 160% of the physical display to the render pixel buffer, so that available
    // fill areas off the screen can be included in the fill.
    double padding = 1.6;

    Geom::Rect screen = desktop->get_display_area();

    unsigned int width = (int)ceil(screen.width() * zoom_scale * padding);
    unsigned int height = (int)ceil(screen.height() * zoom_scale * padding);

    Geom::Point origin(screen.min()[Geom::X],
                       sp_document_height(document) - screen.height() - screen.min()[Geom::Y]);
                    
    origin[Geom::X] = origin[Geom::X] + (screen.width() * ((1 - padding) / 2));
    origin[Geom::Y] = origin[Geom::Y] + (screen.height() * ((1 - padding) / 2));
    
    Geom::Scale scale(zoom_scale, zoom_scale);
    Geom::Matrix affine = scale * Geom::Translate(-origin * scale);
    
    /* Create ArenaItems and set transform */
    NRArenaItem *root = sp_item_invoke_show(SP_ITEM(sp_document_root(document)), arena, dkey, SP_ITEM_SHOW_DISPLAY);
    nr_arena_item_set_transform(NR_ARENA_ITEM(root), affine);

    NRGC gc(NULL);
    gc.transform.setIdentity();
    
    NRRectL final_bbox;
    final_bbox.x0 = 0;
    final_bbox.y0 = 0; //row;
    final_bbox.x1 = width;
    final_bbox.y1 = height; //row + num_rows;
    
    nr_arena_item_invoke_update(root, &final_bbox, &gc, NR_ARENA_ITEM_STATE_ALL, NR_ARENA_ITEM_STATE_NONE);

    guchar *px = g_new(guchar, 4 * width * height);
    
    NRPixBlock B;
    nr_pixblock_setup_extern( &B, NR_PIXBLOCK_MODE_R8G8B8A8N,
                              final_bbox.x0, final_bbox.y0, final_bbox.x1, final_bbox.y1,
                              px, 4 * width, FALSE, FALSE );
    
    SPNamedView *nv = sp_desktop_namedview(desktop);
    unsigned long bgcolor = nv->pagecolor;
    
    unsigned char dtc[4];
    dtc[0] = NR_RGBA32_R(bgcolor);
    dtc[1] = NR_RGBA32_G(bgcolor);
    dtc[2] = NR_RGBA32_B(bgcolor);
    dtc[3] = NR_RGBA32_A(bgcolor);
    
    for (unsigned int fy = 0; fy < height; fy++) {
        guchar *p = NR_PIXBLOCK_PX(&B) + fy * B.rs;
        for (unsigned int fx = 0; fx < width; fx++) {
            for (int i = 0; i < 4; i++) { 
                *p++ = dtc[i];
            }
        }
    }

    nr_arena_item_invoke_render(NULL, root, &final_bbox, &B, NR_ARENA_ITEM_RENDER_NO_CACHE );
    nr_pixblock_release(&B);
    
    // Hide items
    sp_item_invoke_hide(SP_ITEM(sp_document_root(document)), dkey);
    
    nr_object_unref((NRObject *) arena);
    
    guchar *trace_px = g_new(guchar, width * height);
    memset(trace_px, 0x00, width * height);
    
    std::deque<Geom::Point> fill_queue;
    std::queue<Geom::Point> color_queue;
    
    std::vector<Geom::Point> fill_points;
    
    bool aborted = false;
    int y_limit = height - 1;

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    PaintBucketChannels method = (PaintBucketChannels) prefs->getInt("/tools/paintbucket/channels", 0);
    int threshold = prefs->getIntLimited("/tools/paintbucket/threshold", 1, 0, 100);

    switch(method) {
        case FLOOD_CHANNELS_ALPHA:
        case FLOOD_CHANNELS_RGB:
        case FLOOD_CHANNELS_R:
        case FLOOD_CHANNELS_G:
        case FLOOD_CHANNELS_B:
            threshold = (255 * threshold) / 100;
            break;
        case FLOOD_CHANNELS_H:
        case FLOOD_CHANNELS_S:
        case FLOOD_CHANNELS_L:
            break;
    }

    bitmap_coords_info bci;
    
    bci.y_limit = y_limit;
    bci.width = width;
    bci.height = height;
    bci.threshold = threshold;
    bci.method = method;
    bci.bbox = *bbox;
    bci.screen = screen;
    bci.dtc = dtc;
    bci.radius = prefs->getIntLimited("/tools/paintbucket/autogap", 0, 0, 3);
    bci.max_queue_size = (width * height) / 4;
    bci.current_step = 0;

    if (is_point_fill) {
        fill_points.push_back(Geom::Point(event->button.x, event->button.y));
    } else {
        Inkscape::Rubberband::Rubberband *r = Inkscape::Rubberband::get(desktop);
        fill_points = r->getPoints();
    }

    for (unsigned int i = 0; i < fill_points.size(); i++) {
        Geom::Point pw = Geom::Point(fill_points[i][Geom::X] / zoom_scale, sp_document_height(document) + (fill_points[i][Geom::Y] / zoom_scale)) * affine;

        pw[Geom::X] = (int)MIN(width - 1, MAX(0, pw[Geom::X]));
        pw[Geom::Y] = (int)MIN(height - 1, MAX(0, pw[Geom::Y]));

        if (is_touch_fill) {
            if (i == 0) {
                color_queue.push(pw);
            } else {
                unsigned char *trace_t = get_trace_pixel(trace_px, (int)pw[Geom::X], (int)pw[Geom::Y], width);
                push_point_onto_queue(&fill_queue, bci.max_queue_size, trace_t, (int)pw[Geom::X], (int)pw[Geom::Y]);
            }
        } else {
            color_queue.push(pw);
        }
    }

    bool reached_screen_boundary = false;

    bool first_run = true;

    unsigned long sort_size_threshold = 5;

    unsigned int min_y = height;
    unsigned int max_y = 0;
    unsigned int min_x = width;
    unsigned int max_x = 0;

    while (!color_queue.empty() && !aborted) {
        Geom::Point color_point = color_queue.front();
        color_queue.pop();

        int cx = (int)color_point[Geom::X];
        int cy = (int)color_point[Geom::Y];

        unsigned char *orig_px = get_pixel(px, cx, cy, width);
        unsigned char orig_color[4];
        for (int i = 0; i < 4; i++) { orig_color[i] = orig_px[i]; }

        unsigned char merged_orig[3];

        merge_pixel_with_background(orig_color, dtc, merged_orig);

        bci.merged_orig_pixel = merged_orig;

        unsigned char *trace_t = get_trace_pixel(trace_px, cx, cy, width);
        if (!is_pixel_checked(trace_t) && !is_pixel_colored(trace_t)) {
            if (check_if_pixel_is_paintable(px, trace_px, cx, cy, orig_color, bci)) {
                shift_point_onto_queue(&fill_queue, bci.max_queue_size, trace_t, cx, cy);

                if (!first_run) {
                    for (unsigned int y = 0; y < height; y++) {
                        trace_t = get_trace_pixel(trace_px, 0, y, width);
                        for (unsigned int x = 0; x < width; x++) {
                            clear_pixel_paintability(trace_t);
                            trace_t++;
                        }
                    }
                }
                first_run = false;
            }
        }

        unsigned long old_fill_queue_size = fill_queue.size();

        while (!fill_queue.empty() && !aborted) {
            Geom::Point cp = fill_queue.front();

            if (bci.radius == 0) {
                unsigned long new_fill_queue_size = fill_queue.size();

                /*
                 * To reduce the number of points in the fill queue, periodically
                 * resort all of the points in the queue so that scanline checks
                 * can complete more quickly.  A point cannot be checked twice
                 * in a normal scanline checks, so forcing scanline checks to start
                 * from one corner of the rendered area as often as possible
                 * will reduce the number of points that need to be checked and queued.
                 */
                if (new_fill_queue_size > sort_size_threshold) {
                    if (new_fill_queue_size > old_fill_queue_size) {
                        std::sort(fill_queue.begin(), fill_queue.end(), sort_fill_queue_vertical);

                        std::deque<Geom::Point>::iterator start_sort = fill_queue.begin();
                        std::deque<Geom::Point>::iterator end_sort = fill_queue.begin();
                        unsigned int sort_y = (unsigned int)cp[Geom::Y];
                        unsigned int current_y = sort_y;
                        
                        for (std::deque<Geom::Point>::iterator i = fill_queue.begin(); i != fill_queue.end(); i++) {
                            Geom::Point current = *i;
                            current_y = (unsigned int)current[Geom::Y];
                            if (current_y != sort_y) {
                                if (start_sort != end_sort) {
                                    std::sort(start_sort, end_sort, sort_fill_queue_horizontal);
                                }
                                sort_y = current_y;
                                start_sort = i;
                            }
                            end_sort = i;
                        }
                        if (start_sort != end_sort) {
                            std::sort(start_sort, end_sort, sort_fill_queue_horizontal);
                        }
                        
                        cp = fill_queue.front();
                    }
                }

                old_fill_queue_size = new_fill_queue_size;
            }

            fill_queue.pop_front();

            int x = (int)cp[Geom::X];
            int y = (int)cp[Geom::Y];

            min_y = MIN((unsigned int)y, min_y);
            max_y = MAX((unsigned int)y, max_y);

            unsigned char *trace_t = get_trace_pixel(trace_px, x, y, width);
            if (!is_pixel_checked(trace_t)) {
                mark_pixel_checked(trace_t);

                if (y == 0) {
                    if (bbox->min()[Geom::Y] > screen.min()[Geom::Y]) {
                        aborted = true; break;
                    } else {
                        reached_screen_boundary = true;
                    }
                }

                if (y == y_limit) {
                    if (bbox->max()[Geom::Y] < screen.max()[Geom::Y]) {
                        aborted = true; break;
                    } else {
                        reached_screen_boundary = true;
                    }
                }

                bci.is_left = true;
                bci.x = x;
                bci.y = y;

                ScanlineCheckResult result = perform_bitmap_scanline_check(&fill_queue, px, trace_px, orig_color, bci, &min_x, &max_x);

                switch (result) {
                    case SCANLINE_CHECK_ABORTED:
                        aborted = true;
                        break;
                    case SCANLINE_CHECK_BOUNDARY:
                        reached_screen_boundary = true;
                        break;
                    default:
                        break;
                }

                if (bci.x < width) {
                    trace_t++;
                    if (!is_pixel_checked(trace_t) && !is_pixel_queued(trace_t)) {
                        mark_pixel_checked(trace_t);
                        bci.is_left = false;
                        bci.x = x + 1;

                        result = perform_bitmap_scanline_check(&fill_queue, px, trace_px, orig_color, bci, &min_x, &max_x);

                        switch (result) {
                            case SCANLINE_CHECK_ABORTED:
                                aborted = true;
                                break;
                            case SCANLINE_CHECK_BOUNDARY:
                                reached_screen_boundary = true;
                                break;
                            default:
                                break;
                        }
                    }
                }
            }

            bci.current_step++;

            if (bci.current_step > bci.max_queue_size) {
                aborted = true;
            }
        }
    }
    
    g_free(px);
    
    if (aborted) {
        g_free(trace_px);
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("<b>Area is not bounded</b>, cannot fill."));
        return;
    }
    
    if (reached_screen_boundary) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("<b>Only the visible part of the bounded area was filled.</b> If you want to fill all of the area, undo, zoom out, and fill again.")); 
    }

    unsigned int trace_padding = bci.radius + 1;
    if (min_y > trace_padding) { min_y -= trace_padding; }
    if (max_y < (y_limit - trace_padding)) { max_y += trace_padding; }
    if (min_x > trace_padding) { min_x -= trace_padding; }
    if (max_x < (width - 1 - trace_padding)) { max_x += trace_padding; }

    Geom::Point min_start = Geom::Point(min_x, min_y);
    
    affine = scale * Geom::Translate(-origin * scale - min_start);
    Geom::Matrix inverted_affine = Geom::Matrix(affine).inverse();
    
    do_trace(bci, trace_px, desktop, inverted_affine, min_x, max_x, min_y, max_y, union_with_selection);

    g_free(trace_px);
    
    sp_document_done(document, SP_VERB_CONTEXT_PAINTBUCKET, _("Fill bounded area"));
}

static gint sp_flood_context_item_handler(SPEventContext *event_context, SPItem *item, GdkEvent *event)
{
    gint ret = FALSE;

    SPDesktop *desktop = event_context->desktop;

    switch (event->type) {
    case GDK_BUTTON_PRESS:
        if ((event->button.state & GDK_CONTROL_MASK) && event->button.button == 1 && !event_context->space_panning) {
            Geom::Point const button_w(event->button.x,
                                       event->button.y);
            
            SPItem *item = sp_event_context_find_item (desktop, button_w, TRUE, TRUE);
            
            Inkscape::XML::Node *pathRepr = SP_OBJECT_REPR(item);
            /* Set style */
            sp_desktop_apply_style_tool (desktop, pathRepr, "/tools/paintbucket", false);
            sp_document_done(sp_desktop_document(desktop), SP_VERB_CONTEXT_PAINTBUCKET, _("Set style on object"));
            ret = TRUE;
        }
        break;
    default:
        break;
    }

    if (((SPEventContextClass *) parent_class)->item_handler) {
        ret = ((SPEventContextClass *) parent_class)->item_handler(event_context, item, event);
    }

    return ret;
}

static gint sp_flood_context_root_handler(SPEventContext *event_context, GdkEvent *event)
{
    static bool dragging;
    
    gint ret = FALSE;
    SPDesktop *desktop = event_context->desktop;

    switch (event->type) {
    case GDK_BUTTON_PRESS:
        if (event->button.button == 1 && !event_context->space_panning) {
            if (!(event->button.state & GDK_CONTROL_MASK)) {
                Geom::Point const button_w(event->button.x,
                                           event->button.y);
    
                if (Inkscape::have_viable_layer(desktop, event_context->defaultMessageContext())) {
                    // save drag origin
                    event_context->xp = (gint) button_w[Geom::X];
                    event_context->yp = (gint) button_w[Geom::Y];
                    event_context->within_tolerance = true;
                      
                    dragging = true;
                    
                    Geom::Point const p(desktop->w2d(button_w));
                    Inkscape::Rubberband::get(desktop)->setMode(RUBBERBAND_MODE_TOUCHPATH);
                    Inkscape::Rubberband::get(desktop)->start(desktop, p);
                }
            }
        }
    case GDK_MOTION_NOTIFY:
        if ( dragging
             && ( event->motion.state & GDK_BUTTON1_MASK ) && !event_context->space_panning)
        {
            if ( event_context->within_tolerance
                 && ( abs( (gint) event->motion.x - event_context->xp ) < event_context->tolerance )
                 && ( abs( (gint) event->motion.y - event_context->yp ) < event_context->tolerance ) ) {
                break; // do not drag if we're within tolerance from origin
            }
            
            event_context->within_tolerance = false;
            
            Geom::Point const motion_pt(event->motion.x, event->motion.y);
            Geom::Point const p(desktop->w2d(motion_pt));
            if (Inkscape::Rubberband::get(desktop)->is_started()) {
                Inkscape::Rubberband::get(desktop)->move(p);
                event_context->defaultMessageContext()->set(Inkscape::NORMAL_MESSAGE, _("<b>Draw over</b> areas to add to fill, hold <b>Alt</b> for touch fill"));
                gobble_motion_events(GDK_BUTTON1_MASK);
            }
        }
        break;

    case GDK_BUTTON_RELEASE:
        if (event->button.button == 1 && !event_context->space_panning) {
            Inkscape::Rubberband::Rubberband *r = Inkscape::Rubberband::get(desktop);
            if (r->is_started()) {
                // set "busy" cursor
                desktop->setWaitingCursor();

                if (SP_IS_EVENT_CONTEXT(event_context)) { 
                    // Since setWaitingCursor runs main loop iterations, we may have already left this tool!
                    // So check if the tool is valid before doing anything
                    dragging = false;

                    bool is_point_fill = event_context->within_tolerance;
                    bool is_touch_fill = event->button.state & GDK_MOD1_MASK;
                    
                    sp_flood_do_flood_fill(event_context, event, event->button.state & GDK_SHIFT_MASK, is_point_fill, is_touch_fill);
                    
                    desktop->clearWaitingCursor();
                    // restore cursor when done; note that it may already be different if e.g. user 
                    // switched to another tool during interruptible tracing or drawing, in which case do nothing

                    ret = TRUE;
                }

                r->stop();

                if (SP_IS_EVENT_CONTEXT(event_context)) {
                    event_context->defaultMessageContext()->clear();
                }
            }
        }
        break;
    case GDK_KEY_PRESS:
        switch (get_group0_keyval (&event->key)) {
        case GDK_Up:
        case GDK_Down:
        case GDK_KP_Up:
        case GDK_KP_Down:
            // prevent the zoom field from activation
            if (!MOD__CTRL_ONLY)
                ret = TRUE;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    if (!ret) {
        if (((SPEventContextClass *) parent_class)->root_handler) {
            ret = ((SPEventContextClass *) parent_class)->root_handler(event_context, event);
        }
    }

    return ret;
}


static void sp_flood_finish(SPFloodContext *rc)
{
    rc->_message_context->clear();

    if ( rc->item != NULL ) {
        SPDesktop * desktop;

        desktop = SP_EVENT_CONTEXT_DESKTOP(rc);

        SP_OBJECT(rc->item)->updateRepr();

        sp_canvas_end_forced_full_redraws(desktop->canvas);

        sp_desktop_selection(desktop)->set(rc->item);
        sp_document_done(sp_desktop_document(desktop), SP_VERB_CONTEXT_PAINTBUCKET,
                        _("Fill bounded area"));

        rc->item = NULL;
    }
}

void flood_channels_set_channels( gint channels )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setInt("/tools/paintbucket/channels", channels);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
