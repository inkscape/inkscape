#ifndef __SP_DRAW_CONTEXT_H__
#define __SP_DRAW_CONTEXT_H__

/*
 * Generic drawing context
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2000 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL
 */

#include <sigc++/sigc++.h>
#include "event-context.h"
#include <forward.h>
#include <display/display-forward.h>
#include <libnr/nr-point.h>
#include "live_effects/effect.h"

/* Freehand context */

#define SP_TYPE_DRAW_CONTEXT (sp_draw_context_get_type())
#define SP_DRAW_CONTEXT(o) (GTK_CHECK_CAST((o), SP_TYPE_DRAW_CONTEXT, SPDrawContext))
#define SP_DRAW_CONTEXT_CLASS(k) (GTK_CHECK_CLASS_CAST((k), SP_TYPE_DRAW_CONTEXT, SPDrawContextClass))
#define SP_IS_DRAW_CONTEXT(o) (GTK_CHECK_TYPE((o), SP_TYPE_DRAW_CONTEXT))
#define SP_IS_DRAW_CONTEXT_CLASS(k) (GTK_CHECK_CLASS_TYPE((k), SP_TYPE_DRAW_CONTEXT))

struct SPDrawAnchor;
namespace Inkscape
{
  class Selection;
}

struct SPDrawContext : public SPEventContext{
    Inkscape::Selection *selection;
    SPCanvasItem *grab;

    guint attach : 1;

    guint32 red_color;
    guint32 blue_color;
    guint32 green_color;

    /* Red */
    SPCanvasItem *red_bpath;
    SPCurve *red_curve;

    /* Blue */
    SPCanvasItem *blue_bpath;
    SPCurve *blue_curve;

    /* Green */
    GSList *green_bpaths;
    SPCurve *green_curve;
    SPDrawAnchor *green_anchor;
    gboolean green_closed; // a flag meaning we hit the green anchor, so close the path on itself

    /* White */
    SPItem *white_item;
    GSList *white_curves;
    GSList *white_anchors;

    /* Start anchor */
    SPDrawAnchor *sa;
    /* End anchor */
    SPDrawAnchor *ea;

    /* type of the LPE that is to be applied automatically to a finished path (if any) */
    Inkscape::LivePathEffect::EffectType waiting_LPE_type;

    sigc::connection sel_changed_connection;
    sigc::connection sel_modified_connection;

    bool red_curve_is_valid;

    bool anchor_statusbar;
};

struct SPDrawContextClass : public SPEventContextClass{};

GType sp_draw_context_get_type(void);
SPDrawAnchor *spdc_test_inside(SPDrawContext *dc, NR::Point p);
void spdc_concat_colors_and_flush(SPDrawContext *dc, gboolean forceclosed);
void spdc_endpoint_snap_rotation(SPEventContext const *const ec, NR::Point &p, NR::Point const o, guint state);
void spdc_endpoint_snap_free(SPEventContext const *ec, NR::Point &p, guint state);
void spdc_check_for_and_apply_waiting_LPE(SPDrawContext *dc, SPItem *item);

#endif

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
