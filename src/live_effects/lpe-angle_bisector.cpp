/*
 * Authors:
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *   Johan Engelen <j.b.c.engelen@alumnus.utwente.nl>
 *
 * Copyright (C) Authors 2007-2012
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/i18n.h>

#include "live_effects/lpe-angle_bisector.h"

#include <2geom/path.h>
#include <2geom/sbasis-to-bezier.h>

#include "sp-lpe-item.h"
#include "knot-holder-entity.h"
#include "knotholder.h"

namespace Inkscape {
namespace LivePathEffect {

namespace AB {

class KnotHolderEntityLeftEnd : public LPEKnotHolderEntity {
public:
    KnotHolderEntityLeftEnd(LPEAngleBisector* effect) : LPEKnotHolderEntity(effect) {};
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual Geom::Point knot_get() const;
};

class KnotHolderEntityRightEnd : public LPEKnotHolderEntity {
public:
    KnotHolderEntityRightEnd(LPEAngleBisector* effect) : LPEKnotHolderEntity(effect) {};
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual Geom::Point knot_get() const;
};

} // namespace AB

LPEAngleBisector::LPEAngleBisector(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    length_left(_("Length left:"), _("Specifies the left end of the bisector"), "length-left", &wr, this, 0),
    length_right(_("Length right:"), _("Specifies the right end of the bisector"), "length-right", &wr, this, 250)
{
    show_orig_path = true;
    _provides_knotholder_entities = true;

    registerParameter( dynamic_cast<Parameter *>(&length_left) );
    registerParameter( dynamic_cast<Parameter *>(&length_right) );
}

LPEAngleBisector::~LPEAngleBisector()
{
}

Geom::PathVector
LPEAngleBisector::doEffect_path (Geom::PathVector const & path_in)
{
    using namespace Geom;

    // we assume that the path has >= 3 nodes
    ptA = path_in[0].pointAt(1);
    Point B = path_in[0].initialPoint();
    Point C = path_in[0].pointAt(2);

    double angle = angle_between(B - ptA, C - ptA);

    dir = unit_vector(B - ptA) * Rotate(angle/2);

    Geom::Point D = ptA - dir * length_left;
    Geom::Point E = ptA + dir * length_right;

    Piecewise<D2<SBasis> > output = Piecewise<D2<SBasis> >(D2<SBasis>(SBasis(D[X], E[X]), SBasis(D[Y], E[Y])));

    return path_from_piecewise(output, LPE_CONVERSION_TOLERANCE);
}

void
LPEAngleBisector::addKnotHolderEntities(KnotHolder *knotholder, SPDesktop *desktop, SPItem *item) {
    {
        KnotHolderEntity *e = new AB::KnotHolderEntityLeftEnd(this);
        e->create( desktop, item, knotholder, Inkscape::CTRL_TYPE_UNKNOWN,
                   _("Adjust the \"left\" end of the bisector") );
        knotholder->add(e);
    }
    {
        KnotHolderEntity *e = new AB::KnotHolderEntityRightEnd(this);
        e->create( desktop, item, knotholder, Inkscape::CTRL_TYPE_UNKNOWN,
                   _("Adjust the \"right\" end of the bisector") );
        knotholder->add(e);
    }
};

namespace AB {

void
KnotHolderEntityLeftEnd::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint state)
{
    LPEAngleBisector *lpe = dynamic_cast<LPEAngleBisector *>(_effect);

    Geom::Point const s = snap_knot_position(p, state);

    double lambda = Geom::nearest_time(s, lpe->ptA, lpe->dir);
    lpe->length_left.param_set_value(-lambda);

    sp_lpe_item_update_patheffect (SP_LPE_ITEM(item), false, true);
}

void
KnotHolderEntityRightEnd::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint state)
{
    LPEAngleBisector *lpe = dynamic_cast<LPEAngleBisector *>(_effect);

    Geom::Point const s = snap_knot_position(p, state);

    double lambda = Geom::nearest_time(s, lpe->ptA, lpe->dir);
    lpe->length_right.param_set_value(lambda);

    sp_lpe_item_update_patheffect (SP_LPE_ITEM(item), false, true);
}

Geom::Point
KnotHolderEntityLeftEnd::knot_get() const
{
    LPEAngleBisector const* lpe = dynamic_cast<LPEAngleBisector const*>(_effect);
    return lpe->ptA - lpe->dir * lpe->length_left;
}

Geom::Point
KnotHolderEntityRightEnd::knot_get() const
{
    LPEAngleBisector const* lpe = dynamic_cast<LPEAngleBisector const*>(_effect);
    return lpe->ptA + lpe->dir * lpe->length_right;
}

} // namespace AB

/* ######################## */

} //namespace LivePathEffect
} /* namespace Inkscape */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
