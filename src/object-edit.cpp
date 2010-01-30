#define __SP_OBJECT_EDIT_C__

/*
 * Node editing extension to objects
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Mitsuru Oka
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *
 * Licensed under GNU GPL
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif



#include "sp-item.h"
#include "sp-rect.h"
#include "box3d.h"
#include "sp-ellipse.h"
#include "sp-star.h"
#include "sp-spiral.h"
#include "sp-offset.h"
#include "sp-flowtext.h"
#include "preferences.h"
#include "style.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "sp-namedview.h"
#include "live_effects/effect.h"

#include "sp-pattern.h"
#include "sp-path.h"

#include <glibmm/i18n.h>

#include "object-edit.h"

#include <libnr/nr-scale-ops.h>

#include "xml/repr.h"

#include "2geom/isnan.h"

#define sp_round(v,m) (((v) < 0.0) ? ((ceil((v) / (m) - 0.5)) * (m)) : ((floor((v) / (m) + 0.5)) * (m)))

static KnotHolder *sp_lpe_knot_holder(SPItem *item, SPDesktop *desktop)
{
    KnotHolder *knot_holder = new KnotHolder(desktop, item, NULL);

    Inkscape::LivePathEffect::Effect *effect = sp_lpe_item_get_current_lpe(SP_LPE_ITEM(item));
    effect->addHandles(knot_holder, desktop, item);

    return knot_holder;
}

KnotHolder *
sp_item_knot_holder(SPItem *item, SPDesktop *desktop)
{
    KnotHolder *knotholder = NULL;

    if (SP_IS_LPE_ITEM(item) &&
        sp_lpe_item_get_current_lpe(SP_LPE_ITEM(item)) &&
        sp_lpe_item_get_current_lpe(SP_LPE_ITEM(item))->isVisible() &&
        sp_lpe_item_get_current_lpe(SP_LPE_ITEM(item))->providesKnotholder()) {
        knotholder = sp_lpe_knot_holder(item, desktop);
    } else if (SP_IS_RECT(item)) {
        knotholder = new RectKnotHolder(desktop, item, NULL);
    } else if (SP_IS_BOX3D(item)) {
        knotholder = new Box3DKnotHolder(desktop, item, NULL);
    } else if (SP_IS_ARC(item)) {
        knotholder = new ArcKnotHolder(desktop, item, NULL);
    } else if (SP_IS_STAR(item)) {
        knotholder = new StarKnotHolder(desktop, item, NULL);
    } else if (SP_IS_SPIRAL(item)) {
        knotholder = new SpiralKnotHolder(desktop, item, NULL);
    } else if (SP_IS_OFFSET(item)) {
        knotholder = new OffsetKnotHolder(desktop, item, NULL);
    } else if (SP_IS_FLOWTEXT(item) && SP_FLOWTEXT(item)->has_internal_frame()) {
        knotholder = new FlowtextKnotHolder(desktop, SP_FLOWTEXT(item)->get_frame(NULL), NULL);
    } else if ((SP_OBJECT(item)->style->fill.isPaintserver())
               && SP_IS_PATTERN(SP_STYLE_FILL_SERVER(SP_OBJECT(item)->style))) {
        knotholder = new KnotHolder(desktop, item, NULL);
        knotholder->add_pattern_knotholder();
    }

    return knotholder;
}

/* SPRect */

/* handle for horizontal rounding radius */
class RectKnotHolderEntityRX : public KnotHolderEntity {
public:
    virtual Geom::Point knot_get();
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual void knot_click(guint state);
};

/* handle for vertical rounding radius */
class RectKnotHolderEntityRY : public KnotHolderEntity {
public:
    virtual Geom::Point knot_get();
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual void knot_click(guint state);
};

/* handle for width/height adjustment */
class RectKnotHolderEntityWH : public KnotHolderEntity {
public:
    virtual Geom::Point knot_get();
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);

protected:
    void set_internal(Geom::Point const &p, Geom::Point const &origin, guint state);
};

/* handle for x/y adjustment */
class RectKnotHolderEntityXY : public KnotHolderEntity {
public:
    virtual Geom::Point knot_get();
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
};

Geom::Point
RectKnotHolderEntityRX::knot_get()
{
    SPRect *rect = SP_RECT(item);

    return Geom::Point(rect->x.computed + rect->width.computed - rect->rx.computed, rect->y.computed);
}

