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


#include "ui/draw-anchor.h"
#include "desktop.h"
#include "ui/tools/tool-base.h"
#include "ui/tools/lpe-tool.h"
#include "display/sodipodi-ctrl.h"
#include "display/curve.h"
#include "ui/control-manager.h"

using Inkscape::ControlManager;

#define FILL_COLOR_NORMAL 0xffffff7f
#define FILL_COLOR_MOUSEOVER 0xff0000ff

/**
 * Creates an anchor object and initializes it.
 */
SPDrawAnchor *sp_draw_anchor_new(Inkscape::UI::Tools::FreehandBase *dc, SPCurve *curve, bool start, Geom::Point delta)
{
    if (SP_IS_LPETOOL_CONTEXT(dc)) {
        // suppress all kinds of anchors in LPEToolContext
        return NULL;
    }

    SPDrawAnchor *a = g_new(SPDrawAnchor, 1);

    a->dc = dc;
    a->curve = curve;
    curve->ref();
    a->start = start;
    a->active = FALSE;
    a->dp = delta;
    a->ctrl = ControlManager::getManager().createControl(dc->getDesktop().getControls(), Inkscape::CTRL_TYPE_ANCHOR);

    SP_CTRL(a->ctrl)->moveto(delta);

    ControlManager::getManager().track(a->ctrl);

    return a;
}

/**
 * Destroys the anchor's canvas item and frees the anchor object.
 */
SPDrawAnchor *sp_draw_anchor_destroy(SPDrawAnchor *anchor)
{
    if (anchor->curve) {
        anchor->curve->unref();
    }
    if (anchor->ctrl) {
        sp_canvas_item_destroy(anchor->ctrl);
    }
    g_free(anchor);
    return NULL;
}

/**
 * Test if point is near anchor, if so fill anchor on canvas and return
 * pointer to it or NULL.
 */
SPDrawAnchor *sp_draw_anchor_test(SPDrawAnchor *anchor, Geom::Point w, bool activate)
{
    SPCtrl *ctrl = SP_CTRL(anchor->ctrl);

    if ( activate && ( Geom::LInfty( w - anchor->dc->getDesktop().d2w(anchor->dp) ) <= (ctrl->box.width() / 2.0) ) ) {
        if (!anchor->active) {
            ControlManager::getManager().setControlResize(anchor->ctrl, 4);
            g_object_set(anchor->ctrl, "fill_color", FILL_COLOR_MOUSEOVER, NULL);
            anchor->active = TRUE;
        }
        return anchor;
    }

    if (anchor->active) {
        ControlManager::getManager().setControlResize(anchor->ctrl, 0);
        g_object_set(anchor->ctrl, "fill_color", FILL_COLOR_NORMAL, NULL);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
