/*
 * Node editing extension to objects
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Mitsuru Oka
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *   Abhishek Sharma
 *   Jon A. Cruz <jon@joncruz.org>
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
#include "xml/repr.h"
#include <2geom/math-utils.h>
#include "knot-holder-entity.h"

#define sp_round(v,m) (((v) < 0.0) ? ((ceil((v) / (m) - 0.5)) * (m)) : ((floor((v) / (m) + 0.5)) * (m)))

namespace {

static KnotHolder *sp_lpe_knot_holder(SPLPEItem *item, SPDesktop *desktop)
{
    KnotHolder *knot_holder = new KnotHolder(desktop, item, NULL);

    Inkscape::LivePathEffect::Effect *effect = item->getCurrentLPE();
    effect->addHandles(knot_holder, desktop, item);

    return knot_holder;
}

} // namespace

namespace Inkscape {

KnotHolder *createKnotHolder(SPItem *item, SPDesktop *desktop)
{
    KnotHolder *knotholder = NULL;

    SPLPEItem *lpe = dynamic_cast<SPLPEItem *>(item);
    if (lpe &&
        lpe->getCurrentLPE() &&
        lpe->getCurrentLPE()->isVisible() &&
        lpe->getCurrentLPE()->providesKnotholder()) {
        knotholder = sp_lpe_knot_holder(lpe, desktop);
    } else if (dynamic_cast<SPRect *>(item)) {
        knotholder = new RectKnotHolder(desktop, item, NULL);
    } else if (dynamic_cast<SPBox3D *>(item)) {
        knotholder = new Box3DKnotHolder(desktop, item, NULL);
    } else if (dynamic_cast<SPGenericEllipse *>(item)) {
        knotholder = new ArcKnotHolder(desktop, item, NULL);
    } else if (dynamic_cast<SPStar *>(item)) {
        knotholder = new StarKnotHolder(desktop, item, NULL);
    } else if (dynamic_cast<SPSpiral *>(item)) {
        knotholder = new SpiralKnotHolder(desktop, item, NULL);
    } else if (dynamic_cast<SPOffset *>(item)) {
        knotholder = new OffsetKnotHolder(desktop, item, NULL);
    } else {
        SPFlowtext *flowtext = dynamic_cast<SPFlowtext *>(item);
        if (flowtext && flowtext->has_internal_frame()) {
            knotholder = new FlowtextKnotHolder(desktop, flowtext->get_frame(NULL), NULL);
        } else if ((item->style->fill.isPaintserver() && dynamic_cast<SPPattern *>(item->style->getFillPaintServer())) ||
                   (item->style->stroke.isPaintserver() && dynamic_cast<SPPattern *>(item->style->getStrokePaintServer()))) {
            knotholder = new KnotHolder(desktop, item, NULL);
            knotholder->add_pattern_knotholder();
        }
    }

    return knotholder;
}

} // Inkscape

/* SPRect */

/* handle for horizontal rounding radius */
class RectKnotHolderEntityRX : public KnotHolderEntity {
public:
    virtual Geom::Point knot_get() const;
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual void knot_click(guint state);
};

/* handle for vertical rounding radius */
class RectKnotHolderEntityRY : public KnotHolderEntity {
public:
    virtual Geom::Point knot_get() const;
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual void knot_click(guint state);
};

/* handle for width/height adjustment */
class RectKnotHolderEntityWH : public KnotHolderEntity {
public:
    virtual Geom::Point knot_get() const;
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);

protected:
    void set_internal(Geom::Point const &p, Geom::Point const &origin, guint state);
};

/* handle for x/y adjustment */
class RectKnotHolderEntityXY : public KnotHolderEntity {
public:
    virtual Geom::Point knot_get() const;
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
};

Geom::Point
RectKnotHolderEntityRX::knot_get() const
{
    SPRect *rect = dynamic_cast<SPRect *>(item);
    g_assert(rect != NULL);

    return Geom::Point(rect->x.computed + rect->width.computed - rect->rx.computed, rect->y.computed);
}