void
RectKnotHolderEntityRX::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint state)
{
    SPRect *rect = SP_RECT(item);

    //In general we cannot just snap this radius to an arbitrary point, as we have only a single
    //degree of freedom. For snapping to an arbitrary point we need two DOF. If we're going to snap
    //the radius then we should have a constrained snap. snap_knot_position() is unconstrained
    Geom::Point const s = snap_knot_position_constrained(p, Inkscape::Snapper::ConstraintLine(Geom::Point(rect->x.computed + rect->width.computed, rect->y.computed), Geom::Point(-1, 0)));

    if (state & GDK_CONTROL_MASK) {
        gdouble temp = MIN(rect->height.computed, rect->width.computed) / 2.0;
        rect->rx.computed = rect->ry.computed = CLAMP(rect->x.computed + rect->width.computed - s[Geom::X], 0.0, temp);
        rect->rx._set = rect->ry._set = true;

    } else {
        rect->rx.computed = CLAMP(rect->x.computed + rect->width.computed - s[Geom::X], 0.0, rect->width.computed / 2.0);
        rect->rx._set = true;
    }

    update_knot();

    ((SPObject*)rect)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

void
RectKnotHolderEntityRX::knot_click(guint state)
{
    SPRect *rect = SP_RECT(item);

    if (state & GDK_SHIFT_MASK) {
        /* remove rounding from rectangle */
        SP_OBJECT_REPR(rect)->setAttribute("rx", NULL);
        SP_OBJECT_REPR(rect)->setAttribute("ry", NULL);
    } else if (state & GDK_CONTROL_MASK) {
        /* Ctrl-click sets the vertical rounding to be the same as the horizontal */
        SP_OBJECT_REPR(rect)->setAttribute("ry", SP_OBJECT_REPR(rect)->attribute("rx"));
    }

    update_knot();
}

Geom::Point
RectKnotHolderEntityRY::knot_get()
{
    SPRect *rect = SP_RECT(item);

    return Geom::Point(rect->x.computed + rect->width.computed, rect->y.computed + rect->ry.computed);
}

void
RectKnotHolderEntityRY::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint state)
{
    SPRect *rect = SP_RECT(item);

    //In general we cannot just snap this radius to an arbitrary point, as we have only a single
    //degree of freedom. For snapping to an arbitrary point we need two DOF. If we're going to snap
    //the radius then we should have a constrained snap. snap_knot_position() is unconstrained
    Geom::Point const s = snap_knot_position_constrained(p, Inkscape::Snapper::ConstraintLine(Geom::Point(rect->x.computed + rect->width.computed, rect->y.computed), Geom::Point(0, 1)));

    if (state & GDK_CONTROL_MASK) { // When holding control then rx will be kept equal to ry,
                                    // resulting in a perfect circle (and not an ellipse)
        gdouble temp = MIN(rect->height.computed, rect->width.computed) / 2.0;
        rect->rx.computed = rect->ry.computed = CLAMP(s[Geom::Y] - rect->y.computed, 0.0, temp);
        rect->ry._set = rect->rx._set = true;
    } else {
        if (!rect->rx._set || rect->rx.computed == 0) {
            rect->ry.computed = CLAMP(s[Geom::Y] - rect->y.computed,
                                      0.0,
                                      MIN(rect->height.computed / 2.0, rect->width.computed / 2.0));
        } else {
            rect->ry.computed = CLAMP(s[Geom::Y] - rect->y.computed,
                                      0.0,
                                      rect->height.computed / 2.0);
        }

        rect->ry._set = true;
    }

    update_knot();

    ((SPObject *)rect)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

void
RectKnotHolderEntityRY::knot_click(guint state)
{
    SPRect *rect = SP_RECT(item);

    if (state & GDK_SHIFT_MASK) {
        /* remove rounding */
        SP_OBJECT_REPR(rect)->setAttribute("rx", NULL);
        SP_OBJECT_REPR(rect)->setAttribute("ry", NULL);
    } else if (state & GDK_CONTROL_MASK) {
        /* Ctrl-click sets the vertical rounding to be the same as the horizontal */
        SP_OBJECT_REPR(rect)->setAttribute("rx", SP_OBJECT_REPR(rect)->attribute("ry"));
    }
}

#define SGN(x) ((x)>0?1:((x)<0?-1:0))

static void sp_rect_clamp_radii(SPRect *rect)
{
    // clamp rounding radii so that they do not exceed width/height
    if (2 * rect->rx.computed > rect->width.computed) {
        rect->rx.computed = 0.5 * rect->width.computed;
        rect->rx._set = true;
    }
    if (2 * rect->ry.computed > rect->height.computed) {
        rect->ry.computed = 0.5 * rect->height.computed;
        rect->ry._set = true;
    }
}

Geom::Point
RectKnotHolderEntityWH::knot_get()
{
    SPRect *rect = SP_RECT(item);

    return Geom::Point(rect->x.computed + rect->width.computed, rect->y.computed + rect->height.computed);
}

void
RectKnotHolderEntityWH::set_internal(Geom::Point const &p, Geom::Point const &origin, guint state)
{
    SPRect *rect = SP_RECT(item);

    Geom::Point s = p;

    if (state & GDK_CONTROL_MASK) {
        // original width/height when drag started
        gdouble const w_orig = (origin[Geom::X] - rect->x.computed);
        gdouble const h_orig = (origin[Geom::Y] - rect->y.computed);

        //original ratio
        gdouble ratio = (w_orig / h_orig);

        // mouse displacement since drag started
        gdouble minx = p[Geom::X] - origin[Geom::X];
        gdouble miny = p[Geom::Y] - origin[Geom::Y];

        Geom::Point p_handle(rect->x.computed + rect->width.computed, rect->y.computed + rect->height.computed);

        if (fabs(minx) > fabs(miny)) {
            // snap to horizontal or diagonal
            if (minx != 0 && fabs(miny/minx) > 0.5 * 1/ratio && (SGN(minx) == SGN(miny))) {
                // closer to the diagonal and in same-sign quarters, change both using ratio
                s = snap_knot_position_constrained(p, Inkscape::Snapper::ConstraintLine(p_handle, Geom::Point(-ratio, -1)));
                minx = s[Geom::X] - origin[Geom::X];
                miny = s[Geom::Y] - origin[Geom::Y];
                rect->height.computed = MAX(h_orig + minx / ratio, 0);
            } else {
                // closer to the horizontal, change only width, height is h_orig
                s = snap_knot_position_constrained(p, Inkscape::Snapper::ConstraintLine(p_handle, Geom::Point(-1, 0)));
                minx = s[Geom::X] - origin[Geom::X];
                miny = s[Geom::Y] - origin[Geom::Y];
                rect->height.computed = MAX(h_orig, 0);
            }
            rect->width.computed = MAX(w_orig + minx, 0);

        } else {
            // snap to vertical or diagonal
            if (miny != 0 && fabs(minx/miny) > 0.5 * ratio && (SGN(minx) == SGN(miny))) {
                // closer to the diagonal and in same-sign quarters, change both using ratio
                s = snap_knot_position_constrained(p, Inkscape::Snapper::ConstraintLine(p_handle, Geom::Point(-ratio, -1)));
                minx = s[Geom::X] - origin[Geom::X];
                miny = s[Geom::Y] - origin[Geom::Y];
                rect->width.computed = MAX(w_orig + miny * ratio, 0);
            } else {
                // closer to the vertical, change only height, width is w_orig
                s = snap_knot_position_constrained(p, Inkscape::Snapper::ConstraintLine(p_handle, Geom::Point(0, -1)));
                minx = s[Geom::X] - origin[Geom::X];
                miny = s[Geom::Y] - origin[Geom::Y];
                rect->width.computed = MAX(w_orig, 0);
            }
            rect->height.computed = MAX(h_orig + miny, 0);

        }

        rect->width._set = rect->height._set = true;

    } else {
        // move freely
        s = snap_knot_position(p);
        rect->width.computed = MAX(s[Geom::X] - rect->x.computed, 0);
        rect->height.computed = MAX(s[Geom::Y] - rect->y.computed, 0);
        rect->width._set = rect->height._set = true;
    }

    sp_rect_clamp_radii(rect);

    ((SPObject *)rect)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

void
RectKnotHolderEntityWH::knot_set(Geom::Point const &p, Geom::Point const &origin, guint state)
{
    set_internal(p, origin, state);
    update_knot();
}

Geom::Point
RectKnotHolderEntityXY::knot_get()
{
    SPRect *rect = SP_RECT(item);

    return Geom::Point(rect->x.computed, rect->y.computed);
}

void
RectKnotHolderEntityXY::knot_set(Geom::Point const &p, Geom::Point const &origin, guint state)
{
    SPRect *rect = SP_RECT(item);

    // opposite corner (unmoved)
    gdouble opposite_x = (rect->x.computed + rect->width.computed);
    gdouble opposite_y = (rect->y.computed + rect->height.computed);

    // original width/height when drag started
    gdouble w_orig = opposite_x - origin[Geom::X];
    gdouble h_orig = opposite_y - origin[Geom::Y];

    Geom::Point s = p;
    Geom::Point p_handle(rect->x.computed, rect->y.computed);

    // mouse displacement since drag started
    gdouble minx = p[Geom::X] - origin[Geom::X];
    gdouble miny = p[Geom::Y] - origin[Geom::Y];

    if (state & GDK_CONTROL_MASK) {
        //original ratio
        gdouble ratio = (w_orig / h_orig);

        if (fabs(minx) > fabs(miny)) {
            // snap to horizontal or diagonal
            if (minx != 0 && fabs(miny/minx) > 0.5 * 1/ratio && (SGN(minx) == SGN(miny))) {
                // closer to the diagonal and in same-sign quarters, change both using ratio
                s = snap_knot_position_constrained(p, Inkscape::Snapper::ConstraintLine(p_handle, Geom::Point(-ratio, -1)));
                minx = s[Geom::X] - origin[Geom::X];
                miny = s[Geom::Y] - origin[Geom::Y];
                rect->y.computed = MIN(origin[Geom::Y] + minx / ratio, opposite_y);
                rect->height.computed = MAX(h_orig - minx / ratio, 0);
            } else {
                // closer to the horizontal, change only width, height is h_orig
                s = snap_knot_position_constrained(p, Inkscape::Snapper::ConstraintLine(p_handle, Geom::Point(-1, 0)));
                minx = s[Geom::X] - origin[Geom::X];
                miny = s[Geom::Y] - origin[Geom::Y];
                rect->y.computed = MIN(origin[Geom::Y], opposite_y);
                rect->height.computed = MAX(h_orig, 0);
            }
            rect->x.computed = MIN(s[Geom::X], opposite_x);
            rect->width.computed = MAX(w_orig - minx, 0);
        } else {
            // snap to vertical or diagonal
            if (miny != 0 && fabs(minx/miny) > 0.5 *ratio && (SGN(minx) == SGN(miny))) {
                // closer to the diagonal and in same-sign quarters, change both using ratio
                s = snap_knot_position_constrained(p, Inkscape::Snapper::ConstraintLine(p_handle, Geom::Point(-ratio, -1)));
                minx = s[Geom::X] - origin[Geom::X];
                miny = s[Geom::Y] - origin[Geom::Y];
                rect->x.computed = MIN(origin[Geom::X] + miny * ratio, opposite_x);
                rect->width.computed = MAX(w_orig - miny * ratio, 0);
            } else {
                // closer to the vertical, change only height, width is w_orig
                s = snap_knot_position_constrained(p, Inkscape::Snapper::ConstraintLine(p_handle, Geom::Point(0, -1)));
                minx = s[Geom::X] - origin[Geom::X];
                miny = s[Geom::Y] - origin[Geom::Y];
                rect->x.computed = MIN(origin[Geom::X], opposite_x);
                rect->width.computed = MAX(w_orig, 0);
            }
            rect->y.computed = MIN(s[Geom::Y], opposite_y);
            rect->height.computed = MAX(h_orig - miny, 0);
        }

        rect->width._set = rect->height._set = rect->x._set = rect->y._set = true;

    } else {
        // move freely
        s = snap_knot_position(p);
        minx = s[Geom::X] - origin[Geom::X];
        miny = s[Geom::Y] - origin[Geom::Y];

        rect->x.computed = MIN(s[Geom::X], opposite_x);
        rect->width.computed = MAX(w_orig - minx, 0);
        rect->y.computed = MIN(s[Geom::Y], opposite_y);
        rect->height.computed = MAX(h_orig - miny, 0);
        rect->width._set = rect->height._set = rect->x._set = rect->y._set = true;
    }

    sp_rect_clamp_radii(rect);

    update_knot();

    ((SPObject *)rect)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

RectKnotHolder::RectKnotHolder(SPDesktop *desktop, SPItem *item, SPKnotHolderReleasedFunc relhandler) :
    KnotHolder(desktop, item, relhandler)
{
    RectKnotHolderEntityRX *entity_rx = new RectKnotHolderEntityRX();
    RectKnotHolderEntityRY *entity_ry = new RectKnotHolderEntityRY();
    RectKnotHolderEntityWH *entity_wh = new RectKnotHolderEntityWH();
    RectKnotHolderEntityXY *entity_xy = new RectKnotHolderEntityXY();
    entity_rx->create(desktop, item, this,
                      _("Adjust the <b>horizontal rounding</b> radius; with <b>Ctrl</b> "
                        "to make the vertical radius the same"),
                       SP_KNOT_SHAPE_CIRCLE, SP_KNOT_MODE_XOR);
    entity_ry->create(desktop, item, this,
                      _("Adjust the <b>vertical rounding</b> radius; with <b>Ctrl</b> "
                        "to make the horizontal radius the same"),
                      SP_KNOT_SHAPE_CIRCLE, SP_KNOT_MODE_XOR);
    entity_wh->create(desktop, item, this,
                      _("Adjust the <b>width and height</b> of the rectangle; with <b>Ctrl</b> "
                        "to lock ratio or stretch in one dimension only"),
                      SP_KNOT_SHAPE_SQUARE, SP_KNOT_MODE_XOR);
    entity_xy->create(desktop, item, this,
                      _("Adjust the <b>width and height</b> of the rectangle; with <b>Ctrl</b> "
                        "to lock ratio or stretch in one dimension only"),
                      SP_KNOT_SHAPE_SQUARE, SP_KNOT_MODE_XOR);
    entity.push_back(entity_rx);
    entity.push_back(entity_ry);
    entity.push_back(entity_wh);
    entity.push_back(entity_xy);

    add_pattern_knotholder();
}

/* Box3D (= the new 3D box structure) */

class Box3DKnotHolderEntity : public KnotHolderEntity {
public:
    virtual Geom::Point knot_get() = 0;
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state) = 0;

    Geom::Point knot_get_generic(SPItem *item, unsigned int knot_id);
    void knot_set_generic(SPItem *item, unsigned int knot_id, Geom::Point const &p, guint state);
};

Geom::Point
Box3DKnotHolderEntity::knot_get_generic(SPItem *item, unsigned int knot_id)
{
    return box3d_get_corner_screen(SP_BOX3D(item), knot_id);
}

void
Box3DKnotHolderEntity::knot_set_generic(SPItem *item, unsigned int knot_id, Geom::Point const &new_pos, guint state)
{
    Geom::Point const s = snap_knot_position(new_pos);

    g_assert(item != NULL);
    SPBox3D *box = SP_BOX3D(item);
    Geom::Matrix const i2d (sp_item_i2d_affine (item));

    Box3D::Axis movement;
    if ((knot_id < 4) != (state & GDK_SHIFT_MASK)) {
        movement = Box3D::XY;
    } else {
        movement = Box3D::Z;
    }

    box3d_set_corner (box, knot_id, s * i2d, movement, (state & GDK_CONTROL_MASK));
    box3d_set_z_orders(box);
    box3d_position_set(box);
}

class Box3DKnotHolderEntity0 : public Box3DKnotHolderEntity {
public:
    virtual Geom::Point knot_get();
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
};

class Box3DKnotHolderEntity1 : public Box3DKnotHolderEntity {
public:
    virtual Geom::Point knot_get();
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
};

class Box3DKnotHolderEntity2 : public Box3DKnotHolderEntity {
public:
    virtual Geom::Point knot_get();
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
};

class Box3DKnotHolderEntity3 : public Box3DKnotHolderEntity {
public:
    virtual Geom::Point knot_get();
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
};

class Box3DKnotHolderEntity4 : public Box3DKnotHolderEntity {
public:
    virtual Geom::Point knot_get();
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
};

class Box3DKnotHolderEntity5 : public Box3DKnotHolderEntity {
public:
    virtual Geom::Point knot_get();
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
};

class Box3DKnotHolderEntity6 : public Box3DKnotHolderEntity {
public:
    virtual Geom::Point knot_get();
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
};

class Box3DKnotHolderEntity7 : public Box3DKnotHolderEntity {
public:
    virtual Geom::Point knot_get();
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
};

class Box3DKnotHolderEntityCenter : public KnotHolderEntity {
public:
    virtual Geom::Point knot_get();
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
};

Geom::Point
Box3DKnotHolderEntity0::knot_get()
{
    return knot_get_generic(item, 0);
}

Geom::Point
Box3DKnotHolderEntity1::knot_get()
{
    return knot_get_generic(item, 1);
}

Geom::Point
Box3DKnotHolderEntity2::knot_get()
{
    return knot_get_generic(item, 2);
}

Geom::Point
Box3DKnotHolderEntity3::knot_get()
{
    return knot_get_generic(item, 3);
}

Geom::Point
Box3DKnotHolderEntity4::knot_get()
{
    return knot_get_generic(item, 4);
}

Geom::Point
Box3DKnotHolderEntity5::knot_get()
{
    return knot_get_generic(item, 5);
}

Geom::Point
Box3DKnotHolderEntity6::knot_get()
{
    return knot_get_generic(item, 6);
}

Geom::Point
Box3DKnotHolderEntity7::knot_get()
{
    return knot_get_generic(item, 7);
}

Geom::Point
Box3DKnotHolderEntityCenter::knot_get()
{
    return box3d_get_center_screen(SP_BOX3D(item));
}

void
Box3DKnotHolderEntity0::knot_set(Geom::Point const &new_pos, Geom::Point const &/*origin*/, guint state)
{
    knot_set_generic(item, 0, new_pos, state);
}

void
Box3DKnotHolderEntity1::knot_set(Geom::Point const &new_pos, Geom::Point const &/*origin*/, guint state)
{
    knot_set_generic(item, 1, new_pos, state);
}

void
Box3DKnotHolderEntity2::knot_set(Geom::Point const &new_pos, Geom::Point const &/*origin*/, guint state)
{
    knot_set_generic(item, 2, new_pos, state);
}

void
Box3DKnotHolderEntity3::knot_set(Geom::Point const &new_pos, Geom::Point const &/*origin*/, guint state)
{
    knot_set_generic(item, 3, new_pos, state);
}

void
Box3DKnotHolderEntity4::knot_set(Geom::Point const &new_pos, Geom::Point const &/*origin*/, guint state)
{
    knot_set_generic(item, 4, new_pos, state);
}

void
Box3DKnotHolderEntity5::knot_set(Geom::Point const &new_pos, Geom::Point const &/*origin*/, guint state)
{
    knot_set_generic(item, 5, new_pos, state);
}

void
Box3DKnotHolderEntity6::knot_set(Geom::Point const &new_pos, Geom::Point const &/*origin*/, guint state)
{
    knot_set_generic(item, 6, new_pos, state);
}

void
Box3DKnotHolderEntity7::knot_set(Geom::Point const &new_pos, Geom::Point const &/*origin*/, guint state)
{
    knot_set_generic(item, 7, new_pos, state);
}

void
Box3DKnotHolderEntityCenter::knot_set(Geom::Point const &new_pos, Geom::Point const &origin, guint state)
{
    Geom::Point const s = snap_knot_position(new_pos);

    SPBox3D *box = SP_BOX3D(item);
    Geom::Matrix const i2d (sp_item_i2d_affine (item));

    box3d_set_center (SP_BOX3D(item), s * i2d, origin * i2d, !(state & GDK_SHIFT_MASK) ? Box3D::XY : Box3D::Z,
                      state & GDK_CONTROL_MASK);

    box3d_set_z_orders(box);
    box3d_position_set(box);
}

Box3DKnotHolder::Box3DKnotHolder(SPDesktop *desktop, SPItem *item, SPKnotHolderReleasedFunc relhandler) :
    KnotHolder(desktop, item, relhandler)
{
    Box3DKnotHolderEntity0 *entity_corner0 = new Box3DKnotHolderEntity0();
    Box3DKnotHolderEntity1 *entity_corner1 = new Box3DKnotHolderEntity1();
    Box3DKnotHolderEntity2 *entity_corner2 = new Box3DKnotHolderEntity2();
    Box3DKnotHolderEntity3 *entity_corner3 = new Box3DKnotHolderEntity3();
    Box3DKnotHolderEntity4 *entity_corner4 = new Box3DKnotHolderEntity4();
    Box3DKnotHolderEntity5 *entity_corner5 = new Box3DKnotHolderEntity5();
    Box3DKnotHolderEntity6 *entity_corner6 = new Box3DKnotHolderEntity6();
    Box3DKnotHolderEntity7 *entity_corner7 = new Box3DKnotHolderEntity7();
    Box3DKnotHolderEntityCenter *entity_center = new Box3DKnotHolderEntityCenter();

    entity_corner0->create(desktop, item, this,
                           _("Resize box in X/Y direction; with <b>Shift</b> along the Z axis; "
                             "with <b>Ctrl</b> to constrain to the directions of edges or diagonals"));
    entity_corner1->create(desktop, item, this,
                           _("Resize box in X/Y direction; with <b>Shift</b> along the Z axis; "
                             "with <b>Ctrl</b> to constrain to the directions of edges or diagonals"));
    entity_corner2->create(desktop, item, this,
                           _("Resize box in X/Y direction; with <b>Shift</b> along the Z axis; "
                             "with <b>Ctrl</b> to constrain to the directions of edges or diagonals"));
    entity_corner3->create(desktop, item, this,
                           _("Resize box in X/Y direction; with <b>Shift</b> along the Z axis; "
                             "with <b>Ctrl</b> to constrain to the directions of edges or diagonals"));
    entity_corner4->create(desktop, item, this,
                     _("Resize box along the Z axis; with <b>Shift</b> in X/Y direction; "
                       "with <b>Ctrl</b> to constrain to the directions of edges or diagonals"));
    entity_corner5->create(desktop, item, this,
                     _("Resize box along the Z axis; with <b>Shift</b> in X/Y direction; "
                       "with <b>Ctrl</b> to constrain to the directions of edges or diagonals"));
    entity_corner6->create(desktop, item, this,
                     _("Resize box along the Z axis; with <b>Shift</b> in X/Y direction; "
                       "with <b>Ctrl</b> to constrain to the directions of edges or diagonals"));
    entity_corner7->create(desktop, item, this,
                     _("Resize box along the Z axis; with <b>Shift</b> in X/Y direction; "
                       "with <b>Ctrl</b> to constrain to the directions of edges or diagonals"));
    entity_center->create(desktop, item, this,
                          _("Move the box in perspective"),
                          SP_KNOT_SHAPE_CROSS);

    entity.push_back(entity_corner0);
    entity.push_back(entity_corner1);
    entity.push_back(entity_corner2);
    entity.push_back(entity_corner3);
    entity.push_back(entity_corner4);
    entity.push_back(entity_corner5);
    entity.push_back(entity_corner6);
    entity.push_back(entity_corner7);
    entity.push_back(entity_center);

    add_pattern_knotholder();
}

/* SPArc */

class ArcKnotHolderEntityStart : public KnotHolderEntity {
public:
    virtual Geom::Point knot_get();
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
};

class ArcKnotHolderEntityEnd : public KnotHolderEntity {
public:
    virtual Geom::Point knot_get();
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual void knot_click(guint state);
};

class ArcKnotHolderEntityRX : public KnotHolderEntity {
public:
    virtual Geom::Point knot_get();
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual void knot_click(guint state);
};

class ArcKnotHolderEntityRY : public KnotHolderEntity {
public:
    virtual Geom::Point knot_get();
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual void knot_click(guint state);
};

/*
 * return values:
 *   1  : inside
 *   0  : on the curves
 *   -1 : outside
 */
static gint
sp_genericellipse_side(SPGenericEllipse *ellipse, Geom::Point const &p)
{
    gdouble dx = (p[Geom::X] - ellipse->cx.computed) / ellipse->rx.computed;
    gdouble dy = (p[Geom::Y] - ellipse->cy.computed) / ellipse->ry.computed;

    gdouble s = dx * dx + dy * dy;
    if (s < 1.0) return 1;
    if (s > 1.0) return -1;
    return 0;
}

void
ArcKnotHolderEntityStart::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint state)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int snaps = prefs->getInt("/options/rotationsnapsperpi/value", 12);

    SPGenericEllipse *ge = SP_GENERICELLIPSE(item);
    SPArc *arc = SP_ARC(item);

    ge->closed = (sp_genericellipse_side(ge, p) == -1) ? TRUE : FALSE;

    Geom::Point delta = p - Geom::Point(ge->cx.computed, ge->cy.computed);
    Geom::Scale sc(ge->rx.computed, ge->ry.computed);
    ge->start = atan2(delta * sc.inverse());
    if ( ( state & GDK_CONTROL_MASK )
         && snaps )
    {
        ge->start = sp_round(ge->start, M_PI/snaps);
    }
    sp_genericellipse_normalize(ge);
    ((SPObject *)arc)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

Geom::Point
ArcKnotHolderEntityStart::knot_get()
{
    SPGenericEllipse *ge = SP_GENERICELLIPSE(item);
    SPArc *arc = SP_ARC(item);

    return sp_arc_get_xy(arc, ge->start);
}

void
ArcKnotHolderEntityEnd::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint state)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int snaps = prefs->getInt("/options/rotationsnapsperpi/value", 12);

    SPGenericEllipse *ge = SP_GENERICELLIPSE(item);
    SPArc *arc = SP_ARC(item);

    ge->closed = (sp_genericellipse_side(ge, p) == -1) ? TRUE : FALSE;

    Geom::Point delta = p - Geom::Point(ge->cx.computed, ge->cy.computed);
    Geom::Scale sc(ge->rx.computed, ge->ry.computed);
    ge->end = atan2(delta * sc.inverse());
    if ( ( state & GDK_CONTROL_MASK )
         && snaps )
    {
        ge->end = sp_round(ge->end, M_PI/snaps);
    }
    sp_genericellipse_normalize(ge);
    ((SPObject *)arc)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

