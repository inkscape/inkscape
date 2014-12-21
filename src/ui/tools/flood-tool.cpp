/**
 * @file
 * Bucket fill drawing context, works by bitmap filling an area on a rendered version
 * of the current display and then tracing the result using potrace.
 */
/* Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   John Bintz <jcoswell@coswellproductions.org>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
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

#include "trace/potrace/inkscape-potrace.h"
#include <2geom/pathvector.h>
#include <gdk/gdkkeysyms.h>
#include <queue>
#include <deque>
#include <glibmm/i18n.h>

#include "color.h"
#include "context-fns.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "desktop-style.h"
#include "display/cairo-utils.h"
#include "display/drawing-context.h"
#include "display/drawing-image.h"
#include "display/drawing-item.h"
#include "display/drawing.h"
#include "display/sp-canvas.h"
#include "document.h"
#include "document-undo.h"
#include "ui/tools/flood-tool.h"
#include "livarot/Path.h"
#include "livarot/Shape.h"
#include "macros.h"
#include "message-context.h"
#include "message-stack.h"
#include "preferences.h"
#include "rubberband.h"
#include "selection.h"
#include "ui/shape-editor.h"
#include "sp-defs.h"
#include "sp-item.h"
#include "splivarot.h"
#include "sp-namedview.h"
#include "sp-object.h"
#include "sp-path.h"
#include "sp-rect.h"
#include "sp-root.h"
#include "svg/svg.h"
#include "trace/imagemap.h"
#include "trace/trace.h"
#include "xml/node-event-vector.h"
#include "xml/repr.h"
#include "verbs.h"

#include "pixmaps/cursor-paintbucket.xpm"

using Inkscape::DocumentUndo;

using Inkscape::Display::ExtractARGB32;
using Inkscape::Display::ExtractRGB32;
using Inkscape::Display::AssembleARGB32;

#include "ui/tool-factory.h"

namespace Inkscape {
namespace UI {
namespace Tools {

namespace {
	ToolBase* createPaintbucketContext() {
		return new FloodTool();
	}

	bool paintbucketContextRegistered = ToolFactory::instance().registerObject("/tools/paintbucket", createPaintbucketContext);
}

const std::string& FloodTool::getPrefsPath() {
	return FloodTool::prefsPath;
}

const std::string FloodTool::prefsPath = "/tools/paintbucket";

FloodTool::FloodTool()
    : ToolBase(cursor_paintbucket_xpm, 11, 30)
    , item(NULL)
{
    // TODO: Why does the flood tool use a hardcoded tolerance instead of a pref?
    this->tolerance = 4;
}

FloodTool::~FloodTool() {
    this->sel_changed_connection.disconnect();

    delete this->shape_editor;
    this->shape_editor = NULL;

    /* fixme: This is necessary because we do not grab */
    if (this->item) {
        this->finishItem();
    }
}

/**
 * Callback that processes the "changed" signal on the selection;
 * destroys old and creates new knotholder.
 */
void FloodTool::selection_changed(Inkscape::Selection* selection) {
    this->shape_editor->unset_item();
    this->shape_editor->set_item(selection->singleItem());
}

void FloodTool::setup() {
    ToolBase::setup();

    this->shape_editor = new ShapeEditor(this->desktop);

    SPItem *item = this->desktop->getSelection()->singleItem();
    if (item) {
        this->shape_editor->set_item(item);
    }

    this->sel_changed_connection.disconnect();
    this->sel_changed_connection = this->desktop->getSelection()->connectChanged(
    	sigc::mem_fun(this, &FloodTool::selection_changed)
    );

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    if (prefs->getBool("/tools/paintbucket/selcue")) {
        this->enableSelectionCue();
    }
}


// Changes from 0.48 -> 0.49 (Cairo)
// 0.49: Ignores alpha in background
// 0.48: RGBA, 0.49 ARGB
// 0.49: premultiplied alpha
inline static guint32 compose_onto(guint32 px, guint32 bg)
{
    guint ap = 0, rp = 0, gp = 0, bp = 0;
    guint rb = 0, gb = 0, bb = 0;
    ExtractARGB32(px, ap, rp, gp, bp);
    ExtractRGB32(bg, rb, gb, bb);

    // guint ao = 255*255 - (255-ap)*(255-bp);  ao = (ao + 127) / 255;
    // guint ao = (255-ap)*ab + 255*ap;             ao = (ao + 127) / 255;
    guint ao = 255; // Cairo version doesn't allow background to have alpha != 1.
    guint ro = (255-ap)*rb + 255*rp;             ro = (ro + 127) / 255;
    guint go = (255-ap)*gb + 255*gp;             go = (go + 127) / 255;
    guint bo = (255-ap)*bb + 255*bp;             bo = (bo + 127) / 255;

    guint pxout = AssembleARGB32(ao, ro, go, bo);
    return pxout;
}