void
RectKnotHolderEntityRX::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint state)
{
    SPRect *rect = dynamic_cast<SPRect *>(item);
    g_assert(rect != NULL);

    //In general we cannot just snap this radius to an arbitrary point, as we have only a single
    //degree of freedom. For snapping to an arbitrary point we need two DOF. If we're going to snap
    //the radius then we should have a constrained snap. snap_knot_position() is unconstrained
    Geom::Point const s = snap_knot_position_constrained(p, Inkscape::Snapper::SnapConstraint(Geom::Point(rect->x.computed + rect->width.computed, rect->y.computed), Geom::Point(-1, 0)), state);

    if (state & GDK_CONTROL_MASK) {
        gdouble temp = MIN(rect->height.computed, rect->width.computed) / 2.0;
        rect->rx.computed = rect->ry.computed = CLAMP(rect->x.computed + rect->width.computed - s[Geom::X], 0.0, temp);
        rect->rx._set = rect->ry._set = true;

    } else {
        rect->rx.computed = CLAMP(rect->x.computed + rect->width.computed - s[Geom::X], 0.0, rect->width.computed / 2.0);
        rect->rx._set = true;
    }

    update_knot();

    rect->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

void
RectKnotHolderEntityRX::knot_click(guint state)
{
    SPRect *rect = dynamic_cast<SPRect *>(item);
    g_assert(rect != NULL);

    if (state & GDK_SHIFT_MASK) {
        /* remove rounding from rectangle */
        rect->getRepr()->setAttribute("rx", NULL);
        rect->getRepr()->setAttribute("ry", NULL);
    } else if (state & GDK_CONTROL_MASK) {
        /* Ctrl-click sets the vertical rounding to be the same as the horizontal */
        rect->getRepr()->setAttribute("ry", rect->getRepr()->attribute("rx"));
    }

}

Geom::Point
RectKnotHolderEntityRY::knot_get() const
{
    SPRect *rect = dynamic_cast<SPRect *>(item);
    g_assert(rect != NULL);

    return Geom::Point(rect->x.computed + rect->width.computed, rect->y.computed + rect->ry.computed);
}

void
RectKnotHolderEntityRY::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint state)
{
    SPRect *rect = dynamic_cast<SPRect *>(item);
    g_assert(rect != NULL);

    //In general we cannot just snap this radius to an arbitrary point, as we have only a single
    //degree of freedom. For snapping to an arbitrary point we need two DOF. If we're going to snap
    //the radius then we should have a constrained snap. snap_knot_position() is unconstrained
    Geom::Point const s = snap_knot_position_constrained(p, Inkscape::Snapper::SnapConstraint(Geom::Point(rect->x.computed + rect->width.computed, rect->y.computed), Geom::Point(0, 1)), state);

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

    rect->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

void
RectKnotHolderEntityRY::knot_click(guint state)
{
    SPRect *rect = dynamic_cast<SPRect *>(item);
    g_assert(rect != NULL);

    if (state & GDK_SHIFT_MASK) {
        /* remove rounding */
        rect->getRepr()->setAttribute("rx", NULL);
        rect->getRepr()->setAttribute("ry", NULL);
    } else if (state & GDK_CONTROL_MASK) {
        /* Ctrl-click sets the vertical rounding to be the same as the horizontal */
        rect->getRepr()->setAttribute("rx", rect->getRepr()->attribute("ry"));
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
RectKnotHolderEntityWH::knot_get() const
{
    SPRect *rect = dynamic_cast<SPRect *>(item);
    g_assert(rect != NULL);

    return Geom::Point(rect->x.computed + rect->width.computed, rect->y.computed + rect->height.computed);
}

void
RectKnotHolderEntityWH::set_internal(Geom::Point const &p, Geom::Point const &origin, guint state)
{
    SPRect *rect = dynamic_cast<SPRect *>(item);
    g_assert(rect != NULL);

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
                s = snap_knot_position_constrained(p, Inkscape::Snapper::SnapConstraint(p_handle, Geom::Point(-ratio, -1)), state);
                minx = s[Geom::X] - origin[Geom::X];
                miny = s[Geom::Y] - origin[Geom::Y];
                rect->height.computed = MAX(h_orig + minx / ratio, 0);
            } else {
                // closer to the horizontal, change only width, height is h_orig
                s = snap_knot_position_constrained(p, Inkscape::Snapper::SnapConstraint(p_handle, Geom::Point(-1, 0)), state);
                minx = s[Geom::X] - origin[Geom::X];
                miny = s[Geom::Y] - origin[Geom::Y];
                rect->height.computed = MAX(h_orig, 0);
            }
            rect->width.computed = MAX(w_orig + minx, 0);

        } else {
            // snap to vertical or diagonal
            if (miny != 0 && fabs(minx/miny) > 0.5 * ratio && (SGN(minx) == SGN(miny))) {
                // closer to the diagonal and in same-sign quarters, change both using ratio
                s = snap_knot_position_constrained(p, Inkscape::Snapper::SnapConstraint(p_handle, Geom::Point(-ratio, -1)), state);
                minx = s[Geom::X] - origin[Geom::X];
                miny = s[Geom::Y] - origin[Geom::Y];
                rect->width.computed = MAX(w_orig + miny * ratio, 0);
            } else {
                // closer to the vertical, change only height, width is w_orig
                s = snap_knot_position_constrained(p, Inkscape::Snapper::SnapConstraint(p_handle, Geom::Point(0, -1)), state);
                minx = s[Geom::X] - origin[Geom::X];
                miny = s[Geom::Y] - origin[Geom::Y];
                rect->width.computed = MAX(w_orig, 0);
            }
            rect->height.computed = MAX(h_orig + miny, 0);

        }

        rect->width._set = rect->height._set = true;

    } else {
        // move freely
        s = snap_knot_position(p, state);
        rect->width.computed = MAX(s[Geom::X] - rect->x.computed, 0);
        rect->height.computed = MAX(s[Geom::Y] - rect->y.computed, 0);
        rect->width._set = rect->height._set = true;
    }

    sp_rect_clamp_radii(rect);

    rect->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

void
RectKnotHolderEntityWH::knot_set(Geom::Point const &p, Geom::Point const &origin, guint state)
{
    set_internal(p, origin, state);
    update_knot();
}

Geom::Point
RectKnotHolderEntityXY::knot_get() const
{
    SPRect *rect = dynamic_cast<SPRect *>(item);
    g_assert(rect != NULL);

    return Geom::Point(rect->x.computed, rect->y.computed);
}

void
RectKnotHolderEntityXY::knot_set(Geom::Point const &p, Geom::Point const &origin, guint state)
{
    SPRect *rect = dynamic_cast<SPRect *>(item);
    g_assert(rect != NULL);

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
                s = snap_knot_position_constrained(p, Inkscape::Snapper::SnapConstraint(p_handle, Geom::Point(-ratio, -1)), state);
                minx = s[Geom::X] - origin[Geom::X];
                miny = s[Geom::Y] - origin[Geom::Y];
                rect->y.computed = MIN(origin[Geom::Y] + minx / ratio, opposite_y);
                rect->height.computed = MAX(h_orig - minx / ratio, 0);
            } else {
                // closer to the horizontal, change only width, height is h_orig
                s = snap_knot_position_constrained(p, Inkscape::Snapper::SnapConstraint(p_handle, Geom::Point(-1, 0)), state);
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
                s = snap_knot_position_constrained(p, Inkscape::Snapper::SnapConstraint(p_handle, Geom::Point(-ratio, -1)), state);
                minx = s[Geom::X] - origin[Geom::X];
                miny = s[Geom::Y] - origin[Geom::Y];
                rect->x.computed = MIN(origin[Geom::X] + miny * ratio, opposite_x);
                rect->width.computed = MAX(w_orig - miny * ratio, 0);
            } else {
                // closer to the vertical, change only height, width is w_orig
                s = snap_knot_position_constrained(p, Inkscape::Snapper::SnapConstraint(p_handle, Geom::Point(0, -1)), state);
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
        s = snap_knot_position(p, state);
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

    rect->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

RectKnotHolder::RectKnotHolder(SPDesktop *desktop, SPItem *item, SPKnotHolderReleasedFunc relhandler) :
    KnotHolder(desktop, item, relhandler)
{
    RectKnotHolderEntityRX *entity_rx = new RectKnotHolderEntityRX();
    RectKnotHolderEntityRY *entity_ry = new RectKnotHolderEntityRY();
    RectKnotHolderEntityWH *entity_wh = new RectKnotHolderEntityWH();
    RectKnotHolderEntityXY *entity_xy = new RectKnotHolderEntityXY();

    entity_rx->create(desktop, item, this, Inkscape::CTRL_TYPE_ROTATE,
                      _("Adjust the <b>horizontal rounding</b> radius; with <b>Ctrl</b> "
                        "to make the vertical radius the same"),
                      SP_KNOT_SHAPE_CIRCLE, SP_KNOT_MODE_XOR);

    entity_ry->create(desktop, item, this, Inkscape::CTRL_TYPE_ROTATE,
                      _("Adjust the <b>vertical rounding</b> radius; with <b>Ctrl</b> "
                        "to make the horizontal radius the same"),
                      SP_KNOT_SHAPE_CIRCLE, SP_KNOT_MODE_XOR);

    entity_wh->create(desktop, item, this, Inkscape::CTRL_TYPE_SIZER,
                      _("Adjust the <b>width and height</b> of the rectangle; with <b>Ctrl</b> "
                        "to lock ratio or stretch in one dimension only"),
                      SP_KNOT_SHAPE_SQUARE, SP_KNOT_MODE_XOR);

    entity_xy->create(desktop, item, this, Inkscape::CTRL_TYPE_SIZER,
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
    virtual Geom::Point knot_get() const = 0;
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state) = 0;

    Geom::Point knot_get_generic(SPItem *item, unsigned int knot_id) const;
    void knot_set_generic(SPItem *item, unsigned int knot_id, Geom::Point const &p, guint state);
};

Geom::Point
Box3DKnotHolderEntity::knot_get_generic(SPItem *item, unsigned int knot_id) const
{
    SPBox3D *box = dynamic_cast<SPBox3D *>(item);
    if (box) {
        return box3d_get_corner_screen(box, knot_id);
    } else {
        return Geom::Point(); // TODO investigate proper fallback
    }
}

void
Box3DKnotHolderEntity::knot_set_generic(SPItem *item, unsigned int knot_id, Geom::Point const &new_pos, guint state)
{
    Geom::Point const s = snap_knot_position(new_pos, state);

    g_assert(item != NULL);
    SPBox3D *box = dynamic_cast<SPBox3D *>(item);
    g_assert(box != NULL);
    Geom::Affine const i2dt (item->i2dt_affine ());

    Box3D::Axis movement;
    if ((knot_id < 4) != (state & GDK_SHIFT_MASK)) {
        movement = Box3D::XY;
    } else {
        movement = Box3D::Z;
    }

    box3d_set_corner (box, knot_id, s * i2dt, movement, (state & GDK_CONTROL_MASK));
    box3d_set_z_orders(box);
    box3d_position_set(box);
}

class Box3DKnotHolderEntity0 : public Box3DKnotHolderEntity {
public:
    virtual Geom::Point knot_get() const;
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
};

class Box3DKnotHolderEntity1 : public Box3DKnotHolderEntity {
public:
    virtual Geom::Point knot_get() const;
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
};

class Box3DKnotHolderEntity2 : public Box3DKnotHolderEntity {
public:
    virtual Geom::Point knot_get() const;
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
};

class Box3DKnotHolderEntity3 : public Box3DKnotHolderEntity {
public:
    virtual Geom::Point knot_get() const;
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
};

class Box3DKnotHolderEntity4 : public Box3DKnotHolderEntity {
public:
    virtual Geom::Point knot_get() const;
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
};

class Box3DKnotHolderEntity5 : public Box3DKnotHolderEntity {
public:
    virtual Geom::Point knot_get() const;
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
};

class Box3DKnotHolderEntity6 : public Box3DKnotHolderEntity {
public:
    virtual Geom::Point knot_get() const;
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
};

class Box3DKnotHolderEntity7 : public Box3DKnotHolderEntity {
public:
    virtual Geom::Point knot_get() const;
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
};

class Box3DKnotHolderEntityCenter : public KnotHolderEntity {
public:
    virtual Geom::Point knot_get() const;
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
};

Geom::Point
Box3DKnotHolderEntity0::knot_get() const
{
    return knot_get_generic(item, 0);
}

Geom::Point
Box3DKnotHolderEntity1::knot_get() const
{
    return knot_get_generic(item, 1);
}

Geom::Point
Box3DKnotHolderEntity2::knot_get() const
{
    return knot_get_generic(item, 2);
}

Geom::Point
Box3DKnotHolderEntity3::knot_get() const
{
    return knot_get_generic(item, 3);
}

Geom::Point
Box3DKnotHolderEntity4::knot_get() const
{
    return knot_get_generic(item, 4);
}

Geom::Point
Box3DKnotHolderEntity5::knot_get() const
{
    return knot_get_generic(item, 5);
}

Geom::Point
Box3DKnotHolderEntity6::knot_get() const
{
    return knot_get_generic(item, 6);
}

Geom::Point
Box3DKnotHolderEntity7::knot_get() const
{
    return knot_get_generic(item, 7);
}

Geom::Point
Box3DKnotHolderEntityCenter::knot_get() const
{
    SPBox3D *box = dynamic_cast<SPBox3D *>(item);
    if (box) {
        return box3d_get_center_screen(box);
    } else {
        return Geom::Point(); // TODO investigate proper fallback
    }
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
    Geom::Point const s = snap_knot_position(new_pos, state);

    SPBox3D *box = dynamic_cast<SPBox3D *>(item);
    g_assert(box != NULL);
    Geom::Affine const i2dt (item->i2dt_affine ());

    box3d_set_center(box, s * i2dt, origin * i2dt, !(state & GDK_SHIFT_MASK) ? Box3D::XY : Box3D::Z,
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

    entity_corner0->create(desktop, item, this, Inkscape::CTRL_TYPE_SHAPER,
                           _("Resize box in X/Y direction; with <b>Shift</b> along the Z axis; "
                             "with <b>Ctrl</b> to constrain to the directions of edges or diagonals"));

    entity_corner1->create(desktop, item, this, Inkscape::CTRL_TYPE_SHAPER,
                           _("Resize box in X/Y direction; with <b>Shift</b> along the Z axis; "
                             "with <b>Ctrl</b> to constrain to the directions of edges or diagonals"));

    entity_corner2->create(desktop, item, this, Inkscape::CTRL_TYPE_SHAPER,
                           _("Resize box in X/Y direction; with <b>Shift</b> along the Z axis; "
                             "with <b>Ctrl</b> to constrain to the directions of edges or diagonals"));

    entity_corner3->create(desktop, item, this, Inkscape::CTRL_TYPE_SHAPER,
                           _("Resize box in X/Y direction; with <b>Shift</b> along the Z axis; "
                             "with <b>Ctrl</b> to constrain to the directions of edges or diagonals"));

    entity_corner4->create(desktop, item, this, Inkscape::CTRL_TYPE_SHAPER,
                     _("Resize box along the Z axis; with <b>Shift</b> in X/Y direction; "
                       "with <b>Ctrl</b> to constrain to the directions of edges or diagonals"));

    entity_corner5->create(desktop, item, this, Inkscape::CTRL_TYPE_SHAPER,
                     _("Resize box along the Z axis; with <b>Shift</b> in X/Y direction; "
                       "with <b>Ctrl</b> to constrain to the directions of edges or diagonals"));

    entity_corner6->create(desktop, item, this, Inkscape::CTRL_TYPE_SHAPER,
                     _("Resize box along the Z axis; with <b>Shift</b> in X/Y direction; "
                       "with <b>Ctrl</b> to constrain to the directions of edges or diagonals"));

    entity_corner7->create(desktop, item, this, Inkscape::CTRL_TYPE_SHAPER,
                     _("Resize box along the Z axis; with <b>Shift</b> in X/Y direction; "
                       "with <b>Ctrl</b> to constrain to the directions of edges or diagonals"));

    entity_center->create(desktop, item, this, Inkscape::CTRL_TYPE_POINT,
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
    virtual Geom::Point knot_get() const;
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual void knot_click(guint state);
};

class ArcKnotHolderEntityEnd : public KnotHolderEntity {
public:
    virtual Geom::Point knot_get() const;
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual void knot_click(guint state);
};

class ArcKnotHolderEntityRX : public KnotHolderEntity {
public:
    virtual Geom::Point knot_get() const;
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual void knot_click(guint state);
};

class ArcKnotHolderEntityRY : public KnotHolderEntity {
public:
    virtual Geom::Point knot_get() const;
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
    int snaps = Inkscape::Preferences::get()->getInt("/options/rotationsnapsperpi/value", 12);

    SPGenericEllipse *arc = dynamic_cast<SPGenericEllipse *>(item);
    g_assert(arc != NULL);

    arc->setClosed(sp_genericellipse_side(arc, p) == -1);

    Geom::Point delta = p - Geom::Point(arc->cx.computed, arc->cy.computed);
    Geom::Scale sc(arc->rx.computed, arc->ry.computed);

    arc->start = atan2(delta * sc.inverse());

    if ((state & GDK_CONTROL_MASK) && snaps) {
        arc->start = sp_round(arc->start, M_PI / snaps);
    }

    arc->normalize();
    arc->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

Geom::Point
ArcKnotHolderEntityStart::knot_get() const
{
    SPGenericEllipse const *ge = dynamic_cast<SPGenericEllipse const *>(item);
    g_assert(ge != NULL);

    return ge->getPointAtAngle(ge->start);
}

void
ArcKnotHolderEntityStart::knot_click(guint state)
{
    SPGenericEllipse *ge = dynamic_cast<SPGenericEllipse *>(item);
    g_assert(ge != NULL);

    if (state & GDK_SHIFT_MASK) {
        ge->end = ge->start = 0;
        ge->updateRepr();
    }
}

void
ArcKnotHolderEntityEnd::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint state)
{
    int snaps = Inkscape::Preferences::get()->getInt("/options/rotationsnapsperpi/value", 12);

    SPGenericEllipse *arc = dynamic_cast<SPGenericEllipse *>(item);
    g_assert(arc != NULL);

    arc->setClosed(sp_genericellipse_side(arc, p) == -1);

    Geom::Point delta = p - Geom::Point(arc->cx.computed, arc->cy.computed);
    Geom::Scale sc(arc->rx.computed, arc->ry.computed);

    arc->end = atan2(delta * sc.inverse());

    if ((state & GDK_CONTROL_MASK) && snaps) {
        arc->end = sp_round(arc->end, M_PI/snaps);
    }

    arc->normalize();
    arc->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

Geom::Point
ArcKnotHolderEntityEnd::knot_get() const
{
    SPGenericEllipse const *ge = dynamic_cast<SPGenericEllipse const *>(item);
    g_assert(ge != NULL);

    return ge->getPointAtAngle(ge->end);
}


void
ArcKnotHolderEntityEnd::knot_click(guint state)
{
    SPGenericEllipse *ge = dynamic_cast<SPGenericEllipse *>(item);
    g_assert(ge != NULL);

    if (state & GDK_SHIFT_MASK) {
        ge->end = ge->start = 0;
        ge->updateRepr();
    }
}


void
ArcKnotHolderEntityRX::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint state)
{
    SPGenericEllipse *ge = dynamic_cast<SPGenericEllipse *>(item);
    g_assert(ge != NULL);

    Geom::Point const s = snap_knot_position(p, state);

    ge->rx.computed = fabs( ge->cx.computed - s[Geom::X] );

    if ( state & GDK_CONTROL_MASK ) {
        ge->ry.computed = ge->rx.computed;
    }

    item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

Geom::Point
ArcKnotHolderEntityRX::knot_get() const
{
    SPGenericEllipse const *ge = dynamic_cast<SPGenericEllipse const *>(item);
    g_assert(ge != NULL);

    return (Geom::Point(ge->cx.computed, ge->cy.computed) -  Geom::Point(ge->rx.computed, 0));
}

void
ArcKnotHolderEntityRX::knot_click(guint state)
{
    SPGenericEllipse *ge = dynamic_cast<SPGenericEllipse *>(item);
    g_assert(ge != NULL);

    if (state & GDK_CONTROL_MASK) {
        ge->ry.computed = ge->rx.computed;
        ge->updateRepr();
    }
}

void
ArcKnotHolderEntityRY::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint state)
{
    SPGenericEllipse *ge = dynamic_cast<SPGenericEllipse *>(item);
    g_assert(ge != NULL);

    Geom::Point const s = snap_knot_position(p, state);

    ge->ry.computed = fabs( ge->cy.computed - s[Geom::Y] );

    if ( state & GDK_CONTROL_MASK ) {
        ge->rx.computed = ge->ry.computed;
    }

    item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

Geom::Point
ArcKnotHolderEntityRY::knot_get() const
{
    SPGenericEllipse const *ge = dynamic_cast<SPGenericEllipse *>(item);
    g_assert(ge != NULL);

    return (Geom::Point(ge->cx.computed, ge->cy.computed) -  Geom::Point(0, ge->ry.computed));
}

void
ArcKnotHolderEntityRY::knot_click(guint state)
{
    SPGenericEllipse *ge = dynamic_cast<SPGenericEllipse *>(item);
    g_assert(ge != NULL);

    if (state & GDK_CONTROL_MASK) {
        ge->rx.computed = ge->ry.computed;
        ge->updateRepr();
    }
}

ArcKnotHolder::ArcKnotHolder(SPDesktop *desktop, SPItem *item, SPKnotHolderReleasedFunc relhandler) :
    KnotHolder(desktop, item, relhandler)
{
    ArcKnotHolderEntityRX *entity_rx = new ArcKnotHolderEntityRX();
    ArcKnotHolderEntityRY *entity_ry = new ArcKnotHolderEntityRY();
    ArcKnotHolderEntityStart *entity_start = new ArcKnotHolderEntityStart();
    ArcKnotHolderEntityEnd *entity_end = new ArcKnotHolderEntityEnd();

    entity_rx->create(desktop, item, this, Inkscape::CTRL_TYPE_SIZER,
                      _("Adjust ellipse <b>width</b>, with <b>Ctrl</b> to make circle"),
                      SP_KNOT_SHAPE_SQUARE, SP_KNOT_MODE_XOR);

    entity_ry->create(desktop, item, this, Inkscape::CTRL_TYPE_SIZER,
                      _("Adjust ellipse <b>height</b>, with <b>Ctrl</b> to make circle"),
                      SP_KNOT_SHAPE_SQUARE, SP_KNOT_MODE_XOR);

    entity_start->create(desktop, item, this, Inkscape::CTRL_TYPE_ROTATE,
                         _("Position the <b>start point</b> of the arc or segment; with <b>Ctrl</b> "
                           "to snap angle; drag <b>inside</b> the ellipse for arc, <b>outside</b> for segment"),
                         SP_KNOT_SHAPE_CIRCLE, SP_KNOT_MODE_XOR);

    entity_end->create(desktop, item, this, Inkscape::CTRL_TYPE_ROTATE,
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
    virtual Geom::Point knot_get() const;
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual void knot_click(guint state);
};

class StarKnotHolderEntity2 : public KnotHolderEntity {
public:
    virtual Geom::Point knot_get() const;
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual void knot_click(guint state);
};

void
StarKnotHolderEntity1::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint state)
{
    SPStar *star = dynamic_cast<SPStar *>(item);
    g_assert(star != NULL);

    Geom::Point const s = snap_knot_position(p, state);

    Geom::Point d = s - star->center;

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
    star->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

void
StarKnotHolderEntity2::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint state)
{
    SPStar *star = dynamic_cast<SPStar *>(item);
    g_assert(star != NULL);

    Geom::Point const s = snap_knot_position(p, state);

    if (star->flatsided == false) {
        Geom::Point d = s - star->center;

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
        star->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
    }
}

Geom::Point
StarKnotHolderEntity1::knot_get() const
{
    g_assert(item != NULL);

    SPStar const *star = dynamic_cast<SPStar const *>(item);
    g_assert(star != NULL);

    return sp_star_get_xy(star, SP_STAR_POINT_KNOT1, 0);

}

Geom::Point
StarKnotHolderEntity2::knot_get() const
{
    g_assert(item != NULL);

    SPStar const *star = dynamic_cast<SPStar const *>(item);
    g_assert(star != NULL);

    return sp_star_get_xy(star, SP_STAR_POINT_KNOT2, 0);
}

static void
sp_star_knot_click(SPItem *item, guint state)
{
    SPStar *star = dynamic_cast<SPStar *>(item);
    g_assert(star != NULL);

    if (state & GDK_MOD1_MASK) {
        star->randomized = 0;
        star->updateRepr();
    } else if (state & GDK_SHIFT_MASK) {
        star->rounded = 0;
        star->updateRepr();
    } else if (state & GDK_CONTROL_MASK) {
        star->arg[1] = star->arg[0] + M_PI / star->sides;
        star->updateRepr();
    }
}

void
StarKnotHolderEntity1::knot_click(guint state)
{
    sp_star_knot_click(item, state);
}

void
StarKnotHolderEntity2::knot_click(guint state)
{
    sp_star_knot_click(item, state);
}

StarKnotHolder::StarKnotHolder(SPDesktop *desktop, SPItem *item, SPKnotHolderReleasedFunc relhandler) :
    KnotHolder(desktop, item, relhandler)
{
    SPStar *star = dynamic_cast<SPStar *>(item);
    g_assert(item != NULL);

    StarKnotHolderEntity1 *entity1 = new StarKnotHolderEntity1();
    entity1->create(desktop, item, this, Inkscape::CTRL_TYPE_SHAPER,
                    _("Adjust the <b>tip radius</b> of the star or polygon; "
                      "with <b>Shift</b> to round; with <b>Alt</b> to randomize"));

    entity.push_back(entity1);

    if (star->flatsided == false) {
        StarKnotHolderEntity2 *entity2 = new StarKnotHolderEntity2();
        entity2->create(desktop, item, this, Inkscape::CTRL_TYPE_SHAPER,
                        _("Adjust the <b>base radius</b> of the star; with <b>Ctrl</b> to keep star rays "
                          "radial (no skew); with <b>Shift</b> to round; with <b>Alt</b> to randomize"));
        entity.push_back(entity2);
    }

    add_pattern_knotholder();
}

/* SPSpiral */

class SpiralKnotHolderEntityInner : public KnotHolderEntity {
public:
    virtual Geom::Point knot_get() const;
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual void knot_click(guint state);
};

class SpiralKnotHolderEntityOuter : public KnotHolderEntity {
public:
    virtual Geom::Point knot_get() const;
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

    SPSpiral *spiral = dynamic_cast<SPSpiral *>(item);
    g_assert(spiral != NULL);

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
        spiral->getPolar(spiral->t0, NULL, &arg_t0);

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

    spiral->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
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

    SPSpiral *spiral = dynamic_cast<SPSpiral *>(item);
    g_assert(spiral != NULL);

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
        spiral->getPolar(1, NULL, &arg_1);

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
            spiral->getPolar(t_temp, &rad_new, NULL);

        // change the revo (converting diff from radians to the number of turns)
        spiral->revo += diff/(2*M_PI);
        if (spiral->revo < 1e-3)
            spiral->revo = 1e-3;

        // if alt not pressed and the values are sane, change the rad
        if (!(state & GDK_MOD1_MASK) && rad_new > 1e-3 && rad_new/spiral->rad < 2) {
            // adjust t0 too so that the inner point stays unmoved
            double r0;
            spiral->getPolar(spiral->t0, &r0, NULL);
            spiral->rad = rad_new;
            spiral->t0 = pow(r0 / spiral->rad, 1.0/spiral->exp);
        }
        if (!IS_FINITE(spiral->t0)) spiral->t0 = 0.0;
        spiral->t0 = CLAMP(spiral->t0, 0.0, 0.999);
    }

    spiral->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

Geom::Point
SpiralKnotHolderEntityInner::knot_get() const
{
    SPSpiral const *spiral = dynamic_cast<SPSpiral const *>(item);
    g_assert(spiral != NULL);

    return spiral->getXY(spiral->t0);
}

Geom::Point
SpiralKnotHolderEntityOuter::knot_get() const
{
    SPSpiral const *spiral = dynamic_cast<SPSpiral const *>(item);
    g_assert(spiral != NULL);

    return spiral->getXY(1.0);
}

void
SpiralKnotHolderEntityInner::knot_click(guint state)
{
    SPSpiral *spiral = dynamic_cast<SPSpiral *>(item);
    g_assert(spiral != NULL);

    if (state & GDK_MOD1_MASK) {
        spiral->exp = 1;
        spiral->updateRepr();
    } else if (state & GDK_SHIFT_MASK) {
        spiral->t0 = 0;
        spiral->updateRepr();
    }
}

SpiralKnotHolder::SpiralKnotHolder(SPDesktop *desktop, SPItem *item, SPKnotHolderReleasedFunc relhandler) :
    KnotHolder(desktop, item, relhandler)
{
    SpiralKnotHolderEntityInner *entity_inner = new SpiralKnotHolderEntityInner();
    SpiralKnotHolderEntityOuter *entity_outer = new SpiralKnotHolderEntityOuter();

    entity_inner->create(desktop, item, this, Inkscape::CTRL_TYPE_SHAPER,
                         _("Roll/unroll the spiral from <b>inside</b>; with <b>Ctrl</b> to snap angle; "
                           "with <b>Alt</b> to converge/diverge"));

    entity_outer->create(desktop, item, this, Inkscape::CTRL_TYPE_SHAPER,
                         _("Roll/unroll the spiral from <b>outside</b>; with <b>Ctrl</b> to snap angle; "
                           "with <b>Shift</b> to scale/rotate; with <b>Alt</b> to lock radius"));

    entity.push_back(entity_inner);
    entity.push_back(entity_outer);

    add_pattern_knotholder();
}

/* SPOffset */

class OffsetKnotHolderEntity : public KnotHolderEntity {
public:
    virtual Geom::Point knot_get() const;
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
};

void
OffsetKnotHolderEntity::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint /*state*/)
{
    SPOffset *offset = dynamic_cast<SPOffset *>(item);
    g_assert(offset != NULL);

    offset->rad = sp_offset_distance_to_original(offset, p);
    offset->knot = p;
    offset->knotSet = true;

    offset->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}


Geom::Point
OffsetKnotHolderEntity::knot_get() const
{
    SPOffset const *offset = dynamic_cast<SPOffset const *>(item);
    g_assert(offset != NULL);

    Geom::Point np;
    sp_offset_top_point(offset,&np);
    return np;
}

OffsetKnotHolder::OffsetKnotHolder(SPDesktop *desktop, SPItem *item, SPKnotHolderReleasedFunc relhandler) :
    KnotHolder(desktop, item, relhandler)
{
    OffsetKnotHolderEntity *entity_offset = new OffsetKnotHolderEntity();
    entity_offset->create(desktop, item, this, Inkscape::CTRL_TYPE_SHAPER,
                          _("Adjust the <b>offset distance</b>"));
    entity.push_back(entity_offset);

    add_pattern_knotholder();
}

// TODO: this is derived from RectKnotHolderEntityWH because it used the same static function
// set_internal as the latter before KnotHolderEntity was C++ified. Check whether this also makes
// sense logically.
class FlowtextKnotHolderEntity : public RectKnotHolderEntityWH {
public:
    virtual Geom::Point knot_get() const;
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
};

Geom::Point
FlowtextKnotHolderEntity::knot_get() const
{
    SPRect const *rect = dynamic_cast<SPRect const *>(item);
    g_assert(rect != NULL);

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
    entity_flowtext->create(desktop, item, this, Inkscape::CTRL_TYPE_SHAPER,
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
