/*
 * KnotHolderEntity definition.
 *
 * Authors:
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *   Abhishek Sharma
 *
 * Copyright (C) 1999-2001 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2001 Mitsuru Oka
 * Copyright (C) 2004 Monash University
 * Copyright (C) 2008 Maximilian Albert
 *
 * Released under GNU GPL
 */

#include "knot-holder-entity.h"
#include "knotholder.h"
#include "sp-item.h"
#include "style.h"
#include "preferences.h"
#include "macros.h"
#include "sp-pattern.h"
#include "snap.h"
#include "desktop.h"
#include "sp-namedview.h"
#include <2geom/affine.h>
#include <2geom/transforms.h>

int KnotHolderEntity::counter = 0;

void KnotHolderEntity::create(SPDesktop *desktop, SPItem *item, KnotHolder *parent, Inkscape::ControlType type,
                              const gchar *tip,
                              SPKnotShapeType shape, SPKnotModeType mode, guint32 color)
{
    knot = new SPKnot(desktop, tip);

    this->parent_holder = parent;
    this->item = item; // TODO: remove the item either from here or from knotholder.cpp
    this->desktop = desktop;

    my_counter = KnotHolderEntity::counter++;

    g_object_set(G_OBJECT(knot->item), "shape", shape, NULL);
    g_object_set(G_OBJECT(knot->item), "mode", mode, NULL);

    // TODO base more appearance from this type instead of passing in arbitrary values.
    knot->item->ctrlType = type;

    knot->fill [SP_KNOT_STATE_NORMAL] = color;
    g_object_set (G_OBJECT(knot->item), "fill_color", color, NULL);

    update_knot();
    knot->show();

    _moved_connection = knot->moved_signal.connect(sigc::mem_fun(*parent_holder, &KnotHolder::knot_moved_handler));
    _click_connection = knot->click_signal.connect(sigc::mem_fun(*parent_holder, &KnotHolder::knot_clicked_handler));
    _ungrabbed_connection = knot->ungrabbed_signal.connect(sigc::mem_fun(*parent_holder, &KnotHolder::knot_ungrabbed_handler));
}


KnotHolderEntity::~KnotHolderEntity()
{
    _moved_connection.disconnect();
    _click_connection.disconnect();
    _ungrabbed_connection.disconnect();

    /* unref should call destroy */
    if (knot) {
        //g_object_unref(knot);
        knot_unref(knot);
    } else {
        // FIXME: This shouldn't occur. Perhaps it is caused by LPE PointParams being knotholder entities, too
        //        If so, it will likely be fixed with upcoming refactoring efforts.
        g_return_if_fail(knot);
    }
}

void
KnotHolderEntity::update_knot()
{
    Geom::Point knot_pos(knot_get());
    if (knot_pos.isFinite()) {
        Geom::Point dp(knot_pos * item->i2dt_affine());

        _moved_connection.block();
        knot->setPosition(dp, SP_KNOT_STATE_NORMAL);
        _moved_connection.unblock();
    } else {
        // knot coords are non-finite, hide knot
        knot->hide();
    }
}

Geom::Point
KnotHolderEntity::snap_knot_position(Geom::Point const &p, guint state)
{
    if (state & GDK_SHIFT_MASK) { // Don't snap when shift-key is held
        return p;
    }

    Geom::Affine const i2dt (item->i2dt_affine());
    Geom::Point s = p * i2dt;

    SnapManager &m = desktop->namedview->snap_manager;
    m.setup(desktop, true, item);
    m.freeSnapReturnByRef(s, Inkscape::SNAPSOURCE_NODE_HANDLE);
    m.unSetup();

    return s * i2dt.inverse();
}

Geom::Point
KnotHolderEntity::snap_knot_position_constrained(Geom::Point const &p, Inkscape::Snapper::SnapConstraint const &constraint, guint state)
{
    if (state & GDK_SHIFT_MASK) { // Don't snap when shift-key is held
        return p;
    }

    Geom::Affine const i2d (item->i2dt_affine());
    Geom::Point s = p * i2d;

    SnapManager &m = desktop->namedview->snap_manager;
    m.setup(desktop, true, item);

    // constrainedSnap() will first project the point p onto the constraint line and then try to snap along that line.
    // This way the constraint is already enforced, no need to worry about that later on
    Inkscape::Snapper::SnapConstraint transformed_constraint = Inkscape::Snapper::SnapConstraint(constraint.getPoint() * i2d, (constraint.getPoint() + constraint.getDirection()) * i2d - constraint.getPoint() * i2d);
    m.constrainedSnapReturnByRef(s, Inkscape::SNAPSOURCE_NODE_HANDLE, transformed_constraint);
    m.unSetup();

    return s * i2d.inverse();
}


/* Pattern manipulation */

/*  TODO: this pattern manipulation is not able to handle general transformation matrices. Only matrices that are the result of a pure scale times a pure rotation. */

static gdouble sp_pattern_extract_theta(SPPattern const *pat)
{
    Geom::Affine transf = pat->getTransform();
    return Geom::atan2(transf.xAxis());
}

static Geom::Point sp_pattern_extract_scale(SPPattern const *pat)
{
    Geom::Affine transf = pat->getTransform();
    return Geom::Point( transf.expansionX(), transf.expansionY() );
}

