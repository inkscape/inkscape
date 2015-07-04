/**
 * @file
 * LPE sketch effect implementation.
 */
/* Authors:
 *   Jean-Francois Barraud <jf.barraud@gmail.com>
 *   Johan Engelen <j.b.c.engelen@utwente.nl>
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-sketch.h"

#include <glibmm/i18n.h>

// You might need to include other 2geom files. You can add them here:
#include <2geom/path.h>
#include <2geom/sbasis.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/sbasis-math.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/d2.h>
#include <2geom/sbasis-math.h>
#include <2geom/piecewise.h>
#include <2geom/crossing.h>
#include <2geom/path-intersection.h>

namespace Inkscape {
namespace LivePathEffect {

LPESketch::LPESketch(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    // initialise your parameters here:
    //testpointA(_("Test Point A"), _("Test A"), "ptA", &wr, this, Geom::Point(100,100)),
    nbiter_approxstrokes(_("Strokes:"), _("Draw that many approximating strokes"), "nbiter_approxstrokes", &wr, this, 5),
    strokelength(_("Max stroke length:"),
                 _("Maximum length of approximating strokes"), "strokelength", &wr, this, 100.),
    strokelength_rdm(_("Stroke length variation:"),
                     _("Random variation of stroke length (relative to maximum length)"), "strokelength_rdm", &wr, this, .3),
    strokeoverlap(_("Max. overlap:"),
                  _("How much successive strokes should overlap (relative to maximum length)"), "strokeoverlap", &wr, this, .3),
    strokeoverlap_rdm(_("Overlap variation:"),
                      _("Random variation of overlap (relative to maximum overlap)"), "strokeoverlap_rdm", &wr, this, .3),
    ends_tolerance(_("Max. end tolerance:"),
                   _("Maximum distance between ends of original and approximating paths (relative to maximum length)"), "ends_tolerance", &wr, this, .1),
    parallel_offset(_("Average offset:"),
                    _("Average distance each stroke is away from the original path"), "parallel_offset", &wr, this, 5.),
    tremble_size(_("Max. tremble:"),
                 _("Maximum tremble magnitude"), "tremble_size", &wr, this, 5.),
    tremble_frequency(_("Tremble frequency:"),
                      _("Average number of tremble periods in a stroke"), "tremble_frequency", &wr, this, 1.)
#ifdef LPE_SKETCH_USE_CONSTRUCTION_LINES
    ,nbtangents(_("Construction lines:"),
               _("How many construction lines (tangents) to draw"), "nbtangents", &wr, this, 5),
    tgtscale(_("Scale:"),
             _("Scale factor relating curvature and length of construction lines (try 5*offset)"), "tgtscale", &wr, this, 10.0),
    tgtlength(_("Max. length:"), _("Maximum length of construction lines"), "tgtlength", &wr, this, 100.0),
    tgtlength_rdm(_("Length variation:"), _("Random variation of the length of construction lines"), "tgtlength_rdm", &wr, this, .3),
    tgt_places_rdmness(_("Placement randomness:"), _("0: evenly distributed construction lines, 1: purely random placement"), "tgt_places_rdmness", &wr, this, 1.)
#ifdef LPE_SKETCH_USE_CURVATURE
    ,min_curvature(_("k_min:"), _("min curvature"), "k_min", &wr, this, 4.0)
    ,max_curvature(_("k_max:"), _("max curvature"), "k_max", &wr, this, 1000.0)
#endif
#endif
{
    // register all your parameters here, so Inkscape knows which parameters this effect has:
    //Add some comment in the UI:  *warning* the precise output of this effect might change in future releases!
    //convert to path if you want to keep exact output unchanged in future releases...
    //registerParameter( dynamic_cast<Parameter *>(&testpointA) );
    registerParameter( dynamic_cast<Parameter *>(&nbiter_approxstrokes) );
    registerParameter( dynamic_cast<Parameter *>(&strokelength) );
    registerParameter( dynamic_cast<Parameter *>(&strokelength_rdm) );
    registerParameter( dynamic_cast<Parameter *>(&strokeoverlap) );
    registerParameter( dynamic_cast<Parameter *>(&strokeoverlap_rdm) );
    registerParameter( dynamic_cast<Parameter *>(&ends_tolerance) );
    registerParameter( dynamic_cast<Parameter *>(&parallel_offset) );
    registerParameter( dynamic_cast<Parameter *>(&tremble_size) );
    registerParameter( dynamic_cast<Parameter *>(&tremble_frequency) );
#ifdef LPE_SKETCH_USE_CONSTRUCTION_LINES
    registerParameter( dynamic_cast<Parameter *>(&nbtangents) );
    registerParameter( dynamic_cast<Parameter *>(&tgt_places_rdmness) );
    registerParameter( dynamic_cast<Parameter *>(&tgtscale) );
    registerParameter( dynamic_cast<Parameter *>(&tgtlength) );
    registerParameter( dynamic_cast<Parameter *>(&tgtlength_rdm) );
#ifdef LPE_SKETCH_USE_CURVATURE
    registerParameter( dynamic_cast<Parameter *>(&min_curvature) );
    registerParameter( dynamic_cast<Parameter *>(&max_curvature) );
#endif
#endif

    nbiter_approxstrokes.param_make_integer();
    nbiter_approxstrokes.param_set_range(0, Geom::infinity());
    strokelength.param_set_range(1, Geom::infinity());
    strokelength.param_set_increments(1., 5.);
    strokelength_rdm.param_set_range(0, 1.);
    strokeoverlap.param_set_range(0, 1.);
    strokeoverlap.param_set_increments(0.1, 0.30);
    ends_tolerance.param_set_range(0., 1.);
    parallel_offset.param_set_range(0, Geom::infinity());
    tremble_frequency.param_set_range(0.01, 100.);
    tremble_frequency.param_set_increments(.5, 1.5);
    strokeoverlap_rdm.param_set_range(0, 1.);

#ifdef LPE_SKETCH_USE_CONSTRUCTION_LINES
    nbtangents.param_make_integer();
    nbtangents.param_set_range(0, Geom::infinity());
    tgtscale.param_set_range(0, Geom::infinity());
    tgtscale.param_set_increments(.1, .5);
    tgtlength.param_set_range(0, Geom::infinity());
    tgtlength.param_set_increments(1., 5.);
    tgtlength_rdm.param_set_range(0, 1.);
    tgt_places_rdmness.param_set_range(0, 1.);
    //this is not very smart, but required to avoid having lot of tangents stacked on short components.
    //Nota: we could specify a density instead of an absolute number, but this would be scale dependant.
    concatenate_before_pwd2 = true;
#endif
}

LPESketch::~LPESketch()
{

}

/*
Geom::Piecewise<Geom::D2<Geom::SBasis> >
addLinearEnds (Geom::Piecewise<Geom::D2<Geom::SBasis> > & m){
    using namespace Geom;
    Piecewise<D2<SBasis> > output;
    Piecewise<D2<SBasis> > start;
    Piecewise<D2<SBasis> > end;
    double x,y,vx,vy;

    x  = m.segs.front()[0].at0();
    y  = m.segs.front()[1].at0();
    vx = m.segs.front()[0][1][0]+Tri(m.segs.front()[0][0]);
    vy = m.segs.front()[1][1][0]+Tri(m.segs.front()[1][0]);
    start = Piecewise<D2<SBasis> >(D2<SBasis>(Linear (x-vx,x),Linear (y-vy,y)));
    start.offsetDomain(m.cuts.front()-1.);

    x  = m.segs.back()[0].at1();
    y  = m.segs.back()[1].at1();
    vx = -m.segs.back()[0][1][1]+Tri(m.segs.back()[0][0]);;
    vy = -m.segs.back()[1][1][1]+Tri(m.segs.back()[1][0]);;
    end = Piecewise<D2<SBasis> >(D2<SBasis>(Linear (x,x+vx),Linear (y,y+vy)));
    //end.offsetDomain(m.cuts.back());

    output = start;
    output.concat(m);
    output.concat(end);
    return output;
}
*/