Geom::Point
ArcKnotHolderEntityEnd::knot_get()
{
    SPGenericEllipse *ge = SP_GENERICELLIPSE(item);
    SPArc *arc = SP_ARC(item);

    return sp_arc_get_xy(arc, ge->end);
}


void
ArcKnotHolderEntityEnd::knot_click(guint state)
{
    SPGenericEllipse *ge = SP_GENERICELLIPSE(item);

    if (state & GDK_SHIFT_MASK) {
        ge->end = ge->start = 0;
        ((SPObject *)ge)->updateRepr();
    }
}


void
ArcKnotHolderEntityRX::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint state)
{
    SPGenericEllipse *ge = SP_GENERICELLIPSE(item);

    Geom::Point const s = snap_knot_position(p);

    ge->rx.computed = fabs( ge->cx.computed - s[Geom::X] );

    if ( state & GDK_CONTROL_MASK ) {
        ge->ry.computed = ge->rx.computed;
    }

    ((SPObject *)item)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

Geom::Point
ArcKnotHolderEntityRX::knot_get()
{
    SPGenericEllipse *ge = SP_GENERICELLIPSE(item);

    return (Geom::Point(ge->cx.computed, ge->cy.computed) -  Geom::Point(ge->rx.computed, 0));
}

void
ArcKnotHolderEntityRX::knot_click(guint state)
{
    SPGenericEllipse *ge = SP_GENERICELLIPSE(item);

    if (state & GDK_CONTROL_MASK) {
        ge->ry.computed = ge->rx.computed;
        ((SPObject *)ge)->updateRepr();
    }
}

void
ArcKnotHolderEntityRY::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint state)
{
    SPGenericEllipse *ge = SP_GENERICELLIPSE(item);

    Geom::Point const s = snap_knot_position(p);

    ge->ry.computed = fabs( ge->cy.computed - s[Geom::Y] );

    if ( state & GDK_CONTROL_MASK ) {
        ge->rx.computed = ge->ry.computed;
    }

    ((SPObject *)item)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

Geom::Point
ArcKnotHolderEntityRY::knot_get()
{
    SPGenericEllipse *ge = SP_GENERICELLIPSE(item);

    return (Geom::Point(ge->cx.computed, ge->cy.computed) -  Geom::Point(0, ge->ry.computed));
}

void
ArcKnotHolderEntityRY::knot_click(guint state)
{
    SPGenericEllipse *ge = SP_GENERICELLIPSE(item);

    if (state & GDK_CONTROL_MASK) {
        ge->rx.computed = ge->ry.computed;
        ((SPObject *)ge)->updateRepr();
    }
}

ArcKnotHolder::ArcKnotHolder(SPDesktop *desktop, SPItem *item, SPKnotHolderReleasedFunc relhandler) :
    KnotHolder(desktop, item, relhandler)
{
    ArcKnotHolderEntityRX *entity_rx = new ArcKnotHolderEntityRX();
    ArcKnotHolderEntityRY *entity_ry = new ArcKnotHolderEntityRY();
    ArcKnotHolderEntityStart *entity_start = new ArcKnotHolderEntityStart();
    ArcKnotHolderEntityEnd *entity_end = new ArcKnotHolderEntityEnd();
    entity_rx->create(desktop, item, this,
                      _("Adjust ellipse <b>width</b>, with <b>Ctrl</b> to make circle"),
                      SP_KNOT_SHAPE_SQUARE, SP_KNOT_MODE_XOR);
    entity_ry->create(desktop, item, this,
                      _("Adjust ellipse <b>height</b>, with <b>Ctrl</b> to make circle"),
                      SP_KNOT_SHAPE_SQUARE, SP_KNOT_MODE_XOR);
    entity_start->create(desktop, item, this,
                         _("Position the <b>start point</b> of the arc or segment; with <b>Ctrl</b> "
                           "to snap angle; drag <b>inside</b> the ellipse for arc, <b>outside</b> for segment"),
                         SP_KNOT_SHAPE_CIRCLE, SP_KNOT_MODE_XOR);
    entity_end->create(desktop, item, this,
                       _("Position the <b>end point</b> of the arc or segment; with <b>Ctrl</b> to snap angle; "
                         "drag <b>inside</b> the ellipse for arc, <b>outside</b> for segment"),
                       SP_KNOT_SHAPE_CIRCLE, SP_KNOT_MODE_XOR);
    entity.push_back(entity_rx);
    entity.push_back(entity_ry);
    entity.push_back(entity_start);
    entity.push_back(entity_end);

    add_pattern_knotholder();
}

/* SPStar */

class StarKnotHolderEntity1 : public KnotHolderEntity {
public:
    virtual Geom::Point knot_get();
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual void knot_click(guint state);
};

class StarKnotHolderEntity2 : public KnotHolderEntity {
public:
    virtual Geom::Point knot_get();
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual void knot_click(guint state);
};

void
StarKnotHolderEntity1::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint state)
{
    SPStar *star = SP_STAR(item);

    Geom::Point const s = snap_knot_position(p);

    Geom::Point d = s - to_2geom(star->center);

    double arg1 = atan2(d);
    double darg1 = arg1 - star->arg[0];

    if (state & GDK_MOD1_MASK) {
        star->randomized = darg1/(star->arg[0] - star->arg[1]);
    } else if (state & GDK_SHIFT_MASK) {
        star->rounded = darg1/(star->arg[0] - star->arg[1]);
    } else if (state & GDK_CONTROL_MASK) {
        star->r[0]    = L2(d);
    } else {
        star->r[0]    = L2(d);
        star->arg[0]  = arg1;
        star->arg[1] += darg1;
    }
    ((SPObject *)star)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

void
StarKnotHolderEntity2::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint state)
{
    SPStar *star = SP_STAR(item);

    Geom::Point const s = snap_knot_position(p);

    if (star->flatsided == false) {
        Geom::Point d = s - to_2geom(star->center);

        double arg1 = atan2(d);
        double darg1 = arg1 - star->arg[1];

        if (state & GDK_MOD1_MASK) {
            star->randomized = darg1/(star->arg[0] - star->arg[1]);
        } else if (state & GDK_SHIFT_MASK) {
            star->rounded = fabs(darg1/(star->arg[0] - star->arg[1]));
        } else if (state & GDK_CONTROL_MASK) {
            star->r[1]   = L2(d);
            star->arg[1] = star->arg[0] + M_PI / star->sides;
        }
        else {
            star->r[1]   = L2(d);
            star->arg[1] = atan2(d);
        }
        ((SPObject *)star)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
    }
}