static Geom::Point sp_pattern_extract_trans(SPPattern const *pat)
{
    return Geom::Point(pat->getTransform()[4], pat->getTransform()[5]);
}

void
PatternKnotHolderEntityXY::knot_set(Geom::Point const &p, Geom::Point const &origin, guint state)
{
    SPPattern *pat = _fill ? SP_PATTERN(item->style->getFillPaintServer()) : SP_PATTERN(item->style->getStrokePaintServer());

    // FIXME: this snapping should be done together with knowing whether control was pressed. If GDK_CONTROL_MASK, then constrained snapping should be used.
    Geom::Point p_snapped = snap_knot_position(p, state);

    if ( state & GDK_CONTROL_MASK ) {
        if (fabs((p - origin)[Geom::X]) > fabs((p - origin)[Geom::Y])) {
            p_snapped[Geom::Y] = origin[Geom::Y];
        } else {
            p_snapped[Geom::X] = origin[Geom::X];
        }
    }

    if (state)  {
        Geom::Point const q = p_snapped - sp_pattern_extract_trans(pat);
        item->adjust_pattern(Geom::Translate(q), false, _fill ? TRANSFORM_FILL : TRANSFORM_STROKE);
    }

    item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

Geom::Point
PatternKnotHolderEntityXY::knot_get() const
{
    SPPattern *pat = _fill ? SP_PATTERN(item->style->getFillPaintServer()) : SP_PATTERN(item->style->getStrokePaintServer());
    return sp_pattern_extract_trans(pat);
}

Geom::Point
PatternKnotHolderEntityAngle::knot_get() const
{
    SPPattern *pat = _fill ? SP_PATTERN(item->style->getFillPaintServer()) : SP_PATTERN(item->style->getStrokePaintServer());

    gdouble x = pat->width();
    gdouble y = 0;
    Geom::Point delta = Geom::Point(x,y);
    Geom::Point scale = sp_pattern_extract_scale(pat);
    gdouble theta = sp_pattern_extract_theta(pat);
    delta = delta * Geom::Affine(Geom::Scale(scale))*Geom::Affine(Geom::Rotate(theta));
    delta = delta + sp_pattern_extract_trans(pat);
    return delta;
}

void
PatternKnotHolderEntityAngle::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint state)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int const snaps = prefs->getInt("/options/rotationsnapsperpi/value", 12);

    SPPattern *pat = _fill ? SP_PATTERN(item->style->getFillPaintServer()) : SP_PATTERN(item->style->getStrokePaintServer());

    // get the angle from pattern 0,0 to the cursor pos
    Geom::Point delta = p - sp_pattern_extract_trans(pat);
    gdouble theta = atan2(delta);

    if ( state & GDK_CONTROL_MASK ) {
        theta = sp_round(theta, M_PI/snaps);
    }

    // get the scale from the current transform so we can keep it.
    Geom::Point scl = sp_pattern_extract_scale(pat);
    Geom::Affine rot = Geom::Affine(Geom::Scale(scl)) * Geom::Affine(Geom::Rotate(theta));
    Geom::Point const t = sp_pattern_extract_trans(pat);
    rot[4] = t[Geom::X];
    rot[5] = t[Geom::Y];
    item->adjust_pattern(rot, true, _fill ? TRANSFORM_FILL : TRANSFORM_STROKE);
    item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

void
PatternKnotHolderEntityScale::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint state)
{
    SPPattern *pat = _fill ? SP_PATTERN(item->style->getFillPaintServer()) : SP_PATTERN(item->style->getStrokePaintServer());

    // FIXME: this snapping should be done together with knowing whether control was pressed. If GDK_CONTROL_MASK, then constrained snapping should be used.
    Geom::Point p_snapped = snap_knot_position(p, state);

    // get angle from current transform
    gdouble theta = sp_pattern_extract_theta(pat);

    // Get the new scale from the position of the knotholder
    Geom::Point d = p_snapped - sp_pattern_extract_trans(pat);
    gdouble pat_x = pat->width();
    gdouble pat_y = pat->height();
    Geom::Scale scl(1);
    if ( state & GDK_CONTROL_MASK ) {
        // if ctrl is pressed: use 1:1 scaling
        gdouble pat_h = hypot(pat_x, pat_y);
        scl = Geom::Scale(d.length() / pat_h);
    } else {
        d *= Geom::Rotate(-theta);
        scl = Geom::Scale(d[Geom::X] / pat_x, d[Geom::Y] / pat_y);
    }

    Geom::Affine rot = (Geom::Affine)scl * Geom::Rotate(theta);

    Geom::Point const t = sp_pattern_extract_trans(pat);
    rot[4] = t[Geom::X];
    rot[5] = t[Geom::Y];
    item->adjust_pattern(rot, true, _fill ? TRANSFORM_FILL : TRANSFORM_STROKE);
    item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}


Geom::Point
PatternKnotHolderEntityScale::knot_get() const
{
    SPPattern *pat = _fill ? SP_PATTERN(item->style->getFillPaintServer()) : SP_PATTERN(item->style->getStrokePaintServer());

    gdouble x = pat->width();
    gdouble y = pat->height();
    Geom::Point delta = Geom::Point(x,y);
    Geom::Affine a = pat->getTransform();
    a[4] = 0;
    a[5] = 0;
    delta = delta * a;
    delta = delta + sp_pattern_extract_trans(pat);
    return delta;
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