//This returns a random perturbation. Notice the domain is [s0,s0+first multiple of period>s1]...
Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPESketch::computePerturbation (double s0, double s1){
    using namespace Geom;
    Piecewise<D2<SBasis> >res;

    //global offset for this stroke.
    double offsetX = 2*parallel_offset-parallel_offset.get_value();
    double offsetY = 2*parallel_offset-parallel_offset.get_value();
    Point A,dA,B,dB,offset = Point(offsetX,offsetY);
    //start point A
    for (unsigned dim=0; dim<2; dim++){
        A[dim]  = offset[dim] + 2*tremble_size-tremble_size.get_value();
        dA[dim] = 2*tremble_size-tremble_size.get_value();
    }
    //compute howmany deg 3 sbasis to concat according to frequency.

    unsigned count = unsigned((s1-s0)/strokelength*tremble_frequency)+1; 
    //unsigned count = unsigned((s1-s0)/tremble_frequency)+1; 

    for (unsigned i=0; i<count; i++){
        D2<SBasis> perturb = D2<SBasis>(SBasis(2, Linear()), SBasis(2, Linear()));
        for (unsigned dim=0; dim<2; dim++){
            B[dim] = offset[dim] + 2*tremble_size-tremble_size.get_value();
            perturb[dim][0] = Linear(A[dim],B[dim]);
            dA[dim] = dA[dim]-B[dim]+A[dim];
            //avoid dividing by 0. Very short strokes will have ends parallel to the curve...
            if ( s1-s0 > 1e-2)
                dB[dim] = -(2*tremble_size-tremble_size.get_value())/(s0-s1)-B[dim]+A[dim];
            else
                dB[dim] = -(2*tremble_size-tremble_size.get_value())-B[dim]+A[dim];
            perturb[dim][1] = Linear(dA[dim],dB[dim]);
        }
        dA = B-A-dB;
        A = B;
        //dA = B-A-dB;
        res.concat(Piecewise<D2<SBasis> >(perturb));
    }
    res.setDomain(Interval(s0,s0+count*strokelength/tremble_frequency));
    //res.setDomain(Interval(s0,s0+count*tremble_frequency));
    return res;
}


