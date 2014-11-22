#ifndef SEEN_SP_CANVAS_H
#define SEEN_SP_CANVAS_H

/**
 * @file
 * SPCanvas, SPCanvasBuf.
 */
/*
 * Authors:
 *   Federico Mena <federico@nuclecu.unam.mx>
 *   Raph Levien <raph@gimp.org>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 1998 The Free Software Foundation
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <gtk/gtk.h>
#include <glibmm/ustring.h>
#include <2geom/affine.h>
#include <2geom/rect.h>

G_BEGIN_DECLS

#define SP_TYPE_CANVAS (sp_canvas_get_type())
#define SP_CANVAS(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_CANVAS, SPCanvas))
#define SP_IS_CANVAS(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_CANVAS))

struct SPCanvas;
struct SPCanvasItem;
struct SPCanvasGroup;

enum {
    SP_CANVAS_UPDATE_REQUESTED  = 1 << 0,
    SP_CANVAS_UPDATE_AFFINE     = 1 << 1
};

/**
 * Structure used when rendering canvas items.
 */
struct SPCanvasBuf {
    cairo_t *ct;
    Geom::IntRect rect;
    Geom::IntRect visible_rect;

    unsigned char *buf;
    int buf_rowstride;
    bool is_empty;
};

G_END_DECLS

// SPCanvas -------------------------------------------------

class SPCanvasImpl;

GType sp_canvas_get_type() G_GNUC_CONST;

/**
 * Port of GnomeCanvas for inkscape needs.
 */
struct SPCanvas {
    friend class SPCanvasImpl;

    /**
     * Returns new canvas as widget.
     */
    static GtkWidget *createAA();

    /**
     * Returns the root group of the specified canvas.
     */
    SPCanvasGroup *getRoot();

    /**
     * Scrolls canvas to specific position (cx and cy are measured in screen pixels).
     */
    void scrollTo(double cx, double cy, unsigned int clear, bool is_scrolling = false);


    /**
     * Updates canvas if necessary.
     */
    void updateNow();

    /**
     * Forces redraw of rectangular canvas area.
     */
    void requestRedraw(int x1, int y1, int x2, int y2);

    /**
     * Force a full redraw after a specified number of interrupted redraws.
     */
    void forceFullRedrawAfterInterruptions(unsigned int count);

    /**
     * End forced full redraw requests.
     */
    void endForcedFullRedraws();


    // Data members: ----------------------------------------------------------

    GtkWidget widget;

    guint idle_id;

    SPCanvasItem *root;

    bool is_dragging;
    double dx0;
    double dy0;
    int x0;
    int y0;

    /* Area that needs redrawing, stored as a microtile array */
    int    tLeft, tTop, tRight, tBottom;
    int    tileH, tileV;
    uint8_t *tiles;

    /** Last known modifier state, for deferred repick when a button is down. */
    int state;

    /** The item containing the mouse pointer, or NULL if none. */
    SPCanvasItem *current_item;

    /** Item that is about to become current (used to track deletions and such). */
    SPCanvasItem *new_current_item;

    /** Item that holds a pointer grab, or NULL if none. */
    SPCanvasItem *grabbed_item;

    /** Event mask specified when grabbing an item. */
    guint grabbed_event_mask;

    /** If non-NULL, the currently focused item. */
    SPCanvasItem *focused_item;

    /** Event on which selection of current item is based. */
    GdkEvent pick_event;

    int close_enough;

    unsigned int need_update : 1;
    unsigned int need_redraw : 1;
    unsigned int need_repick : 1;

    int forced_redraw_count;
    int forced_redraw_limit;

    /** For use by internal pick_current_item() function. */
    unsigned int left_grabbed_item : 1;

    /** For use by internal pick_current_item() function. */
    unsigned int in_repick : 1;

    // In most tools Inkscape only generates enter and leave events
    // on the current item, but no other enter events if a mouse button
    // is depressed -- see function pick_current_item().  Some tools
    // may wish the canvas to generate to all enter events, (e.g., the
    // connector tool).  If so, they may temporarily set this flag to
    // 'true'.
    bool gen_all_enter_events;
    
    /** For scripting, sometimes we want to delay drawing. */
    bool drawing_disabled;

    int rendermode;
    int colorrendermode;

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    bool enable_cms_display_adj;
    Glib::ustring cms_key;
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

    bool is_scrolling;

    Geom::Rect getViewbox() const;
    Geom::IntRect getViewboxIntegers() const;
};

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
