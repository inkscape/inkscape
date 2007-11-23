#define INKSCAPE_LPE_PATHALONGPATH_CPP

/*
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-pathalongpath.h"
#include "sp-shape.h"
#include "sp-item.h"
#include "sp-path.h"
#include "display/curve.h"
#include <libnr/n-art-bpath.h>
#include <libnr/nr-matrix-fns.h>
#include "live_effects/n-art-bpath-2geom.h"
#include "svg/svg.h"
#include "ui/widget/scalar.h"

#include <2geom/sbasis.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/d2.h>
#include <2geom/piecewise.h>

#include <algorithm>
using std::vector;


/* Theory in e-mail from J.F. Barraud
Let B be the skeleton path, and P the pattern (the path to be deformed).

P is a map t --> P(t) = ( x(t), y(t) ).
B is a map t --> B(t) = ( a(t), b(t) ).

The first step is to re-parametrize B by its arc length: this is the parametrization in which a point p on B is located by its distance s from start. One obtains a new map s --> U(s) = (a'(s),b'(s)), that still describes the same path B, but where the distance along B from start to
U(s) is s itself.

We also need a unit normal to the path. This can be obtained by computing a unit tangent vector, and rotate it by 90°. Call this normal vector N(s).

The basic deformation associated to B is then given by:

   (x,y) --> U(x)+y*N(x)

(i.e. we go for distance x along the path, and then for distance y along the normal)

Of course this formula needs some minor adaptations (as is it depends on the absolute position of P for instance, so a little translation is needed
first) but I think we can first forget about them.
*/