// Main effect body...
Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPESketch::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    using namespace Geom;
    //If the input path is empty, do nothing.
    //Note: this happens when duplicating a 3d box... dunno why.
    if (pwd2_in.size()==0) return pwd2_in;

    Piecewise<D2<SBasis> > output;

    // some variables for futur use (for construction lines; compute arclength only once...)
    // notations will be : t = path time, s = distance from start along the path.
    Piecewise<SBasis> pathlength;
    double total_length = 0;

    //TODO: split Construction Lines/Approximated Strokes into two separate effects?

    //----- Approximated Strokes.
    std::vector<Piecewise<D2<SBasis> > > pieces_in = split_at_discontinuities (pwd2_in);

    //work separately on each component.
    for (unsigned pieceidx = 0; pieceidx < pieces_in.size(); pieceidx++){

        Piecewise<D2<SBasis> > piece = pieces_in[pieceidx];
        Piecewise<SBasis> piecelength = arcLengthSb(piece,.1);
        double piece_total_length = piecelength.segs.back().at1()-piecelength.segs.front().at0();
        pathlength.concat(piecelength + total_length);
        total_length += piece_total_length;


        //TODO: better check this on the Geom::Path.
        bool closed = piece.segs.front().at0() == piece.segs.back().at1();
        if (closed){
            piece.concat(piece);
            piecelength.concat(piecelength+piece_total_length);
        }

        for (unsigned i = 0; i<nbiter_approxstrokes; i++){
            //Basic steps:
            //- Choose a rdm seg [s0,s1], find corresponding [t0,t1],
            //- Pick a rdm perturbation delta(s), collect 'piece(t)+delta(s(t))' over [t0,t1] into output.

            // pick a point where to start the stroke (s0 = dist from start).
            double s1=0.,s0 = ends_tolerance*strokelength+0.0001;//the root finder might miss 0.
            double t1, t0;
            double s0_initial = s0;
            bool done = false;// was the end of the component reached?

            while (!done){
                // if the start point is already too far... do nothing. (this should not happen!)
                if (!closed && s1>piece_total_length - ends_tolerance.get_value()*strokelength) break;
                if ( closed && s0>piece_total_length + s0_initial) break;

                std::vector<double> times;
                times = roots(piecelength-s0);
                t0 = times.at(0);//there should be one and only one solution!!

                // pick a new end point (s1 = s0 + strokelength).
                s1 = s0 + strokelength*(1-strokelength_rdm);
                // don't let it go beyond the end of the orgiginal path.
                // TODO/FIXME: this might result in short strokes near the end...
                if (!closed && s1>piece_total_length-ends_tolerance.get_value()*strokelength){
                    done = true;
                    //!!the root solver might miss s1==piece_total_length...
                    if (s1>piece_total_length){s1 = piece_total_length - ends_tolerance*strokelength-0.0001;}
                }
                if (closed && s1>piece_total_length + s0_initial){
                    done = true;
                    if (closed && s1>2*piece_total_length){
                        s1 = 2*piece_total_length - strokeoverlap*(1-strokeoverlap_rdm)*strokelength-0.0001;
                    }
                }
                times = roots(piecelength-s1);
                if (times.empty()) break;//we should not be there.
                t1 = times[0];

                //pick a rdm perturbation, and collect the perturbed piece into output.
                Piecewise<D2<SBasis> > pwperturb = computePerturbation(s0-0.01,s1+0.01);
                pwperturb = compose(pwperturb,portion(piecelength,t0,t1));

                output.concat(portion(piece,t0,t1)+pwperturb);

                //step points: s0 = s1 - overlap.
                //TODO: make sure this has to end?
                s0 = s1 - strokeoverlap*(1-strokeoverlap_rdm)*(s1-s0);
            }
        }
    }

