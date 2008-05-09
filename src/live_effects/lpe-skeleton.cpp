#define INKSCAPE_LPE_SKELETON_CPP
/** \file
 * LPE <skeleton> implementation, used as an example for a base starting class
 * when implementing new LivePathEffects.
 *
 * In vi, three global search-and-replaces will let you rename everything
 * in this and the .h file:
 *
 *   :%s/SKELETON/YOURNAME/g
 *   :%s/Skeleton/Yourname/g
 *   :%s/skeleton/yourname/g
 */
/*
 * Authors:
 *   Johan Engelen
*
* Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-skeleton.h"

// You might need to include other 2geom files. You can add them here:
#include <2geom/path.h>

namespace Inkscape {
namespace LivePathEffect {

LPESkeleton::LPESkeleton(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    // initialise your parameters here:
    number(_("Float parameter"), _("just a real number like 1.4!"), "svgname", &wr, this, 1.2)
{
    // register all your parameters here, so Inkscape knows which parameters this effect has:
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

std::vector<Geom::Path>
LPESkeleton::doEffect_path (std::vector<Geom::Path> & path_in)
{
        std::vector<Geom::Path> path_out;

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
