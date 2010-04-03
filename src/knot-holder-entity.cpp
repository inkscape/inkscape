#define __KNOT_HOLDER_ENTITY_C__

/** \file
 * KnotHolderEntity definition.
 *
 * Authors:
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *
 * Copyright (C) 1999-2001 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2001 Mitsuru Oka
 * Copyright (C) 2004 Monash University
 * Copyright (C) 2008 Maximilian Albert
 *
 * Released under GNU GPL
 */

#include "knotholder.h"
#include "sp-item.h"
#include "style.h"
#include "preferences.h"
#include "macros.h"
#include <libnr/nr-matrix-ops.h>
#include "sp-pattern.h"
#include "snap.h"
#include "desktop.h"
#include "sp-namedview.h"
#include <2geom/matrix.h>
#include <2geom/transforms.h>

int KnotHolderEntity::counter = 0;

void
KnotHolderEntity::create(SPDesktop *desktop, SPItem *item, KnotHolder *parent, const gchar *tip,
                         SPKnotShapeType shape, SPKnotModeType mode, guint32 color)
{
    knot = sp_knot_new(desktop, tip);

    this->parent_holder = parent;
    this->item = item; // TODO: remove the item either from here or from knotholder.cpp
    this->desktop = desktop;

    my_counter = KnotHolderEntity::counter++;

    g_object_set(G_OBJECT (knot->item), "shape", shape, NULL);
    g_object_set(G_OBJECT (knot->item), "mode", mode, NULL);

    knot->fill [SP_KNOT_STATE_NORMAL] = color;
    g_object_set (G_OBJECT (knot->item), "fill_color", color, NULL);

    update_knot();
    sp_knot_show(knot);

    _moved_connection = knot->_moved_signal.connect(sigc::mem_fun(*parent_holder, &KnotHolder::knot_moved_handler));
    _click_connection = knot->_click_signal.connect(sigc::mem_fun(*parent_holder, &KnotHolder::knot_clicked_handler));
    _ungrabbed_connection = knot->_ungrabbed_signal.connect(sigc::mem_fun(*parent_holder, &KnotHolder::knot_ungrabbed_handler));
}


KnotHolderEntity::~KnotHolderEntity()
{
    _moved_connection.disconnect();
    _click_connection.disconnect();
    _ungrabbed_connection.disconnect();

    /* unref should call destroy */
    if (knot) {
        g_object_unref(knot);
    } else {
        // FIXME: This shouldn't occur. Perhaps it is caused by LPE PointParams being knotholder entities, too
        //        If so, it will likely be fixed with upcoming refactoring efforts.
        g_return_if_fail(knot);
    }
}

void
KnotHolderEntity::update_knot()
{
    Geom::Matrix const i2d(sp_item_i2d_affine(item));

    Geom::Point dp(knot_get() * i2d);

    _moved_connection.block();
    sp_knot_set_position(knot, dp, SP_KNOT_STATE_NORMAL);
    _moved_connection.unblock();
}

Geom::Point
KnotHolderEntity::snap_knot_position(Geom::Point const &p)
{
    Geom::Matrix const i2d (sp_item_i2d_affine(item));
    Geom::Point s = p * i2d;

    SnapManager &m = desktop->namedview->snap_manager;
    m.setup(desktop, true, item);

    m.freeSnapReturnByRef(s, Inkscape::SNAPSOURCE_NODE_HANDLE);

    return s * i2d.inverse();
}

Geom::Point
KnotHolderEntity::snap_knot_position_constrained(Geom::Point const &p, Inkscape::Snapper::ConstraintLine const &constraint)
{
    Geom::Matrix const i2d (sp_item_i2d_affine(item));
    Geom::Point s = p * i2d;

    SnapManager &m = desktop->namedview->snap_manager;
    m.setup(desktop, true, item);

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if ((prefs->getBool("/options/snapmousepointer/value", false))) { // legacy behavior (pre v0.47)
        // Snapping the mouse pointer instead of the constrained position of the knot allows to snap to
        // things which don't intersect with the constraint line. This should be handled by the
        // smart dynamic guides which are yet to be implemented, making this behavior more clean and
        // transparent. With the current implementation it leads to unexpected results, and it doesn't
        // allow accurately controlling what is being snapped to.

        // freeSnap() will try snapping point p. This will not take into account the constraint, which
        // is therefore to be enforced after snap_knot_position_constrained() has finished
        m.freeSnapReturnByRef(s, Inkscape::SNAPSOURCE_NODE_HANDLE);
    } else {
        // constrainedSnap() will first project the point p onto the constraint line and then try to snap along that line.
        // This way the constraint is already enforced, no need to worry about that later on
        Inkscape::Snapper::ConstraintLine transformed_constraint = Inkscape::Snapper::ConstraintLine(constraint.getPoint() * i2d, (constraint.getPoint() + constraint.getDirection()) * i2d - constraint.getPoint() * i2d);
        m.constrainedSnapReturnByRef(s, Inkscape::SNAPSOURCE_NODE_HANDLE, transformed_constraint);
    }

    return s * i2d.inverse();
}