#ifdef LPE_SKETCH_USE_CONSTRUCTION_LINES

    //----- Construction lines.
    //TODO: choose places according to curvature?.

    //at this point we should have:
    //pathlength = arcLengthSb(pwd2_in,.1);
    //total_length = pathlength.segs.back().at1()-pathlength.segs.front().at0();
    Piecewise<D2<SBasis> > m = pwd2_in;
    Piecewise<D2<SBasis> > v = derivative(pwd2_in);
    Piecewise<D2<SBasis> > a = derivative(v);

#ifdef LPE_SKETCH_USE_CURVATURE
    //---- curvature experiment...(enable
    Piecewise<SBasis> k = curvature(pwd2_in);
    OptInterval k_bnds = bounds_exact(abs(k));
    double k_min = k_bnds->min() + k_bnds->extent() * min_curvature;
    double k_max = k_bnds->min() + k_bnds->extent() * max_curvature;

    Piecewise<SBasis> bump;
    //SBasis bump_seg = SBasis( 2, Linear(0) );
    //bump_seg[1] = Linear( 4. );
    SBasis bump_seg = SBasis( 1, Linear(1) );
    bump.push_cut( k_bnds->min() - 1 );
    bump.push( Linear(0), k_min );
    bump.push(bump_seg,k_max);
    bump.push( Linear(0), k_bnds->max()+1 );
        
    Piecewise<SBasis> repartition = compose( bump, k );
    repartition = integral(repartition);
    //-------------------------------
#endif

    for (unsigned i=0; i<nbtangents; i++){

        // pick a point where to draw a tangent (s = dist from start along path).
#ifdef LPE_SKETCH_USE_CURVATURE
        double proba = repartition.firstValue()+ (rand()%100)/100.*(repartition.lastValue()-repartition.firstValue());
        std::vector<double> times;
        times = roots(repartition - proba);
        double t = times.at(0);//there should be one and only one solution!
#else
        //double s = total_length * ( i + tgtlength_rdm ) / (nbtangents+1.);
        double reg_place = total_length * ( i + .5) / ( nbtangents );
        double rdm_place = total_length * tgt_places_rdmness;
        double s = ( 1.- tgt_places_rdmness.get_value() ) * reg_place  +  rdm_place ;
        std::vector<double> times;
        times = roots(pathlength-s);
        double t = times.at(0);//there should be one and only one solution!
#endif
        Point m_t = m(t), v_t = v(t), a_t = a(t);
        //Compute tgt length according to curvature (not exceeding tgtlength) so that
        //  dist to origninal curve ~ 4 * (parallel_offset+tremble_size).
        //TODO: put this 4 as a parameter in the UI...
        //TODO: what if with v=0?
        double l = tgtlength*(1-tgtlength_rdm)/v_t.length();
        double r = std::pow(v_t.length(), 3) / cross(v_t, a_t);
        r = sqrt((2*fabs(r)-tgtscale)*tgtscale)/v_t.length();
        l=(r<l)?r:l;
        //collect the tgt segment into output.
        D2<SBasis> tgt = D2<SBasis>();
        for (unsigned dim=0; dim<2; dim++){
            tgt[dim] = SBasis(Linear(m_t[dim]-v_t[dim]*l, m_t[dim]+v_t[dim]*l));
        }
        output.concat(Piecewise<D2<SBasis> >(tgt));
    }
#endif

    return output;
}

void
LPESketch::doBeforeEffect (SPLPEItem const*/*lpeitem*/)
{
    //init random parameters.
    parallel_offset.resetRandomizer();
    strokelength_rdm.resetRandomizer();
    strokeoverlap_rdm.resetRandomizer();
    ends_tolerance.resetRandomizer();
    tremble_size.resetRandomizer();
#ifdef LPE_SKETCH_USE_CONSTRUCTION_LINES
    tgtlength_rdm.resetRandomizer();
    tgt_places_rdmness.resetRandomizer();
#endif
}

/* ######################## */

} //namespace LivePathEffect (setq default-directory "c:/Documents And Settings/jf/Mes Documents/InkscapeSVN")
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