Geom::Point
StarKnotHolderEntity1::knot_get()
{
    g_assert(item != NULL);

    SPStar *star = SP_STAR(item);

    return sp_star_get_xy(star, SP_STAR_POINT_KNOT1, 0);

}

Geom::Point
StarKnotHolderEntity2::knot_get()
{
    g_assert(item != NULL);

    SPStar *star = SP_STAR(item);

    return sp_star_get_xy(star, SP_STAR_POINT_KNOT2, 0);
}

static void
sp_star_knot_click(SPItem *item, guint state)
{
    SPStar *star = SP_STAR(item);

    if (state & GDK_MOD1_MASK) {
        star->randomized = 0;
        ((SPObject *)star)->updateRepr();
    } else if (state & GDK_SHIFT_MASK) {
        star->rounded = 0;
        ((SPObject *)star)->updateRepr();
    } else if (state & GDK_CONTROL_MASK) {
        star->arg[1] = star->arg[0] + M_PI / star->sides;
        ((SPObject *)star)->updateRepr();
    }
}

void
StarKnotHolderEntity1::knot_click(guint state)
{
    return sp_star_knot_click(item, state);
}

void
StarKnotHolderEntity2::knot_click(guint state)
{
    return sp_star_knot_click(item, state);
}

StarKnotHolder::StarKnotHolder(SPDesktop *desktop, SPItem *item, SPKnotHolderReleasedFunc relhandler) :
    KnotHolder(desktop, item, relhandler)
{
    SPStar *star = SP_STAR(item);

    StarKnotHolderEntity1 *entity1 = new StarKnotHolderEntity1();
    entity1->create(desktop, item, this,
                    _("Adjust the <b>tip radius</b> of the star or polygon; "
                      "with <b>Shift</b> to round; with <b>Alt</b> to randomize"));
    entity.push_back(entity1);

    if (star->flatsided == false) {
        StarKnotHolderEntity2 *entity2 = new StarKnotHolderEntity2();
        entity2->create(desktop, item, this,
                        _("Adjust the <b>base radius</b> of the star; with <b>Ctrl</b> to keep star rays "
                          "radial (no skew); with <b>Shift</b> to round; with <b>Alt</b> to randomize"));
        entity.push_back(entity2);
    }

    add_pattern_knotholder();
}

