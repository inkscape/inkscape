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
#include "prefs-utils.h"
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
    NR::Matrix const i2d(from_2geom(sp_item_i2d_affine(item)));

    Geom::Point dp(knot_get() * i2d);

    _moved_connection.block();
    sp_knot_set_position(knot, dp, SP_KNOT_STATE_NORMAL); 
    _moved_connection.unblock();
}

Geom::Point
KnotHolderEntity::snap_knot_position(Geom::Point const &p)
{
    NR::Matrix const i2d (from_2geom(sp_item_i2d_affine(item)));
    NR::Point s = p * i2d;
    SnapManager &m = desktop->namedview->snap_manager;
    m.setup(desktop, item);
    m.freeSnapReturnByRef(Inkscape::Snapper::SNAPPOINT_NODE, s);
    return s * i2d.inverse();
}


/* Pattern manipulation */

static gdouble sp_pattern_extract_theta(SPPattern *pat, Geom::Point scale)
{
    gdouble theta = asin(pat->patternTransform[1] / scale[NR::X]);
    if (pat->patternTransform[0] < 0) theta = M_PI - theta ;
    return theta;
}

static Geom::Point sp_pattern_extract_scale(SPPattern *pat)
{
    Geom::Matrix transf = to_2geom(pat->patternTransform);
    return Geom::Point( transf.expansionX(), transf.expansionY() );
}

static Geom::Point sp_pattern_extract_trans(SPPattern const *pat)
{
    return Geom::Point(pat->patternTransform[4], pat->patternTransform[5]);
}

void
PatternKnotHolderEntityXY::knot_set(Geom::Point const &p, Geom::Point const &origin, guint state)
{
    SPPattern *pat = SP_PATTERN(SP_STYLE_FILL_SERVER(SP_OBJECT(item)->style));

    Geom::Point p_snapped = snap_knot_position(p);

    if ( state & GDK_CONTROL_MASK ) {
        if (fabs((p - origin)[NR::X]) > fabs((p - origin)[NR::Y])) {
            p_snapped[NR::Y] = origin[NR::Y];
        } else {
            p_snapped[NR::X] = origin[NR::X];
        }
    }

    if (state)  {
        Geom::Point const q = p_snapped - sp_pattern_extract_trans(pat);
        sp_item_adjust_pattern(item, NR::Matrix(NR::translate(q)));
    }

    item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

Geom::Point
PatternKnotHolderEntityXY::knot_get()
{
    SPPattern const *pat = SP_PATTERN(SP_STYLE_FILL_SERVER(SP_OBJECT(item)->style));
    return sp_pattern_extract_trans(pat);
}

Geom::Point
PatternKnotHolderEntityAngle::knot_get()
{
    SPPattern *pat = SP_PATTERN(SP_STYLE_FILL_SERVER(SP_OBJECT(item)->style));

    gdouble x = (pattern_width(pat));
    gdouble y = 0;
    Geom::Point delta = Geom::Point(x,y);
    Geom::Point scale = sp_pattern_extract_scale(pat);
    gdouble theta = sp_pattern_extract_theta(pat, scale);
    delta = delta * NR::Matrix(NR::scale(scale))*NR::Matrix(NR::rotate(theta));
    delta = delta + sp_pattern_extract_trans(pat);
    return delta;
}

void
PatternKnotHolderEntityAngle::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint state)
{
    int const snaps = prefs_get_int_attribute("options.rotationsnapsperpi", "value", 12);

    SPPattern *pat = SP_PATTERN(SP_STYLE_FILL_SERVER(SP_OBJECT(item)->style));

    // get the angle from pattern 0,0 to the cursor pos
    Geom::Point delta = p - sp_pattern_extract_trans(pat);
    gdouble theta = atan2(delta);

    if ( state & GDK_CONTROL_MASK ) {
        theta = sp_round(theta, M_PI/snaps);
    }

    // get the scale from the current transform so we can keep it.
    Geom::Point scl = sp_pattern_extract_scale(pat);
    NR::Matrix rot = NR::Matrix(NR::scale(scl)) * NR::Matrix(NR::rotate(theta));
    Geom::Point const t = sp_pattern_extract_trans(pat);
    rot[4] = t[NR::X];
    rot[5] = t[NR::Y];
    sp_item_adjust_pattern(item, rot, true);
    item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

void
PatternKnotHolderEntityScale::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint /*state*/)
{
    SPPattern *pat = SP_PATTERN(SP_STYLE_FILL_SERVER(SP_OBJECT(item)->style));

    Geom::Point p_snapped = snap_knot_position(p);

    // get angle from current transform, (need get current scale first to calculate angle)
    Geom::Point oldscale = sp_pattern_extract_scale(pat);
    gdouble theta = sp_pattern_extract_theta(pat,oldscale);

    // Get the new scale from the position of the knotholder
    Geom::Point d = p_snapped - sp_pattern_extract_trans(pat);
    d *= Geom::Rotate(-theta);
    gdouble pat_x = pattern_width(pat);
    gdouble pat_y = pattern_height(pat);
    Geom::Point scl (d[NR::X] / pat_x, d[NR::Y] / pat_y);

    NR::Matrix rot =  NR::Matrix(NR::scale(scl)) * NR::Matrix(NR::rotate(theta));

    Geom::Point const t = sp_pattern_extract_trans(pat);
    rot[4] = t[NR::X];
    rot[5] = t[NR::Y];
    sp_item_adjust_pattern(item, rot, true);
    item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}


Geom::Point
PatternKnotHolderEntityScale::knot_get()
{
    SPPattern *pat = SP_PATTERN(SP_STYLE_FILL_SERVER(SP_OBJECT(item)->style));

    gdouble x = pattern_width(pat);
    gdouble y = pattern_height(pat);
    Geom::Point delta = Geom::Point(x,y);
    NR::Matrix a = pat->patternTransform;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