/**
 * Get the pointer to a pixel in a pixel buffer.
 * @param px The pixel buffer.
 * @param x The X coordinate.
 * @param y The Y coordinate.
 * @param stride The rowstride of the pixel buffer.
 */
inline guint32 get_pixel(guchar *px, int x, int y, int stride) {
    return *reinterpret_cast<guint32*>(px + y * stride + x * 4);
}

inline unsigned char * get_trace_pixel(guchar *trace_px, int x, int y, int width) {
    return trace_px + (x + y * width);
}

/**
 * Generate the list of trace channel selection entries.
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
 * Generate the list of autogap selection entries.
 */
GList * flood_autogap_dropdown_items_list() {
    GList *glist = NULL;

    glist = g_list_append (glist, (void*) C_("Flood autogap", "None"));
    glist = g_list_append (glist, (void*) C_("Flood autogap", "Small"));
    glist = g_list_append (glist, (void*) C_("Flood autogap", "Medium"));
    glist = g_list_append (glist, (void*) C_("Flood autogap", "Large"));

    return glist;
}

/**
 * Compare a pixel in a pixel buffer with another pixel to determine if a point should be included in the fill operation.
 * @param check The pixel in the pixel buffer to check.
 * @param orig The original selected pixel to use as the fill target color.
 * @param merged_orig_pixel The original pixel merged with the background.
 * @param dtc The desktop background color.
 * @param threshold The fill threshold.
 * @param method The fill method to use as defined in PaintBucketChannels.
 */