/* SPSpiral */

class SpiralKnotHolderEntityInner : public KnotHolderEntity {
public:
    virtual Geom::Point knot_get();
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual void knot_click(guint state);
};

class SpiralKnotHolderEntityOuter : public KnotHolderEntity {
public:
    virtual Geom::Point knot_get();
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
};


/*
 * set attributes via inner (t=t0) knot point:
 *   [default] increase/decrease inner point
 *   [shift]   increase/decrease inner and outer arg synchronizely
 *   [control] constrain inner arg to round per PI/4
 */
void
SpiralKnotHolderEntityInner::knot_set(Geom::Point const &p, Geom::Point const &origin, guint state)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int snaps = prefs->getInt("/options/rotationsnapsperpi/value", 12);

    SPSpiral *spiral = SP_SPIRAL(item);

    gdouble   dx = p[Geom::X] - spiral->cx;
    gdouble   dy = p[Geom::Y] - spiral->cy;

    gdouble   moved_y = p[Geom::Y] - origin[Geom::Y];

    if (state & GDK_MOD1_MASK) {
        // adjust divergence by vertical drag, relative to rad
        if (spiral->rad > 0) {
            double exp_delta = 0.1*moved_y/(spiral->rad); // arbitrary multiplier to slow it down
            spiral->exp += exp_delta;
            if (spiral->exp < 1e-3)
                spiral->exp = 1e-3;
        }
    } else {
        // roll/unroll from inside
        gdouble   arg_t0;
        sp_spiral_get_polar(spiral, spiral->t0, NULL, &arg_t0);

        gdouble   arg_tmp = atan2(dy, dx) - arg_t0;
        gdouble   arg_t0_new = arg_tmp - floor((arg_tmp+M_PI)/(2.0*M_PI))*2.0*M_PI + arg_t0;
        spiral->t0 = (arg_t0_new - spiral->arg) / (2.0*M_PI*spiral->revo);

        /* round inner arg per PI/snaps, if CTRL is pressed */
        if ( ( state & GDK_CONTROL_MASK )
             && ( fabs(spiral->revo) > SP_EPSILON_2 )
             && ( snaps != 0 ) ) {
            gdouble arg = 2.0*M_PI*spiral->revo*spiral->t0 + spiral->arg;
            spiral->t0 = (sp_round(arg, M_PI/snaps) - spiral->arg)/(2.0*M_PI*spiral->revo);
        }

        spiral->t0 = CLAMP(spiral->t0, 0.0, 0.999);
    }

    ((SPObject *)spiral)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

