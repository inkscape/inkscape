/**
 * @file
 * LPE effect for extruding paths (making them "3D").
 *
 */
/* Authors:
 *   Johan Engelen <j.b.c.engelen@utwente.nl>
 *
 * Copyright (C) 2009 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-extrude.h"

#include <glibmm/i18n.h>

#include <2geom/path.h>
#include <2geom/piecewise.h>
#include <2geom/transforms.h>
#include <algorithm>

#include "sp-item.h"

namespace Inkscape {
namespace LivePathEffect {

LPEExtrude::LPEExtrude(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    extrude_vector(_("Direction"), _("Defines the direction and magnitude of the extrusion"), "extrude_vector", &wr, this, Geom::Point(-10,10))
{
    show_orig_path = true;
    concatenate_before_pwd2 = false;

    registerParameter( dynamic_cast<Parameter *>(&extrude_vector) );
}

LPEExtrude::~LPEExtrude()
{

}

static bool are_colinear(Geom::Point a, Geom::Point b) {
    return Geom::are_near(cross(a,b), 0., 0.5);
}

// find cusps, except at start/end for closed paths.
// this should be factored out later.
static std::vector<double> find_cusps( Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in ) {
    using namespace Geom;
    Piecewise<D2<SBasis> > deriv = derivative(pwd2_in);
    std::vector<double> cusps;
    // cusps are spots where the derivative jumps.
    for (unsigned i = 1 ; i < deriv.size() ; ++i) {
        if ( ! are_colinear(deriv[i-1].at1(), deriv[i].at0()) ) {
            // there is a jump in the derivative, so add it to the cusps list
            cusps.push_back(deriv.cuts[i]);
        }
    }
    return cusps;
}

Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPEExtrude::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    using namespace Geom;

    // generate connecting lines (the 'sides' of the extrusion)
    Path path(Point(0.,0.));
    path.appendNew<Geom::LineSegment>( extrude_vector.getVector() );
    Piecewise<D2<SBasis> > connector = path.toPwSb();

    switch( 1 ) {
    case 0: {
        /* This one results in the following subpaths: the original, a displaced copy, and connector lines between the two
         */

        Piecewise<D2<SBasis> > pwd2_out = pwd2_in;
        // generate extrusion bottom: (just a copy of original path, displaced a bit)
        pwd2_out.concat( pwd2_in + extrude_vector.getVector() );

        // connecting lines should be put at start and end of path if it is not closed
        // it is not possible to check whether a piecewise<T> path is closed, 
        // so we check whether start and end are close
        if ( ! are_near(pwd2_in.firstValue(), pwd2_in.lastValue()) ) {
            pwd2_out.concat( connector + pwd2_in.firstValue() );
            pwd2_out.concat( connector + pwd2_in.lastValue() );
        }
        // connecting lines should be put at cusps
        Piecewise<D2<SBasis> > deriv = derivative(pwd2_in);
        std::vector<double> cusps; // = roots(deriv);
        for (unsigned i = 0; i < cusps.size() ; ++i) {
            pwd2_out.concat( connector + pwd2_in.valueAt(cusps[i]) );
        }
        // connecting lines should be put where the tangent of the path equals the extrude_vector in direction
        std::vector<double> rts = roots(dot(deriv, rot90(extrude_vector.getVector())));
        for (unsigned i = 0; i < rts.size() ; ++i) {
            pwd2_out.concat( connector + pwd2_in.valueAt(rts[i]) );
        }
        return pwd2_out;
    }

    default:
    case 1: {
        /* This one creates separate closed subpaths that correspond to the faces of the extruded shape.
         * When the LPE is complete, one can convert the shape to a normal path, then break subpaths apart and start coloring them.
         */

        Piecewise<D2<SBasis> > pwd2_out;
        // split input path in pieces between points where deriv == vector
        Piecewise<D2<SBasis> > deriv = derivative(pwd2_in);
        std::vector<double> rts = roots(dot(deriv, rot90(extrude_vector.getVector())));

        std::vector<double> cusps = find_cusps(pwd2_in);

        // see if we should treat the path as being closed.
        bool closed_path = false;
        if ( are_near(pwd2_in.firstValue(), pwd2_in.lastValue()) ) {
            // the path is closed, however if there is a cusp at the closing point, we should treat it as being an open path.
            if ( are_colinear(deriv.firstValue(), deriv.lastValue()) ) {
                // there is no jump in the derivative, so treat path as being closed
                closed_path = true;
            }
        }

        std::vector<double> connector_pts;
        if (rts.empty()) {
            connector_pts = cusps;
        } else if (cusps.empty()) {
            connector_pts = rts;
        } else {
            connector_pts = rts;
            connector_pts.insert(connector_pts.begin(), cusps.begin(), cusps.end());
            sort(connector_pts.begin(), connector_pts.end());
        }

        double portion_t = 0.;
        for (unsigned i = 0; i < connector_pts.size() ; ++i) {
            Piecewise<D2<SBasis> > cut = portion(pwd2_in, portion_t, connector_pts[i] );
            portion_t = connector_pts[i];
            if (closed_path && i == 0) {
                // if the path is closed, skip the first cut and add it to the last cut later
                continue;
            }
            Piecewise<D2<SBasis> > part = cut;
            part.continuousConcat(connector + cut.lastValue());
            part.continuousConcat(reverse(cut) + extrude_vector.getVector());
            part.continuousConcat(reverse(connector) + cut.firstValue());
            pwd2_out.concat( part );
        }
        if (closed_path) {
            Piecewise<D2<SBasis> > cut = portion(pwd2_in, portion_t, pwd2_in.domain().max() );
            cut.continuousConcat(portion(pwd2_in, pwd2_in.domain().min(), connector_pts[0] ));
            Piecewise<D2<SBasis> > part = cut;
            part.continuousConcat(connector + cut.lastValue());
            part.continuousConcat(reverse(cut) + extrude_vector.getVector());
            part.continuousConcat(reverse(connector) + cut.firstValue());
            pwd2_out.concat( part );
        } else if (!are_near(portion_t, pwd2_in.domain().max())) {
            Piecewise<D2<SBasis> > cut = portion(pwd2_in, portion_t, pwd2_in.domain().max() );
            Piecewise<D2<SBasis> > part = cut;
            part.continuousConcat(connector + cut.lastValue());
            part.continuousConcat(reverse(cut) + extrude_vector.getVector());
            part.continuousConcat(reverse(connector) + cut.firstValue());
            pwd2_out.concat( part );
        }
        return pwd2_out;
    }
    }
}

void
LPEExtrude::resetDefaults(SPItem const* item)
{
    Effect::resetDefaults(item);

    using namespace Geom;

    Geom::OptRect bbox = item->geometricBounds();
    if (bbox) {
        Interval const &boundingbox_X = (*bbox)[Geom::X];
        Interval const &boundingbox_Y = (*bbox)[Geom::Y];
        extrude_vector.set_and_write_new_values( Geom::Point(boundingbox_X.middle(), boundingbox_Y.middle()), 
                                                 (boundingbox_X.extent() + boundingbox_Y.extent())*Geom::Point(-0.05,0.2) );
    }
}

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
