/** \file
 * Anchors implementation.
 */

/*
 * Authors:
 * Copyright (C) 2000 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2002 Lauris Kaplinski
 * Copyright (C) 2004 Monash University
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include "draw-anchor.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "event-context.h"
#include "lpe-tool-context.h"
#include "display/sodipodi-ctrl.h"
#include "display/curve.h"

/**
 * Creates an anchor object and initializes it.
 */
SPDrawAnchor *
sp_draw_anchor_new(SPDrawContext *dc, SPCurve *curve, gboolean start, Geom::Point delta)
{
    if (SP_IS_LPETOOL_CONTEXT(dc)) {
        // suppress all kinds of anchors in LPEToolContext
        return NULL;
    }

    SPDesktop *dt = SP_EVENT_CONTEXT_DESKTOP(dc);

    SPDrawAnchor *a = g_new(SPDrawAnchor, 1);

    a->dc = dc;
    a->curve = curve;
    curve->ref();
    a->start = start;
    a->active = FALSE;
    a->dp = delta;
    a->ctrl = sp_canvas_item_new(sp_desktop_controls(dt), SP_TYPE_CTRL,
                                 "size", 6.0,
                                 "filled", 0,
                                 "fill_color", 0xff00007f,
                                 "stroked", 1,
                                 "stroke_color", 0x000000ff,
                                 NULL);

    SP_CTRL(a->ctrl)->moveto(delta);

    return a;
}

/**
 * Destroys the anchor's canvas item and frees the anchor object.
 */
SPDrawAnchor *
sp_draw_anchor_destroy(SPDrawAnchor *anchor)
{
    if (anchor->curve) {
        anchor->curve->unref();
    }
    if (anchor->ctrl) {
        gtk_object_destroy(GTK_OBJECT(anchor->ctrl));
    }
    g_free(anchor);
    return NULL;
}

#define A_SNAP 4.0

/**
 * Test if point is near anchor, if so fill anchor on canvas and return
 * pointer to it or NULL.
 */
SPDrawAnchor *
sp_draw_anchor_test(SPDrawAnchor *anchor, Geom::Point w, gboolean activate)
{
    SPDesktop *dt = SP_EVENT_CONTEXT_DESKTOP(anchor->dc);

    if ( activate && ( Geom::LInfty( w - dt->d2w(anchor->dp) ) <= A_SNAP ) ) {
        if (!anchor->active) {
            sp_canvas_item_set((GtkObject *) anchor->ctrl, "filled", TRUE, NULL);
            anchor->active = TRUE;
        }
        return anchor;
    }

    if (anchor->active) {
        sp_canvas_item_set((GtkObject *) anchor->ctrl, "filled", FALSE, NULL);
        anchor->active = FALSE;
    }
    return NULL;
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