/*
 * set attributes via outer (t=1) knot point:
 *   [default] increase/decrease revolution factor
 *   [control] constrain inner arg to round per PI/4
 */
void
SpiralKnotHolderEntityOuter::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint state)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int snaps = prefs->getInt("/options/rotationsnapsperpi/value", 12);

    SPSpiral *spiral = SP_SPIRAL(item);

    gdouble  dx = p[Geom::X] - spiral->cx;
    gdouble  dy = p[Geom::Y] - spiral->cy;

    if (state & GDK_SHIFT_MASK) { // rotate without roll/unroll
        spiral->arg = atan2(dy, dx) - 2.0*M_PI*spiral->revo;
        if (!(state & GDK_MOD1_MASK)) {
            // if alt not pressed, change also rad; otherwise it is locked
            spiral->rad = MAX(hypot(dx, dy), 0.001);
        }
        if ( ( state & GDK_CONTROL_MASK )
             && snaps ) {
            spiral->arg = sp_round(spiral->arg, M_PI/snaps);
        }
    } else { // roll/unroll
        // arg of the spiral outer end
        double arg_1;
        sp_spiral_get_polar(spiral, 1, NULL, &arg_1);

        // its fractional part after the whole turns are subtracted
        double arg_r = arg_1 - sp_round(arg_1, 2.0*M_PI);

        // arg of the mouse point relative to spiral center
        double mouse_angle = atan2(dy, dx);
        if (mouse_angle < 0)
            mouse_angle += 2*M_PI;

        // snap if ctrl
        if ( ( state & GDK_CONTROL_MASK ) && snaps ) {
            mouse_angle = sp_round(mouse_angle, M_PI/snaps);
        }

        // by how much we want to rotate the outer point
        double diff = mouse_angle - arg_r;
        if (diff > M_PI)
            diff -= 2*M_PI;
        else if (diff < -M_PI)
            diff += 2*M_PI;

        // calculate the new rad;
        // the value of t corresponding to the angle arg_1 + diff:
        double t_temp = ((arg_1 + diff) - spiral->arg)/(2*M_PI*spiral->revo);
        // the rad at that t:
        double rad_new = 0;
        if (t_temp > spiral->t0)
            sp_spiral_get_polar(spiral, t_temp, &rad_new, NULL);

        // change the revo (converting diff from radians to the number of turns)
        spiral->revo += diff/(2*M_PI);
        if (spiral->revo < 1e-3)
            spiral->revo = 1e-3;

        // if alt not pressed and the values are sane, change the rad
        if (!(state & GDK_MOD1_MASK) && rad_new > 1e-3 && rad_new/spiral->rad < 2) {
            // adjust t0 too so that the inner point stays unmoved
            double r0;
            sp_spiral_get_polar(spiral, spiral->t0, &r0, NULL);
            spiral->rad = rad_new;
            spiral->t0 = pow(r0 / spiral->rad, 1.0/spiral->exp);
        }
        if (!IS_FINITE(spiral->t0)) spiral->t0 = 0.0;
        spiral->t0 = CLAMP(spiral->t0, 0.0, 0.999);
    }

    ((SPObject *)spiral)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