namespace Inkscape {
namespace LivePathEffect {

static const Util::EnumData<PAPCopyType> PAPCopyTypeData[PAPCT_END] = {
    {PAPCT_SINGLE,               N_("Single"),               "single"},
    {PAPCT_SINGLE_STRETCHED,     N_("Single, stretched"),    "single_stretched"},
    {PAPCT_REPEATED,             N_("Repeated"),             "repeated"},
    {PAPCT_REPEATED_STRETCHED,   N_("Repeated, stretched"),  "repeated_stretched"}
};
static const Util::EnumDataConverter<PAPCopyType> PAPCopyTypeConverter(PAPCopyTypeData, PAPCT_END);

LPEPathAlongPath::LPEPathAlongPath(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    bend_path(_("Bend path"), _("Path along which to bend the original path"), "bendpath", &wr, this, "M0,0 L1,0"),
    width_path(_("Width path"), _("..."), "widthpath", &wr, this, "M0,0 L1,0"),
    copytype(_("Path copies"), _("How many copies to place along the skeleton path"), "copytype", PAPCopyTypeConverter, &wr, this, PAPCT_SINGLE_STRETCHED),
    prop_scale(_("Width"), _("Width of the path"), "prop_scale", &wr, this, 1),
    scale_y_rel(_("Width in units of length"), _("Scale the width of the path in units of its length"), "scale_y_rel", &wr, this, false),
    vertical_pattern(_("Original path is vertical"), "", "vertical", &wr, this, false)
{
    registerParameter( dynamic_cast<Parameter *>(&bend_path) );
    registerParameter( dynamic_cast<Parameter *>(&width_path) );
    registerParameter( dynamic_cast<Parameter *>(&copytype) );
    registerParameter( dynamic_cast<Parameter *>(&prop_scale) );
    registerParameter( dynamic_cast<Parameter *>(&scale_y_rel) );
    registerParameter( dynamic_cast<Parameter *>(&vertical_pattern) );

    prop_scale.param_set_digits(3);
    prop_scale.param_set_increments(0.01, 0.10);
}

LPEPathAlongPath::~LPEPathAlongPath()
{

}


Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPEPathAlongPath::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > & pwd2_in)
{
    using namespace Geom;

/* Much credit should go to jfb and mgsloan of lib2geom development for the code below! */

    PAPCopyType type = copytype.get_value();

    Piecewise<D2<SBasis> > uskeleton = arc_length_parametrization(Piecewise<D2<SBasis> >(bend_path),2,.1);
    uskeleton = remove_short_cuts(uskeleton,.01);
    Rect uskeletonbounds = bounds_exact(uskeleton);
    uskeleton -= uskeletonbounds.midpoint();

    Piecewise<D2<SBasis> > n = rot90(derivative(uskeleton));
    n = force_continuity(remove_short_cuts(n,.1));

    D2<Piecewise<SBasis> > patternd2 = make_cuts_independant(pwd2_in);
    Piecewise<SBasis> x = vertical_pattern.get_value() ? Piecewise<SBasis>(patternd2[1]) : Piecewise<SBasis>(patternd2[0]);
    Piecewise<SBasis> y = vertical_pattern.get_value() ? Piecewise<SBasis>(patternd2[0]) : Piecewise<SBasis>(patternd2[1]);
    Interval pattBnds = bounds_exact(x);
    x -= pattBnds.min();
    Interval pattBndsY = bounds_exact(y);
    y -= pattBndsY.middle();

    int nbCopies = int(uskeleton.cuts.back()/pattBnds.extent());
    double scaling = 1;

    switch(type) {
        case PAPCT_REPEATED:
            break;

        case PAPCT_SINGLE:
            nbCopies = (nbCopies > 0) ? 1 : 0;
            break;

        case PAPCT_SINGLE_STRETCHED:
            nbCopies = 1;
            scaling = uskeleton.cuts.back()/pattBnds.extent();
            break;

        case PAPCT_REPEATED_STRETCHED:
             scaling = uskeleton.cuts.back()/(((double)nbCopies)*pattBnds.extent());
            break;

        default:
            return pwd2_in;
    };

    double pattWidth = pattBnds.extent() * scaling;

    if (scaling != 1.0) {
        x*=scaling;
    }
    if ( scale_y_rel.get_value() ) {
        y*=(scaling*prop_scale);
    } else {
        if (prop_scale != 1.0) y *= prop_scale;
    }

    Piecewise<D2<SBasis> > widthpwd2 = arc_length_parametrization(Piecewise<D2<SBasis> >(width_path),2,.1);
    D2<Piecewise<SBasis> > widthd2pw = make_cuts_independant(widthpwd2);
    Piecewise<SBasis> width = (Piecewise<SBasis>(widthd2pw[Y]) - uskeletonbounds[Y].middle()) * wfactor;


    double offs = 0;
    Piecewise<D2<SBasis> > output;
    for (int i=0; i<nbCopies; i++){
        output.concat(compose(uskeleton,x+offs) + y*compose(width,x+offs)*compose(n,x+offs));
        offs+=pattWidth;
    }

    output += Point(pattBnds.middle(), pattBndsY.middle());

    return output;
}

void
LPEPathAlongPath::resetDefaults(SPItem * item)
{
    if (!SP_IS_PATH(item)) return;

    using namespace Geom;

    // set the bend path to run horizontally in the middle of the bounding box of the original path
    Piecewise<D2<SBasis> > pwd2;
    std::vector<Path> temppath = SVGD_to_2GeomPath( SP_OBJECT_REPR(item)->attribute("inkscape:original-d"));
    for (unsigned int i=0; i < temppath.size(); i++) {
        pwd2.concat( temppath[i].toPwSb() );
    }

    D2<Piecewise<SBasis> > d2pw = make_cuts_independant(pwd2);
    Interval bndsX = bounds_exact(d2pw[0]);
    Interval bndsY = bounds_exact(d2pw[1]);
    Point start(bndsX.min(), (bndsY.max()+bndsY.min())/2);
    Point end(bndsX.max(), (bndsY.max()+bndsY.min())/2);

    Geom::Path path;
    path.start( start );
    path.appendNew<Geom::LineSegment>( end );
    bend_path.param_set_and_write_new_value( path.toPwSb() );

    Point startw(bndsX.min(), bndsY.max());
    Point endw(bndsX.max(), bndsY.max());
    Geom::Path pathw;
    pathw.start( startw );
    pathw.appendNew<Geom::LineSegment>( endw );
    width_path.param_set_and_write_new_value( pathw.toPwSb() );
    
    wfactor = 1/(startw[Y]-start[Y]);
}

} // namespace LivePathEffect
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