/* Pattern manipulation */

/*  TODO: this pattern manipulation is not able to handle general transformation matrices. Only matrices that are the result of a pure scale times a pure rotation. */

Geom::Point
PatternKnotHolderEntityXY::knot_get()
{
    SPPattern *pat = SP_PATTERN(SP_STYLE_FILL_SERVER(SP_OBJECT(item)->style));

    gdouble x = 0;
    gdouble y = -pattern_height(pat);

    Geom::Point delta = Geom::Point(x,y) * pat->patternTransform;
    return delta;
}

void
PatternKnotHolderEntityXY::knot_set(Geom::Point const &p, Geom::Point const &origin, guint state)
{
    SPPattern *pat = SP_PATTERN(SP_STYLE_FILL_SERVER(SP_OBJECT(item)->style));

    // FIXME: this snapping should be done together with knowing whether control was pressed. If GDK_CONTROL_MASK, then constrained snapping should be used.
    Geom::Point p_snapped = snap_knot_position(p);

    if ( state & GDK_CONTROL_MASK ) {
        if (fabs((p - origin)[Geom::X]) > fabs((p - origin)[Geom::Y])) {
            p_snapped[Geom::Y] = origin[Geom::Y];
        } else {
            p_snapped[Geom::X] = origin[Geom::X];
        }
    }

    if (state)  {
        Geom::Point knot_relpos(0, -pattern_height(pat));
        Geom::Point const q = p_snapped - (knot_relpos * pat->patternTransform);
        sp_item_adjust_pattern(item, Geom::Matrix(Geom::Translate(q)));
    }

    item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

Geom::Point
PatternKnotHolderEntityAngle::knot_get()
{
    SPPattern *pat = SP_PATTERN(SP_STYLE_FILL_SERVER(SP_OBJECT(item)->style));

    gdouble x = pattern_width(pat);
    gdouble y = -pattern_height(pat);

    Geom::Point delta = Geom::Point(x,y) * pat->patternTransform;
    return delta;
}

void
PatternKnotHolderEntityAngle::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint state)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int const snaps = prefs->getInt("/options/rotationsnapsperpi/value", 12);

    SPPattern *pat = SP_PATTERN(SP_STYLE_FILL_SERVER(SP_OBJECT(item)->style));

    // rotate pattern around XY knot position
    Geom::Point knot_relpos(pattern_width(pat), -pattern_height(pat));
    Geom::Point xy_knot_relpos(0, -pattern_height(pat));
    Geom::Point transform_origin = xy_knot_relpos * pat->patternTransform;

    Geom::Point oldp = (knot_relpos * pat->patternTransform) - transform_origin;
    Geom::Point newp = p - transform_origin;

    gdouble theta = Geom::angle_between(oldp, newp);

    if ( state & GDK_CONTROL_MASK ) {
        theta = sp_round(theta, M_PI/snaps);
    }

    Geom::Matrix rot = Geom::Matrix(Geom::Translate(-transform_origin))
                     * Geom::Rotate(theta)
                     * Geom::Translate(transform_origin);
    sp_item_adjust_pattern(item, rot);
    item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

Geom::Point
PatternKnotHolderEntityScale::knot_get()
{
    SPPattern *pat = SP_PATTERN(SP_STYLE_FILL_SERVER(SP_OBJECT(item)->style));

    gdouble x = pattern_width(pat);
    gdouble y = 0;
    Geom::Point delta = Geom::Point(x,y) * pat->patternTransform;
    return delta;
}

void
PatternKnotHolderEntityScale::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint state)
{
    SPPattern *pat = SP_PATTERN(SP_STYLE_FILL_SERVER(SP_OBJECT(item)->style));

    // FIXME: this snapping should be done together with knowing whether control was pressed. If GDK_CONTROL_MASK, then constrained snapping should be used.
    Geom::Point p_snapped = snap_knot_position(p);

    Geom::Point knot_relpos(pattern_width(pat), 0);
    Geom::Point xy_knot_relpos(0, -pattern_height(pat));
    Geom::Point transform_origin = xy_knot_relpos * pat->patternTransform;

    // do the scaling in pattern coordinate space
    Geom::Point oldp = knot_relpos - xy_knot_relpos;
    Geom::Point newp = p_snapped * pat->patternTransform.inverse() - xy_knot_relpos;

    if (Geom::are_near(newp.length(), 0)) return;

    Geom::Scale s(1);
    if (state & GDK_CONTROL_MASK) {
        // uniform scaling
        s = Geom::Scale(oldp * (newp.length() * oldp.length()));
    } else {
        s = Geom::Scale(newp[Geom::X] / oldp[Geom::X], newp[Geom::Y] / oldp[Geom::Y]);
    }

    Geom::Matrix scl = Geom::Matrix(Geom::Translate(-xy_knot_relpos))
                     * s
                     * Geom::Translate(xy_knot_relpos)
                     * pat->patternTransform;
    sp_item_adjust_pattern(item, scl, true);
    item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
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
