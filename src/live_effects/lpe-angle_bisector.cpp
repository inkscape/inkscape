#define INKSCAPE_LPE_ANGLE_BISECTOR_CPP
/** \file
 * LPE <angle_bisector> implementation, used as an example for a base starting class
 * when implementing new LivePathEffects.
 *
 * In vi, three global search-and-replaces will let you rename everything
 * in this and the .h file:
 *
 *   :%s/ANGLE_BISECTOR/YOURNAME/g
 *   :%s/AngleBisector/Yourname/g
 *   :%s/angle_bisector/yourname/g
 */
/*
 * Authors:
 *   Johan Engelen
 *
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-angle_bisector.h"

// You might need to include other 2geom files. You can add them here:
#include <2geom/path.h>
#include <2geom/sbasis-to-bezier.h>

namespace Inkscape {
namespace LivePathEffect {

namespace AB {

class KnotHolderEntityLeftEnd : public KnotHolderEntity
{
public:
    virtual bool isLPEParam() { return true; }

    virtual void knot_set(NR::Point const &p, NR::Point const &origin, guint state);
    virtual NR::Point knot_get();
};

class KnotHolderEntityRightEnd : public KnotHolderEntity
{
public:
    virtual bool isLPEParam() { return true; }

    virtual void knot_set(NR::Point const &p, NR::Point const &origin, guint state);
    virtual NR::Point knot_get();
};

} // namespace TtC

LPEAngleBisector::LPEAngleBisector(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    length_left(_("Length left"), _("Specifies the left end of the bisector"), "length-left", &wr, this, 0),
    length_right(_("Length right"), _("Specifies the right end of the bisector"), "length-right", &wr, this, 250)
{
    show_orig_path = true;

    registerParameter( dynamic_cast<Parameter *>(&length_left) );
    registerParameter( dynamic_cast<Parameter *>(&length_right) );

    /* we disable the handles until we support both knotholders and nodepaths */
    //registerKnotHolderHandle(new AB::KnotHolderEntityLeftEnd(), _("Adjust the \"right\" end of the bisector"));
    //registerKnotHolderHandle(new AB::KnotHolderEntityRightEnd(), _("Adjust the point of attachment of the bisector"));
}

LPEAngleBisector::~LPEAngleBisector()
{
}

std::vector<Geom::Path>
LPEAngleBisector::doEffect_path (std::vector<Geom::Path> const & path_in)
{
    using namespace Geom;

    std::vector<Geom::Path> path_out;

    // we assume that the path has >= 3 nodes
    ptA = path_in[0].pointAt(1);
    Point B = path_in[0].initialPoint();
    Point C = path_in[0].pointAt(2);

    double angle = angle_between(B - ptA, C - ptA);

    dir = unit_vector(B - ptA) * Rotate(angle/2);

    Geom::Point D = ptA - dir * length_left;
    Geom::Point E = ptA + dir * length_right;

    Piecewise<D2<SBasis> > output = Piecewise<D2<SBasis> >(D2<SBasis>(Linear(D[X], E[X]), Linear(D[Y], E[Y])));

    return path_from_piecewise(output, LPE_CONVERSION_TOLERANCE);
}
namespace AB {

// TODO: make this more generic
static LPEAngleBisector *
get_effect(SPItem *item)
{
    Effect *effect = sp_lpe_item_get_current_lpe(SP_LPE_ITEM(item));
    if (effect->effectType() != ANGLE_BISECTOR) {
        g_print ("Warning: Effect is not of type LPEAngleBisector!\n");
        return NULL;
    }
    return static_cast<LPEAngleBisector *>(effect);
}

void
KnotHolderEntityLeftEnd::knot_set(NR::Point const &p, NR::Point const &/*origin*/, guint /*state*/)
{
    LPEAngleBisector *lpe = get_effect(item);
    
    double lambda = Geom::nearest_point(p.to_2geom(), lpe->ptA, lpe->dir);
    lpe->length_left.param_set_value(-lambda);

    sp_lpe_item_update_patheffect (SP_LPE_ITEM(item), false, true);
}

void
KnotHolderEntityRightEnd::knot_set(NR::Point const &p, NR::Point const &/*origin*/, guint /*state*/)
{
    LPEAngleBisector *lpe = get_effect(item);
    
    double lambda = Geom::nearest_point(p.to_2geom(), lpe->ptA, lpe->dir);
    lpe->length_right.param_set_value(lambda);

    sp_lpe_item_update_patheffect (SP_LPE_ITEM(item), false, true);
}

NR::Point
KnotHolderEntityLeftEnd::knot_get()
{
    LPEAngleBisector *lpe = get_effect(item);
    return lpe->ptA - lpe->dir * lpe->length_left;
}

NR::Point
KnotHolderEntityRightEnd::knot_get()
{
    LPEAngleBisector *lpe = get_effect(item);
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
