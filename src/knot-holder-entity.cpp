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


int KnotHolderEntity::counter = 0;

void
KnotHolderEntity::create(SPDesktop *desktop, SPItem *item, KnotHolder *parent, const gchar *tip,
                         SPKnotShapeType shape, SPKnotModeType mode, guint32 color)
{
    knot = sp_knot_new(desktop, tip);

    this->parent_holder = parent;
    this->item = item; // TODO: remove the item either from here or from knotholder.cpp

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
    /* unref should call destroy */
    g_object_unref(knot);
}

void
KnotHolderEntity::update_knot()
{
    NR::Matrix const i2d(sp_item_i2d_affine(item));

    NR::Point dp(knot_get() * i2d);

    _moved_connection.block();
    sp_knot_set_position(knot, &dp, SP_KNOT_STATE_NORMAL); 
    _moved_connection.unblock();
}

/* Pattern manipulation */

static gdouble sp_pattern_extract_theta(SPPattern *pat, gdouble scale)
{
    gdouble theta = asin(pat->patternTransform[1] / scale);
    if (pat->patternTransform[0] < 0) theta = M_PI - theta ;
    return theta;
}

static gdouble sp_pattern_extract_scale(SPPattern *pat)
{
    gdouble s = pat->patternTransform[1];
    gdouble c = pat->patternTransform[0];
    gdouble xscale = sqrt(c * c + s * s);
    return xscale;
}

static NR::Point sp_pattern_extract_trans(SPPattern const *pat)
{
    return NR::Point(pat->patternTransform[4], pat->patternTransform[5]);
}

void
PatternKnotHolderEntityXY::knot_set(NR::Point const &p, NR::Point const &origin, guint state)
{
    SPPattern *pat = SP_PATTERN(SP_STYLE_FILL_SERVER(SP_OBJECT(item)->style));

    NR::Point p_snapped = p;

    if ( state & GDK_CONTROL_MASK ) {
        if (fabs((p - origin)[NR::X]) > fabs((p - origin)[NR::Y])) {
            p_snapped[NR::Y] = origin[NR::Y];
        } else {
            p_snapped[NR::X] = origin[NR::X];
        }
    }

    if (state)  {
        NR::Point const q = p_snapped - sp_pattern_extract_trans(pat);
        sp_item_adjust_pattern(item, NR::Matrix(NR::translate(q)));
    }

    item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

NR::Point
PatternKnotHolderEntityXY::knot_get()
{
    SPPattern const *pat = SP_PATTERN(SP_STYLE_FILL_SERVER(SP_OBJECT(item)->style));
    return sp_pattern_extract_trans(pat);
}

NR::Point
PatternKnotHolderEntityAngle::knot_get()
{
    SPPattern *pat = SP_PATTERN(SP_STYLE_FILL_SERVER(SP_OBJECT(item)->style));

    gdouble x = (pattern_width(pat)*0.5);
    gdouble y = 0;
    NR::Point delta = NR::Point(x,y);
    gdouble scale = sp_pattern_extract_scale(pat);
    gdouble theta = sp_pattern_extract_theta(pat, scale);
    delta = delta * NR::Matrix(NR::rotate(theta))*NR::Matrix(NR::scale(scale,scale));
    delta = delta + sp_pattern_extract_trans(pat);
    return delta;
}

void
PatternKnotHolderEntityAngle::knot_set(NR::Point const &p, NR::Point const &/*origin*/, guint state)
{
    int const snaps = prefs_get_int_attribute("options.rotationsnapsperpi", "value", 12);

    SPPattern *pat = SP_PATTERN(SP_STYLE_FILL_SERVER(SP_OBJECT(item)->style));

    // get the angle from pattern 0,0 to the cursor pos
    NR::Point delta = p - sp_pattern_extract_trans(pat);
    gdouble theta = atan2(delta);

    if ( state & GDK_CONTROL_MASK ) {
        theta = sp_round(theta, M_PI/snaps);
    }

    // get the scale from the current transform so we can keep it.
    gdouble scl = sp_pattern_extract_scale(pat);
    NR::Matrix rot =  NR::Matrix(NR::rotate(theta)) * NR::Matrix(NR::scale(scl,scl));
    NR::Point const t = sp_pattern_extract_trans(pat);
    rot[4] = t[NR::X];
    rot[5] = t[NR::Y];
    sp_item_adjust_pattern(item, rot, true);
    item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

void
PatternKnotHolderEntityScale::knot_set(NR::Point const &p, NR::Point const &/*origin*/, guint /*state*/)
{
    SPPattern *pat = SP_PATTERN(SP_STYLE_FILL_SERVER(SP_OBJECT(item)->style));

    // Get the scale from the position of the knotholder,
    NR::Point d = p - sp_pattern_extract_trans(pat);
    gdouble s = NR::L2(d);
    gdouble pat_x = pattern_width(pat) * 0.5;
    gdouble pat_y = pattern_height(pat) * 0.5;
    gdouble pat_h = hypot(pat_x, pat_y);
    gdouble scl = s / pat_h;

    // get angle from current transform, (need get current scale first to calculate angle)
    gdouble oldscale = sp_pattern_extract_scale(pat);
    gdouble theta = sp_pattern_extract_theta(pat,oldscale);

    NR::Matrix rot =  NR::Matrix(NR::rotate(theta)) * NR::Matrix(NR::scale(scl,scl));
    NR::Point const t = sp_pattern_extract_trans(pat);
    rot[4] = t[NR::X];
    rot[5] = t[NR::Y];
    sp_item_adjust_pattern(item, rot, true);
    item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}


NR::Point
PatternKnotHolderEntityScale::knot_get()
{
    SPPattern *pat = SP_PATTERN(SP_STYLE_FILL_SERVER(SP_OBJECT(item)->style));

    gdouble x = pattern_width(pat)*0.5;
    gdouble y = pattern_height(pat)*0.5;
    NR::Point delta = NR::Point(x,y);
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
