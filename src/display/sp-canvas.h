#ifndef SEEN_SP_CANVAS_H
#define SEEN_SP_CANVAS_H

/** \file
 * SPCanvas, SPCanvasBuf.
 *
 * Authors:
 *   Federico Mena <federico@nuclecu.unam.mx>
 *   Raph Levien <raph@gimp.org>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1998 The Free Software Foundation
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_INTTYPES_H
# include <inttypes.h>
#else
# ifdef HAVE_STDINT_H
#  include <stdint.h>
# endif
#endif

#include <glib/gtypes.h>
#include <gdk/gdkevents.h>
#include <gdk/gdkgc.h>
#include <gtk/gtkobject.h>
#include <gtk/gtkwidget.h>

#include <glibmm/ustring.h>

#include <2geom/matrix.h>
#include <libnr/nr-rect-l.h>

#include <2geom/rect.h>

G_BEGIN_DECLS

#define SP_TYPE_CANVAS sp_canvas_get_type()
#define SP_CANVAS(obj) (GTK_CHECK_CAST((obj), SP_TYPE_CANVAS, SPCanvas))
#define SP_IS_CANVAS(obj) (GTK_CHECK_TYPE((obj), SP_TYPE_CANVAS))

GType sp_canvas_get_type();

struct SPCanvas;
struct SPCanvasItem;
struct SPCanvasGroup;

enum {
    SP_CANVAS_UPDATE_REQUESTED  = 1 << 0,
    SP_CANVAS_UPDATE_AFFINE     = 1 << 1
};

/**
 * The canvas buf contains the actual pixels.
 */
struct SPCanvasBuf{
    guchar *buf;
    int buf_rowstride;
    NRRectL rect;
    NRRectL visible_rect;
    /// Background color, given as 0xrrggbb
    guint32 bg_color;
    // If empty, ignore contents of buffer and use a solid area of bg_color
    bool is_empty;
    cairo_t *ct;
};

G_END_DECLS

// SPCanvas -------------------------------------------------
/**
 * Port of GnomeCanvas for inkscape needs.
 */
struct SPCanvas {
    GtkWidget widget;

    guint idle_id;

    SPCanvasItem *root;

    double dx0, dy0;
    int x0, y0;

    /* Area that needs redrawing, stored as a microtile array */
    int    tLeft,tTop,tRight,tBottom;
    int    tileH,tileV;
    uint8_t *tiles;

    /* Last known modifier state, for deferred repick when a button is down */
    int state;

    /* The item containing the mouse pointer, or NULL if none */
    SPCanvasItem *current_item;

    /* Item that is about to become current (used to track deletions and such) */
    SPCanvasItem *new_current_item;

    /* Item that holds a pointer grab, or NULL if none */
    SPCanvasItem *grabbed_item;

    /* Event mask specified when grabbing an item */
    guint grabbed_event_mask;

    /* If non-NULL, the currently focused item */
    SPCanvasItem *focused_item;

    /* Event on which selection of current item is based */
    GdkEvent pick_event;

    int close_enough;

    /* GC for temporary draw pixmap */
    GdkGC *pixmap_gc;

    unsigned int need_update : 1;
    unsigned int need_redraw : 1;
    unsigned int need_repick : 1;

    int forced_redraw_count;
    int forced_redraw_limit;

    /* For use by internal pick_current_item() function */
    unsigned int left_grabbed_item : 1;
    /* For use by internal pick_current_item() function */
    unsigned int in_repick : 1;

    // In most tools Inkscape only generates enter and leave events
    // on the current item, but no other enter events if a mouse button
    // is depressed -- see function pick_current_item().  Some tools
    // may wish the canvas to generate to all enter events, (e.g., the
    // connector tool).  If so, they may temporarily set this flag to
    // 'true'.
    bool gen_all_enter_events;
    
    /* For scripting, sometimes we want to delay drawing. */
    bool drawing_disabled;

    int rendermode;

#if ENABLE_LCMS
    bool enable_cms_display_adj;
    Glib::ustring* cms_key;
#endif // ENABLE_LCMS

    bool is_scrolling;

    Geom::Rect getViewbox() const;
    NR::IRect getViewboxIntegers() const;
};

GtkWidget *sp_canvas_new_aa();

SPCanvasGroup *sp_canvas_root(SPCanvas *canvas);

void sp_canvas_scroll_to(SPCanvas *canvas, double cx, double cy, unsigned int clear, bool is_scrolling = false);
void sp_canvas_update_now(SPCanvas *canvas);

void sp_canvas_request_redraw(SPCanvas *canvas, int x1, int y1, int x2, int y2);
void sp_canvas_force_full_redraw_after_interruptions(SPCanvas *canvas, unsigned int count);
void sp_canvas_end_forced_full_redraws(SPCanvas *canvas);

bool sp_canvas_world_pt_inside_window(SPCanvas const *canvas, Geom::Point const &world);

void sp_canvas_window_to_world(SPCanvas const *canvas, double winx, double winy, double *worldx, double *worldy);
void sp_canvas_world_to_window(SPCanvas const *canvas, double worldx, double worldy, double *winx, double *winy);

Geom::Point sp_canvas_window_to_world(SPCanvas const *canvas, Geom::Point const win);
Geom::Point sp_canvas_world_to_window(SPCanvas const *canvas, Geom::Point const world);

#endif // SEEN_SP_CANVAS_H

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