Geom::Point
SpiralKnotHolderEntityInner::knot_get()
{
    SPSpiral *spiral = SP_SPIRAL(item);

    return sp_spiral_get_xy(spiral, spiral->t0);
}

Geom::Point
SpiralKnotHolderEntityOuter::knot_get()
{
    SPSpiral *spiral = SP_SPIRAL(item);

    return sp_spiral_get_xy(spiral, 1.0);
}

void
SpiralKnotHolderEntityInner::knot_click(guint state)
{
    SPSpiral *spiral = SP_SPIRAL(item);

    if (state & GDK_MOD1_MASK) {
        spiral->exp = 1;
        ((SPObject *)spiral)->updateRepr();
    } else if (state & GDK_SHIFT_MASK) {
        spiral->t0 = 0;
        ((SPObject *)spiral)->updateRepr();
    }
}

SpiralKnotHolder::SpiralKnotHolder(SPDesktop *desktop, SPItem *item, SPKnotHolderReleasedFunc relhandler) :
    KnotHolder(desktop, item, relhandler)
{
    SpiralKnotHolderEntityInner *entity_inner = new SpiralKnotHolderEntityInner();
    SpiralKnotHolderEntityOuter *entity_outer = new SpiralKnotHolderEntityOuter();
    entity_inner->create(desktop, item, this,
                         _("Roll/unroll the spiral from <b>inside</b>; with <b>Ctrl</b> to snap angle; "
                           "with <b>Alt</b> to converge/diverge"));
    entity_outer->create(desktop, item, this,
                         _("Roll/unroll the spiral from <b>outside</b>; with <b>Ctrl</b> to snap angle; "
                           "with <b>Shift</b> to scale/rotate"));
    entity.push_back(entity_inner);
    entity.push_back(entity_outer);

    add_pattern_knotholder();
}

