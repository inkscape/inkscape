#define INKSCAPE_LPE_EXTRUDE_CPP
/** \file
 * @brief LPE effect for extruding paths (making them "3D").
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

#include <2geom/path.h>
#include <2geom/piecewise.h>
#include <2geom/transforms.h>

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

    case 1: {
        Piecewise<D2<SBasis> > pwd2_out;
        bool closed_path = are_near(pwd2_in.firstValue(), pwd2_in.lastValue());
        // split input path in pieces between points where deriv == vector
        Piecewise<D2<SBasis> > deriv = derivative(pwd2_in);
        std::vector<double> rts = roots(dot(deriv, rot90(extrude_vector.getVector())));
        double portion_t = 0.;
        for (unsigned i = 0; i < rts.size() ; ++i) {
            Piecewise<D2<SBasis> > cut = portion(pwd2_in, portion_t, rts[i] );
            portion_t = rts[i];
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
            cut.continuousConcat(portion(pwd2_in, pwd2_in.domain().min(), rts[0] ));
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
LPEExtrude::resetDefaults(SPItem * item)
{
    Effect::resetDefaults(item);

    using namespace Geom;

    Geom::OptRect bbox = item->getBounds(Geom::identity(), SPItem::GEOMETRIC_BBOX);
    if (bbox) {
        Interval boundingbox_X = (*bbox)[Geom::X];
        Interval boundingbox_Y = (*bbox)[Geom::Y];
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
