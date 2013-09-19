#ifndef SEEN_SP_DRAW_CONTEXT_H
#define SEEN_SP_DRAW_CONTEXT_H

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

#include <stddef.h>
#include <sigc++/sigc++.h>
#include <2geom/point.h>
#include "event-context.h"
#include "live_effects/effect.h"

/* Freehand context */

#define SP_DRAW_CONTEXT(obj) (dynamic_cast<SPDrawContext*>((SPEventContext*)obj))
#define SP_IS_DRAW_CONTEXT(obj) (dynamic_cast<const SPDrawContext*>((const SPEventContext*)obj) != NULL)

struct SPDrawAnchor;
namespace Inkscape
{
  class Selection;
}

class SPDrawContext : public SPEventContext {
public:
	SPDrawContext();
	virtual ~SPDrawContext();

    Inkscape::Selection *selection;
    SPCanvasItem *grab;

    guint attach : 1;

    guint32 red_color;
    guint32 blue_color;
    guint32 green_color;

    // Red
    SPCanvasItem *red_bpath;
    SPCurve *red_curve;

    // Blue
    SPCanvasItem *blue_bpath;
    SPCurve *blue_curve;

    // Green
    GSList *green_bpaths;
    SPCurve *green_curve;
    SPDrawAnchor *green_anchor;
    gboolean green_closed; // a flag meaning we hit the green anchor, so close the path on itself

    // White
    SPItem *white_item;
    GSList *white_curves;
    GSList *white_anchors;

    // Start anchor
    SPDrawAnchor *sa;

    // End anchor
    SPDrawAnchor *ea;

    /* type of the LPE that is to be applied automatically to a finished path (if any) */
    Inkscape::LivePathEffect::EffectType waiting_LPE_type;

    sigc::connection sel_changed_connection;
    sigc::connection sel_modified_connection;

    bool red_curve_is_valid;

    bool anchor_statusbar;

protected:
	virtual void setup();
	virtual void finish();
	virtual void set(const Inkscape::Preferences::Entry& val);
	virtual bool root_handler(GdkEvent* event);
};

/**
 * Returns FIRST active anchor (the activated one).
 */
SPDrawAnchor *spdc_test_inside(SPDrawContext *dc, Geom::Point p);

/**
 * Concats red, blue and green.
 * If any anchors are defined, process these, optionally removing curves from white list
 * Invoke _flush_white to write result back to object.
 */
void spdc_concat_colors_and_flush(SPDrawContext *dc, gboolean forceclosed);

/**
 *  Snaps node or handle to PI/rotationsnapsperpi degree increments.
 *
 *  @param dc draw context.
 *  @param p cursor point (to be changed by snapping).
 *  @param o origin point.
 *  @param state  keyboard state to check if ctrl or shift was pressed.
 */
void spdc_endpoint_snap_rotation(SPEventContext const *const ec, Geom::Point &p, Geom::Point const &o, guint state);

void spdc_endpoint_snap_free(SPEventContext const *ec, Geom::Point &p, boost::optional<Geom::Point> &start_of_line, guint state);

/**
 * If we have an item and a waiting LPE, apply the effect to the item
 * (spiro spline mode is treated separately).
 */
void spdc_check_for_and_apply_waiting_LPE(SPDrawContext *dc, SPItem *item);

/**
 * Create a single dot represented by a circle.
 */
void spdc_create_single_dot(SPEventContext *ec, Geom::Point const &pt, char const *tool, guint event_state);

#endif // SEEN_SP_DRAW_CONTEXT_H

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