/* SPOffset */

class OffsetKnotHolderEntity : public KnotHolderEntity {
public:
    virtual Geom::Point knot_get();
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
};

void
OffsetKnotHolderEntity::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint /*state*/)
{
    SPOffset *offset = SP_OFFSET(item);

    offset->rad = sp_offset_distance_to_original(offset, p);
    offset->knot = p;
    offset->knotSet = true;

    ((SPObject *)offset)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}


Geom::Point
OffsetKnotHolderEntity::knot_get()
{
    SPOffset *offset = SP_OFFSET(item);

    Geom::Point np;
    sp_offset_top_point(offset,&np);
    return np;
}

OffsetKnotHolder::OffsetKnotHolder(SPDesktop *desktop, SPItem *item, SPKnotHolderReleasedFunc relhandler) :
    KnotHolder(desktop, item, relhandler)
{
    OffsetKnotHolderEntity *entity_offset = new OffsetKnotHolderEntity();
    entity_offset->create(desktop, item, this,
                          _("Adjust the <b>offset distance</b>"));
    entity.push_back(entity_offset);

    add_pattern_knotholder();
}

// TODO: this is derived from RectKnotHolderEntityWH because it used the same static function
// set_internal as the latter before KnotHolderEntity was C++ified. Check whether this also makes
// sense logically.
class FlowtextKnotHolderEntity : public RectKnotHolderEntityWH {
public:
    virtual Geom::Point knot_get();
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
};

Geom::Point
FlowtextKnotHolderEntity::knot_get()
{
    SPRect *rect = SP_RECT(item);

    return Geom::Point(rect->x.computed + rect->width.computed, rect->y.computed + rect->height.computed);
}

void
FlowtextKnotHolderEntity::knot_set(Geom::Point const &p, Geom::Point const &origin, guint state)
{
    set_internal(p, origin, state);
}

FlowtextKnotHolder::FlowtextKnotHolder(SPDesktop *desktop, SPItem *item, SPKnotHolderReleasedFunc relhandler) :
    KnotHolder(desktop, item, relhandler)
{
    g_assert(item != NULL);

    FlowtextKnotHolderEntity *entity_flowtext = new FlowtextKnotHolderEntity();
    entity_flowtext->create(desktop, item, this,
                            _("Drag to resize the <b>flowed text frame</b>"));
    entity.push_back(entity_flowtext);
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
