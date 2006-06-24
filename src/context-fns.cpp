extern "C" {
#include <libintl.h>
}
#include <glibmm/i18n.h>
#include "sp-item.h"
#include "desktop.h"
#include "message-context.h"
#include "message-stack.h"
#include "context-fns.h"
#include "snap.h"
#include "desktop-affine.h"
#include "event-context.h"
#include "sp-namedview.h"

/* FIXME: could probably use a template here */

/**
 *  Check to see if the current layer is both unhidden and unlocked.  If not,
 *  set a message about it on the given context.
 *
 *  \param desktop Desktop.
 *  \param message Message context to put messages on.
 *  \return true if the current layer is both unhidden and unlocked, otherwise false.
 */

bool Inkscape::have_viable_layer(SPDesktop *desktop, MessageContext *message)
{
    SPItem const *layer = SP_ITEM(desktop->currentLayer());

    if ( !layer || desktop->itemIsHidden(layer) ) {
            message->flash(Inkscape::ERROR_MESSAGE,
                         _("<b>Current layer is hidden</b>. Unhide it to be able to draw on it."));
            return false;
    }

    if ( !layer || layer->isLocked() ) {
            message->flash(Inkscape::ERROR_MESSAGE,
                         _("<b>Current layer is locked</b>. Unlock it to be able to draw on it."));
            return false;
    }

    return true;
}


/**
 *  Check to see if the current layer is both unhidden and unlocked.  If not,
 *  set a message about it on the given context.
 *
 *  \param desktop Desktop.
 *  \param message Message context to put messages on.
 *  \return true if the current layer is both unhidden and unlocked, otherwise false.
 */

bool Inkscape::have_viable_layer(SPDesktop *desktop, MessageStack *message)
{
    SPItem const *layer = SP_ITEM(desktop->currentLayer());

    if ( !layer || desktop->itemIsHidden(layer) ) {
            message->flash(Inkscape::WARNING_MESSAGE,
                         _("<b>Current layer is hidden</b>. Unhide it to be able to draw on it."));
            return false;
    }

    if ( !layer || layer->isLocked() ) {
            message->flash(Inkscape::WARNING_MESSAGE,
                         _("<b>Current layer is locked</b>. Unlock it to be able to draw on it."));
            return false;
    }

    return true;
}


NR::Rect Inkscape::snap_rectangular_box(SPDesktop const *desktop, SPItem *item,
                                        NR::Point const &pt, NR::Point const &center, int state)
{
    NR::Point p[2];

    bool const shift = state & GDK_SHIFT_MASK;
    bool const control = state & GDK_CONTROL_MASK;

    SnapManager const &m = desktop->namedview->snap_manager;

    if (control) {

        /* Control is down: we are constrained to producing integer-ratio rectangles */

        /* Vector from the centre of the box to the point we are dragging to */
        NR::Point delta = pt - center;

        /* Round it so that we have an integer-ratio box */
        if (fabs(delta[NR::X]) > fabs(delta[NR::Y]) && (delta[NR::Y] != 0.0)) {
            delta[NR::X] = floor(delta[NR::X] / delta[NR::Y] + 0.5) * delta[NR::Y];
        } else if (delta[NR::X] != 0.0) {
            delta[NR::Y] = floor(delta[NR::Y] / delta[NR::X] + 0.5) * delta[NR::X];
        }

        /* p[1] is the dragged point with the integer-ratio constraint */
        p[1] = center + delta;

        if (shift) {

            /* Shift is down, so our origin is the centre point rather than the corner
            ** point; this means that corner-point movements are bound to each other.
            */

            /* p[0] is the opposite corner of our box */
            p[0] = center - delta;

            Inkscape::SnappedPoint s[2];

            /* Try to snap p[0] (the opposite corner) along the constraint vector */
            s[0] = m.constrainedSnap(Inkscape::Snapper::SNAP_POINT, p[0],
                                     Inkscape::Snapper::ConstraintLine(p[0] - p[1]), item);

            /* Try to snap p[1] (the dragged corner) along the constraint vector */
            s[1] = m.constrainedSnap(Inkscape::Snapper::SNAP_POINT, p[1],
                                     Inkscape::Snapper::ConstraintLine(p[1] - p[0]), item);

            /* Choose the best snap and update points accordingly */
            if (s[0].getDistance() < s[1].getDistance()) {
                p[0] = s[0].getPoint();
                p[1] = 2 * center - s[0].getPoint();
            } else {
                p[0] = 2 * center - s[1].getPoint();
                p[1] = s[1].getPoint();
            }

        } else {

            /* Our origin is the opposite corner.  Snap the drag point along the constraint vector */
            p[0] = center;
            p[1] = m.constrainedSnap(Inkscape::Snapper::SNAP_POINT, p[1],
                                     Inkscape::Snapper::ConstraintLine(p[1] - p[0]), item).getPoint();
        }

    } else if (shift) {

        /* Shift is down, so our origin is the centre point rather than the corner point;
        ** this means that corner-point movements are bound to each other.
        */

        p[1] = pt;
        p[0] = 2 * center - p[1];

        Inkscape::SnappedPoint s[2];

        s[0] = m.freeSnap(Inkscape::Snapper::SNAP_POINT, p[0], item);
        s[1] = m.freeSnap(Inkscape::Snapper::SNAP_POINT, p[1], item);

        if (s[0].getDistance() < s[1].getDistance()) {
            p[0] = s[0].getPoint();
            p[1] = 2 * center - s[0].getPoint();
        } else {
            p[0] = 2 * center - s[1].getPoint();
            p[1] = s[1].getPoint();
        }

    } else {

        /* There's no constraint on the corner point, so just snap it to anything */
        p[0] = center;
        p[1] = m.freeSnap(Inkscape::Snapper::SNAP_POINT, pt, item).getPoint();
    }

    p[0] = sp_desktop_dt2root_xy_point(desktop, p[0]);
    p[1] = sp_desktop_dt2root_xy_point(desktop, p[1]);

    return NR::Rect(NR::Point(MIN(p[0][NR::X], p[1][NR::X]), MIN(p[0][NR::Y], p[1][NR::Y])),
                    NR::Point(MAX(p[0][NR::X], p[1][NR::X]), MAX(p[0][NR::Y], p[1][NR::Y])));
}



NR::Point Inkscape::setup_for_drag_start(SPDesktop *desktop, SPEventContext* ec, GdkEvent *ev)
{
    ec->xp = static_cast<gint>(ev->button.x);
    ec->yp = static_cast<gint>(ev->button.y);
    ec->within_tolerance = true;

    NR::Point const p(ev->button.x, ev->button.y);
    ec->item_to_select = sp_event_context_find_item(desktop, p, ev->button.state & GDK_MOD1_MASK, TRUE);
    return ec->desktop->w2d(p);
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