static bool compare_pixels(guint32 check, guint32 orig, guint32 merged_orig_pixel, guint32 dtc, int threshold, PaintBucketChannels method)
{
    int diff = 0;
    float hsl_check[3] = {0,0,0}, hsl_orig[3] = {0,0,0};

    guint32 ac = 0, rc = 0, gc = 0, bc = 0;
    ExtractARGB32(check, ac, rc, gc, bc);

    guint32 ao = 0, ro = 0, go = 0, bo = 0;
    ExtractARGB32(orig, ao, ro, go, bo);

    guint32 ad = 0, rd = 0, gd = 0, bd = 0;
    ExtractARGB32(dtc, ad, rd, gd, bd);

    guint32 amop = 0, rmop = 0, gmop = 0, bmop = 0;
    ExtractARGB32(merged_orig_pixel, amop, rmop, gmop, bmop);

    if ((method == FLOOD_CHANNELS_H) ||
        (method == FLOOD_CHANNELS_S) ||
        (method == FLOOD_CHANNELS_L)) {
        double dac = ac;
        double dao = ao;
        sp_color_rgb_to_hsl_floatv(hsl_check, rc / dac, gc / dac, bc / dac);
        sp_color_rgb_to_hsl_floatv(hsl_orig, ro / dao, go / dao, bo / dao);
    }
    
    switch (method) {
        case FLOOD_CHANNELS_ALPHA:
            return abs(static_cast<int>(ac) - ao) <= threshold;
        case FLOOD_CHANNELS_R:
            return abs(static_cast<int>(ac ? unpremul_alpha(rc, ac) : 0) - (ao ? unpremul_alpha(ro, ao) : 0)) <= threshold;
        case FLOOD_CHANNELS_G:
            return abs(static_cast<int>(ac ? unpremul_alpha(gc, ac) : 0) - (ao ? unpremul_alpha(go, ao) : 0)) <= threshold;
        case FLOOD_CHANNELS_B:
            return abs(static_cast<int>(ac ? unpremul_alpha(bc, ac) : 0) - (ao ? unpremul_alpha(bo, ao) : 0)) <= threshold;
        case FLOOD_CHANNELS_RGB:
            guint32 amc, rmc, bmc, gmc;
            //amc = 255*255 - (255-ac)*(255-ad); amc = (amc + 127) / 255;
            //amc = (255-ac)*ad + 255*ac; amc = (amc + 127) / 255;
            amc = 255; // Why are we looking at desktop? Cairo version ignores destop alpha
            rmc = (255-ac)*rd + 255*rc; rmc = (rmc + 127) / 255;
            gmc = (255-ac)*gd + 255*gc; gmc = (gmc + 127) / 255;
            bmc = (255-ac)*bd + 255*bc; bmc = (bmc + 127) / 255;

            diff += abs(static_cast<int>(amc ? unpremul_alpha(rmc, amc) : 0) - (amop ? unpremul_alpha(rmop, amop) : 0));
            diff += abs(static_cast<int>(amc ? unpremul_alpha(gmc, amc) : 0) - (amop ? unpremul_alpha(gmop, amop) : 0));
            diff += abs(static_cast<int>(amc ? unpremul_alpha(bmc, amc) : 0) - (amop ? unpremul_alpha(bmop, amop) : 0));
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
    unsigned int stride;
    unsigned int threshold;
    unsigned int radius;
    PaintBucketChannels method;
    guint32 dtc;
    guint32 merged_orig_pixel;
    Geom::Rect bbox;
    Geom::Rect screen;
    unsigned int max_queue_size;
    unsigned int current_step;
};

/**
 * Check if a pixel can be included in the fill.
 * @param px The rendered pixel buffer to check.
 * @param trace_t The pixel in the trace pixel buffer to check or mark.
 * @param x The X coordinate.
 * @param y The y coordinate.
 * @param orig_color The original selected pixel to use as the fill target color.
 * @param bci The bitmap_coords_info structure.
 */
inline static bool check_if_pixel_is_paintable(guchar *px, unsigned char *trace_t, int x, int y, guint32 orig_color, bitmap_coords_info bci) {
    if (is_pixel_paintability_checked(trace_t)) {
        return is_pixel_paintable(trace_t);
    } else {
        guint32 pixel = get_pixel(px, x, y, bci.stride);
        if (compare_pixels(pixel, orig_color, bci.merged_orig_pixel, bci.dtc, bci.threshold, bci.method)) {
            mark_pixel_paintable(trace_t);
            return true;
        } else {
            mark_pixel_not_paintable(trace_t);
            return false;
        }
    }
}

/**
 * Perform the bitmap-to-vector tracing and place the traced path onto the document.
 * @param px The trace pixel buffer to trace to SVG.
 * @param desktop The desktop on which to place the final SVG path.
 * @param transform The transform to apply to the final SVG path.
 * @param union_with_selection If true, merge the final SVG path with the current selection.
 */
static void do_trace(bitmap_coords_info bci, guchar *trace_px, SPDesktop *desktop, Geom::Affine transform, unsigned int min_x, unsigned int max_x, unsigned int min_y, unsigned int max_y, bool union_with_selection) {
    SPDocument *document = sp_desktop_document(desktop);

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

    //XML Tree being used here directly while it shouldn't be...."
    Inkscape::XML::Document *xml_doc = desktop->doc()->getReprDoc();

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

        desktop->currentLayer()->addChild(pathRepr,NULL);

        SPObject *reprobj = document->getObjectByRepr(pathRepr);
        if (reprobj) {
            SP_ITEM(reprobj)->doWriteTransform(pathRepr, transform, NULL);
            
            // premultiply the item transform by the accumulated parent transform in the paste layer
            Geom::Affine local (SP_GROUP(desktop->currentLayer())->i2doc_affine());
            if (!local.isIdentity()) {
                gchar const *t_str = pathRepr->attribute("transform");
                Geom::Affine item_t (Geom::identity());
                if (t_str)
                    sp_svg_transform_read(t_str, &item_t);
                item_t *= local.inverse();
                // (we're dealing with unattached repr, so we write to its attr instead of using sp_item_set_transform)
                gchar *affinestr=sp_svg_transform_write(item_t);
                pathRepr->setAttribute("transform", affinestr);
                g_free(affinestr);
            }

            Inkscape::Selection *selection = desktop->getSelection();

            pathRepr->setPosition(-1);

            if (union_with_selection) {
                desktop->messageStack()->flashF( Inkscape::WARNING_MESSAGE,
                    ngettext("Area filled, path with <b>%d</b> node created and unioned with selection.","Area filled, path with <b>%d</b> nodes created and unioned with selection.",
                    SP_PATH(reprobj)->nodesInPath()), SP_PATH(reprobj)->nodesInPath() );
                selection->add(reprobj);
                sp_selected_path_union_skip_undo(desktop->getSelection(), desktop);
            } else {
                desktop->messageStack()->flashF( Inkscape::WARNING_MESSAGE,
                    ngettext("Area filled, path with <b>%d</b> node created.","Area filled, path with <b>%d</b> nodes created.",
                    SP_PATH(reprobj)->nodesInPath()), SP_PATH(reprobj)->nodesInPath() );
                selection->set(reprobj);
            }

        }

        Inkscape::GC::release(pathRepr);

    }
}

