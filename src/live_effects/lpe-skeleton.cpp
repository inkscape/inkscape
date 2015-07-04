/**
 * @file
 * Minimal dummy LPE effect implementation, used as an example for a base
 * starting class when implementing new LivePathEffects.
 *
 * In vi, three global search-and-replaces will let you rename everything
 * in this and the .h file:
 *
 *   :%s/SKELETON/YOURNAME/g
 *   :%s/Skeleton/Yourname/g
 *   :%s/skeleton/yourname/g
 */
/* Authors:
 *   Johan Engelen <j.b.c.engelen@alumnus.utwente.nl>
 *
 * Copyright (C) 2007-2012 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-skeleton.h"

// You might need to include other 2geom files. You can add them here:
#include <2geom/path.h>

#include <glibmm/i18n.h>

//#include "knot-holder-entity.h"
//#include "knotholder.h"

namespace Inkscape {
namespace LivePathEffect {

LPESkeleton::LPESkeleton(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    // initialise your parameters here:
    number(_("Float parameter"), _("just a real number like 1.4!"), "svgname", &wr, this, 1.2)
{
    /* uncomment the following line to have the original path displayed while the item is selected */
    //show_orig_path = true;
    /* uncomment the following line to enable display of the effect-specific on-canvas handles (knotholder entities) */
    //_provides_knotholder_entities

    /* register all your parameters here, so Inkscape knows which parameters this effect has: */
    registerParameter( dynamic_cast<Parameter *>(&number) );
}

LPESkeleton::~LPESkeleton()
{

}


/* ########################
 *  Choose to implement one of the doEffect functions. You can delete or comment out the others.
 */

/*
void
LPESkeleton::doEffect (SPCurve * curve)
{
    // spice this up to make the effect actually *do* something!
}

Geom::PathVector
LPESkeleton::doEffect_path (Geom::PathVector const & path_in)
{
        Geom::PathVector path_out;

        path_out = path_in;   // spice this up to make the effect actually *do* something!

        return path_out;
}
*/

Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPESkeleton::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    Geom::Piecewise<Geom::D2<Geom::SBasis> > output;

    output = pwd2_in;   // spice this up to make the effect actually *do* something!

    return output;
}

/* ########################
 *  If you want to provide effect-specific on-canvas handles (knotholder entities), define them here:
 */

/*
namespace Skeleton {

class KnotHolderEntityMyHandle : public LPEKnotHolderEntity
{
public:
    // the set() and get() methods must be implemented, click() is optional
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual Geom::Point knot_get() const;
    //virtual void knot_click(guint state);
};

} // namespace Skeleton

void
LPESkeleton::addKnotHolderEntities(KnotHolder *knotholder, SPDesktop *desktop, SPItem *item) {
    {
        KnotHolderEntityMyHandle *e = new KnotHolderEntityMyHandle(this);
        e->create(  desktop, item, knotholder,
                    _("Text describing what this handle does"),
                    //optional: knot_shape, knot_mode, knot_color);
        knotholder->add(e);
    }
};
*/

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
