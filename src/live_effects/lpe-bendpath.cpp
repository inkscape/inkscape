#define INKSCAPE_LPE_BENDPATH_CPP

/*
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 * Copyright (C) Steren Giannini 2008 <steren.giannini@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-bendpath.h"
#include "sp-shape.h"
#include "sp-item.h"
#include "sp-path.h"
#include "sp-item-group.h"
#include "display/curve.h"
#include <libnr/n-art-bpath.h>
#include <libnr/nr-matrix-fns.h>
#include "libnr/n-art-bpath-2geom.h"
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

LPEBendPath::LPEBendPath(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    bend_path(_("Bend path"), _("Path along which to bend the original path"), "bendpath", &wr, this, "M0,0 L1,0"),
    prop_scale(_("Width"), _("Width of the path"), "prop_scale", &wr, this, 1),
    scale_y_rel(_("Width in units of length"), _("Scale the width of the path in units of its length"), "scale_y_rel", &wr, this, false),
    vertical_pattern(_("Original path is vertical"), _("Rotates the original 90 degrees, before bending it along the bend path"), "vertical", &wr, this, false)
{
    registerParameter( dynamic_cast<Parameter *>(&bend_path) );
    registerParameter( dynamic_cast<Parameter *>(&prop_scale) );
    registerParameter( dynamic_cast<Parameter *>(&scale_y_rel) );
    registerParameter( dynamic_cast<Parameter *>(&vertical_pattern) );

    prop_scale.param_set_digits(3);
    prop_scale.param_set_increments(0.01, 0.10);

    concatenate_before_pwd2 = true;

    groupSpecialBehavior = false;
}

LPEBendPath::~LPEBendPath()
{

}

void
LPEBendPath::doBeforeEffect (SPLPEItem *lpeitem)
{
    if(SP_IS_GROUP(lpeitem))
    {
        groupSpecialBehavior = true;

        original_bbox(lpeitem);
    }
}

Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPEBendPath::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    using namespace Geom;

/* Much credit should go to jfb and mgsloan of lib2geom development for the code below! */

    Piecewise<D2<SBasis> > uskeleton = arc_length_parametrization(Piecewise<D2<SBasis> >(bend_path.get_pwd2()),2,.1);
    uskeleton = remove_short_cuts(uskeleton,.01);
    Piecewise<D2<SBasis> > n = rot90(derivative(uskeleton));
    n = force_continuity(remove_short_cuts(n,.1));

    D2<Piecewise<SBasis> > patternd2 = make_cuts_independant(pwd2_in);
    Piecewise<SBasis> x = vertical_pattern.get_value() ? Piecewise<SBasis>(patternd2[1]) : Piecewise<SBasis>(patternd2[0]);
    Piecewise<SBasis> y = vertical_pattern.get_value() ? Piecewise<SBasis>(patternd2[0]) : Piecewise<SBasis>(patternd2[1]);

//We use the group bounding box size or the path bbox size to translate well x and y
    if(groupSpecialBehavior == false)
    {
        boundingbox_X = bounds_exact(x);
        boundingbox_Y = bounds_exact(y);
    }
    x-= vertical_pattern.get_value() ? boundingbox_Y.min() : boundingbox_X.min();
    y-= vertical_pattern.get_value() ? boundingbox_X.middle() : boundingbox_Y.middle();

  double scaling = uskeleton.cuts.back()/boundingbox_X.extent();

  if (scaling != 1.0) {
        x*=scaling;
    }

    if ( scale_y_rel.get_value() ) {
        y*=(scaling*prop_scale);
    } else {
        if (prop_scale != 1.0) y *= prop_scale;
    }


    Piecewise<D2<SBasis> > output = compose(uskeleton,x) + y*compose(n,x);
    return output;
}

void
LPEBendPath::resetDefaults(SPItem * item)
{
    original_bbox(SP_LPE_ITEM(item));

    Geom::Point start(boundingbox_X.min(), (boundingbox_Y.max()+boundingbox_Y.min())/2);
    Geom::Point end(boundingbox_X.max(), (boundingbox_Y.max()+boundingbox_Y.min())/2);

    if ( Geom::are_near(start,end) ) {
        end += Geom::Point(1.,0.);
    }
     
    Geom::Path path;
    path.start( start );
    path.appendNew<Geom::LineSegment>( end );
    bend_path.param_set_and_write_new_value( path.toPwSb() );
}

void
LPEBendPath::transform_multiply(Geom::Matrix const& postmul, bool set)
{
    // TODO: implement correct transformation instead of this default behavior
    Effect::transform_multiply(postmul, set);
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