/**
 * The possible return states of perform_bitmap_scanline_check().
 */
enum ScanlineCheckResult {
    SCANLINE_CHECK_OK,
    SCANLINE_CHECK_ABORTED,
    SCANLINE_CHECK_BOUNDARY
};

/**
 * Determine if the provided coordinates are within the pixel buffer limits.
 * @param x The X coordinate.
 * @param y The Y coordinate.
 * @param bci The bitmap_coords_info structure.
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
 * Paint a pixel or a square (if autogap is enabled) on the trace pixel buffer.
 * @param px The rendered pixel buffer to check.
 * @param trace_px The trace pixel buffer.
 * @param orig_color The original selected pixel to use as the fill target color.
 * @param bci The bitmap_coords_info structure.
 * @param original_point_trace_t The original pixel in the trace pixel buffer to check.
 */
inline static unsigned int paint_pixel(guchar *px, guchar *trace_px, guint32 orig_color, bitmap_coords_info bci, unsigned char *original_point_trace_t) {
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
 * Push a point to be checked onto the bottom of the rendered pixel buffer check queue.
 * @param fill_queue The fill queue to add the point to.
 * @param max_queue_size The maximum size of the fill queue.
 * @param trace_t The trace pixel buffer pixel.
 * @param x The X coordinate.
 * @param y The Y coordinate.
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
 * Shift a point to be checked onto the top of the rendered pixel buffer check queue.
 * @param fill_queue The fill queue to add the point to.
 * @param max_queue_size The maximum size of the fill queue.
 * @param trace_t The trace pixel buffer pixel.
 * @param x The X coordinate.
 * @param y The Y coordinate.
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
 * Scan a row in the rendered pixel buffer and add points to the fill queue as necessary.
 * @param fill_queue The fill queue to add the point to.
 * @param px The rendered pixel buffer.
 * @param trace_px The trace pixel buffer.
 * @param orig_color The original selected pixel to use as the fill target color.
 * @param bci The bitmap_coords_info structure.
 */
static ScanlineCheckResult perform_bitmap_scanline_check(std::deque<Geom::Point> *fill_queue, guchar *px, guchar *trace_px, guint32 orig_color, bitmap_coords_info bci, unsigned int *min_x, unsigned int *max_x) {
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
 * Sort the rendered pixel buffer check queue vertically.
 */
static bool sort_fill_queue_vertical(Geom::Point a, Geom::Point b) {
    return a[Geom::Y] > b[Geom::Y];
}

/**
 * Sort the rendered pixel buffer check queue horizontally.
 */
static bool sort_fill_queue_horizontal(Geom::Point a, Geom::Point b) {
    return a[Geom::X] > b[Geom::X];
}

/**
 * Perform a flood fill operation.
 * @param event_context The event context for this tool.
 * @param event The details of this event.
 * @param union_with_selection If true, union the new fill with the current selection.
 * @param is_point_fill If false, use the Rubberband "touch selection" to get the initial points for the fill.
 * @param is_touch_fill If true, use only the initial contact point in the Rubberband "touch selection" as the fill target color.
 */
static void sp_flood_do_flood_fill(ToolBase *event_context, GdkEvent *event, bool union_with_selection, bool is_point_fill, bool is_touch_fill) {
    SPDesktop *desktop = event_context->desktop;
    SPDocument *document = sp_desktop_document(desktop);

    document->ensureUpToDate();
    
    Geom::OptRect bbox = document->getRoot()->visualBounds();

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
                       document->getHeight().value("px") - screen.height() - screen.min()[Geom::Y]);
                    
    origin[Geom::X] += (screen.width() * ((1 - padding) / 2));
    origin[Geom::Y] += (screen.height() * ((1 - padding) / 2));
    
    Geom::Scale scale(zoom_scale, zoom_scale);
    Geom::Affine affine = scale * Geom::Translate(-origin * scale);

    int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width);
    guchar *px = g_new(guchar, stride * height);
    guint32 bgcolor, dtc;

    // Draw image into data block px
    { // this block limits the lifetime of Drawing and DrawingContext
        /* Create DrawingItems and set transform */
        unsigned dkey = SPItem::display_key_new(1);
        Inkscape::Drawing drawing;
        Inkscape::DrawingItem *root = document->getRoot()->invoke_show( drawing, dkey, SP_ITEM_SHOW_DISPLAY);
        root->setTransform(affine);
        drawing.setRoot(root);

        Geom::IntRect final_bbox = Geom::IntRect::from_xywh(0, 0, width, height);
        drawing.update(final_bbox);

        cairo_surface_t *s = cairo_image_surface_create_for_data(
            px, CAIRO_FORMAT_ARGB32, width, height, stride);
        Inkscape::DrawingContext dc(s, Geom::Point(0,0));
        // cairo_translate not necessary here - surface origin is at 0,0

        SPNamedView *nv = sp_desktop_namedview(desktop);
        bgcolor = nv->pagecolor;
        // bgcolor is 0xrrggbbaa, we need 0xaarrggbb
        dtc = (bgcolor >> 8) | (bgcolor << 24);

        dc.setSource(bgcolor);
        dc.setOperator(CAIRO_OPERATOR_SOURCE);
        dc.paint();
        dc.setOperator(CAIRO_OPERATOR_OVER);

        drawing.render(dc, final_bbox);

        //cairo_surface_write_to_png( s, "cairo.png" );

        cairo_surface_flush(s);
        cairo_surface_destroy(s);
        
        // Hide items
        document->getRoot()->invoke_hide(dkey);
    }

    // {
    //     // Dump data to png
    //     cairo_surface_t *s = cairo_image_surface_create_for_data(
    //         px, CAIRO_FORMAT_ARGB32, width, height, stride);
    //     cairo_surface_write_to_png( s, "cairo2.png" );
    //     std::cout << "  Wrote cairo2.png" << std::endl;
    // }

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
    bci.stride = stride;
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
        Inkscape::Rubberband *r = Inkscape::Rubberband::get(desktop);
        fill_points = r->getPoints();
    }

    for (unsigned int i = 0; i < fill_points.size(); i++) {
        Geom::Point pw = Geom::Point(fill_points[i][Geom::X] / zoom_scale, document->getHeight().value("px") + (fill_points[i][Geom::Y] / zoom_scale)) * affine;

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

        guint32 orig_color = get_pixel(px, cx, cy, stride);
        bci.merged_orig_pixel = compose_onto(orig_color, dtc);

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
                        
                        for (std::deque<Geom::Point>::iterator i = fill_queue.begin(); i != fill_queue.end(); ++i) {
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
    Geom::Affine inverted_affine = Geom::Affine(affine).inverse();
    
    do_trace(bci, trace_px, desktop, inverted_affine, min_x, max_x, min_y, max_y, union_with_selection);

    g_free(trace_px);
    
    DocumentUndo::done(document, SP_VERB_CONTEXT_PAINTBUCKET, _("Fill bounded area"));
}

bool FloodTool::item_handler(SPItem* item, GdkEvent* event) {
    gint ret = FALSE;

    switch (event->type) {
    case GDK_BUTTON_PRESS:
        if ((event->button.state & GDK_CONTROL_MASK) && event->button.button == 1 && !this->space_panning) {
            Geom::Point const button_w(event->button.x, event->button.y);
            
            SPItem *item = sp_event_context_find_item (desktop, button_w, TRUE, TRUE);
            
            // Set style
            desktop->applyCurrentOrToolStyle(item, "/tools/paintbucket", false);

            DocumentUndo::done(sp_desktop_document(desktop), SP_VERB_CONTEXT_PAINTBUCKET, _("Set style on object"));

            ret = TRUE;
        }
        break;

    default:
        break;
    }

//    if (((ToolBaseClass *) sp_flood_context_parent_class)->item_handler) {
//        ret = ((ToolBaseClass *) sp_flood_context_parent_class)->item_handler(event_context, item, event);
//    }
    // CPPIFY: ret is overwritten...
    ret = ToolBase::item_handler(item, event);

    return ret;
}

bool FloodTool::root_handler(GdkEvent* event) {
    static bool dragging;
    
    gint ret = FALSE;

    switch (event->type) {
    case GDK_BUTTON_PRESS:
        if (event->button.button == 1 && !this->space_panning) {
            if (!(event->button.state & GDK_CONTROL_MASK)) {
                Geom::Point const button_w(event->button.x, event->button.y);
    
                if (Inkscape::have_viable_layer(desktop, this->defaultMessageContext())) {
                    // save drag origin
                    this->xp = (gint) button_w[Geom::X];
                    this->yp = (gint) button_w[Geom::Y];
                    this->within_tolerance = true;
                      
                    dragging = true;
                    
                    Geom::Point const p(desktop->w2d(button_w));
                    Inkscape::Rubberband::get(desktop)->setMode(RUBBERBAND_MODE_TOUCHPATH);
                    Inkscape::Rubberband::get(desktop)->start(desktop, p);
                }
            }
        }

    case GDK_MOTION_NOTIFY:
        if ( dragging && ( event->motion.state & GDK_BUTTON1_MASK ) && !this->space_panning) {
            if ( this->within_tolerance
                 && ( abs( (gint) event->motion.x - this->xp ) < this->tolerance )
                 && ( abs( (gint) event->motion.y - this->yp ) < this->tolerance ) ) {
                break; // do not drag if we're within tolerance from origin
            }
            
            this->within_tolerance = false;
            
            Geom::Point const motion_pt(event->motion.x, event->motion.y);
            Geom::Point const p(desktop->w2d(motion_pt));

            if (Inkscape::Rubberband::get(desktop)->is_started()) {
                Inkscape::Rubberband::get(desktop)->move(p);
                this->defaultMessageContext()->set(Inkscape::NORMAL_MESSAGE, _("<b>Draw over</b> areas to add to fill, hold <b>Alt</b> for touch fill"));
                gobble_motion_events(GDK_BUTTON1_MASK);
            }
        }
        break;

    case GDK_BUTTON_RELEASE:
        if (event->button.button == 1 && !this->space_panning) {
            Inkscape::Rubberband *r = Inkscape::Rubberband::get(desktop);

            if (r->is_started()) {
                // set "busy" cursor
                desktop->setWaitingCursor();

                if (SP_IS_EVENT_CONTEXT(this)) { 
                    // Since setWaitingCursor runs main loop iterations, we may have already left this tool!
                    // So check if the tool is valid before doing anything
                    dragging = false;

                    bool is_point_fill = this->within_tolerance;
                    bool is_touch_fill = event->button.state & GDK_MOD1_MASK;
                    
                    sp_flood_do_flood_fill(this, event, event->button.state & GDK_SHIFT_MASK, is_point_fill, is_touch_fill);
                    
                    desktop->clearWaitingCursor();
                    // restore cursor when done; note that it may already be different if e.g. user 
                    // switched to another tool during interruptible tracing or drawing, in which case do nothing

                    ret = TRUE;
                }

                r->stop();

                //if (SP_IS_EVENT_CONTEXT(this)) {
                this->defaultMessageContext()->clear();
                //}
            }
        }
        break;
    case GDK_KEY_PRESS:
        switch (get_group0_keyval (&event->key)) {
        case GDK_KEY_Up:
        case GDK_KEY_Down:
        case GDK_KEY_KP_Up:
        case GDK_KEY_KP_Down:
            // prevent the zoom field from activation
            if (!MOD__CTRL_ONLY(event))
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
    	ret = ToolBase::root_handler(event);
    }

    return ret;
}

void FloodTool::finishItem() {
    this->message_context->clear();

    if (this->item != NULL) {
        this->item->updateRepr();

        desktop->canvas->endForcedFullRedraws();

        desktop->getSelection()->set(this->item);

        DocumentUndo::done(sp_desktop_document(desktop), SP_VERB_CONTEXT_PAINTBUCKET, _("Fill bounded area"));

        this->item = NULL;
    }
}

void FloodTool::set_channels(gint channels) {
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setInt("/tools/paintbucket/channels", channels);
}

}
}
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
