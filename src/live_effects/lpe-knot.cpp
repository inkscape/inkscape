#define INKSCAPE_LPE_KNOT_CPP
/** \file
 * LPE <knot> implementation
 */
/*
 * Authors:
 *   Johan Engelen
*
* Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-knot.h"
#include "display/curve.h"
#include <libnr/n-art-bpath.h>

// You might need to include other 2geom files. You can add them here:
#include <2geom/path.h>
#include <2geom/sbasis.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/d2.h>
#include <2geom/sbasis-math.h>
#include <2geom/piecewise.h>
#include <2geom/crossing.h>
#include <2geom/path-intersection.h>

namespace Inkscape {
namespace LivePathEffect {

LPEKnot::LPEKnot(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    // initialise your parameters here:
    interruption_width(_("Interruption width"), _("Howmuch the lower strand is obscured by the upper."), "interruption_width", &wr, this, 10)
{
    // register all your parameters here, so Inkscape knows which parameters this effect has:
    registerParameter( dynamic_cast<Parameter *>(&interruption_width) );
}

LPEKnot::~LPEKnot()
{

}


/* ########################
 *  Choose to implement one of the doEffect functions. You can delete or comment out the others.
*/

/*
void
LPEKnot::doEffect (SPCurve * curve)
{
    // spice this up to make the effect actually *do* something!
}

NArtBpath *
LPEKnot::doEffect_nartbpath (NArtBpath * path_in)
{
        NArtBpath *path_out;
        unsigned ret = 0;
        while ( path_in[ret].code != NR_END ) {
            ++ret;
        }
        unsigned len = ++ret;
        path_out = g_new(NArtBpath, len);

        memcpy(path_out, path_in, len * sizeof(NArtBpath));   // spice this up to make the effect actually *do* something!

        return path_out;
}
*/

std::vector<Geom::Interval> complementOf(Geom::Interval I, std::vector<Geom::Interval> domain){
    std::vector<Geom::Interval> ret;
    double min = domain.front().min();
    double max = domain.back().max();
    Geom::Interval I1 = Geom::Interval(min,I.min());
    Geom::Interval I2 = Geom::Interval(I.max(),max);

    for (unsigned i = 0; i<domain.size(); i++){
        boost::optional<Geom::Interval> I1i = intersect(domain.at(i),I1);
        if (I1i) ret.push_back(I1i.get());
        boost::optional<Geom::Interval> I2i = intersect(domain.at(i),I2);
        if (I2i) ret.push_back(I2i.get());
    }
    return ret;
}

Geom::Interval
findShadowedTime(Geom::Path &patha, 
                 Geom::Path &pathb, 
                 Geom::Crossing crossing, 
                 unsigned idx, double width){
    using namespace Geom;
    double curveidx, timeoncurve = modf(crossing.getOtherTime(idx),&curveidx);
    if(curveidx == pathb.size() && timeoncurve == 0) { curveidx--; timeoncurve = 0.99999;}
    assert(curveidx >= 0 && curveidx < pathb.size());
    
    std::vector<Point> MV = pathb[unsigned(curveidx)].pointAndDerivatives(timeoncurve,2);
    Point T = unit_vector(MV.at(1));
    Point N = T.cw();
    Point A = MV.at(0)-10*width*T, B = MV.at(0)+10*width*T;
    
    std::vector<Geom::Path> cutter;
    Geom::Path cutterLeft  = Geom::Path();
    Geom::Path cutterRight = Geom::Path();
    cutterLeft.append (LineSegment (A-width*N, B-width*N));
    cutterRight.append(LineSegment (A+width*N, B+width*N));
    cutter.push_back(cutterLeft);
    cutter.push_back(cutterRight);
    
    std::vector<Geom::Path> patha_as_vect = std::vector<Geom::Path>(1,patha);
    
    CrossingSet crossingTable = crossings (patha_as_vect, cutter);
    double t0 = crossing.getTime(idx);
    double tmin = 0,tmax = patha.size()-0.0001;
    assert(crossingTable.size()>=1);
    for (unsigned c=0; c<crossingTable.front().size(); c++){
        double t = crossingTable.front().at(c).ta;
        assert(crossingTable.front().at(c).a==0);
        if (t>tmin and t<t0) tmin = t;
        if (t<tmax and t>t0) tmax = t;
    }
    //return Interval(t0-0.1,t0+0.1);
    return Interval(tmin,tmax);
}                


std::vector<Geom::Path>
LPEKnot::doEffect_path (std::vector<Geom::Path> & path_in)
{
    using namespace Geom;
    std::vector<Geom::Path> path_out;
    double width = interruption_width;
    
    CrossingSet crossingTable = crossings_among(path_in);
    for (unsigned i = 0; i < crossingTable.size(); i++){
        std::vector<Interval> dom;
        dom.push_back(Interval(0.,path_in.at(i).size()-0.00001));
        //TODO: handle closed curves...
        for (unsigned crs = 0; crs < crossingTable.at(i).size(); crs++){
            Crossing crossing = crossingTable.at(i).at(crs);
            unsigned j = crossing.getOther(i);
            //TODO: select dir according to a parameter...
            if ((crossing.dir and crossing.a==i) or (not crossing.dir and crossing.b==i) or (i==j)){
                if (i==j and not crossing.dir) {
                    double temp = crossing.ta;
                    crossing.ta = crossing.tb; 
                    crossing.tb = temp; 
                    crossing.dir = not crossing.dir;
                }
                Interval hidden = findShadowedTime(path_in.at(i),path_in.at(j),crossing,i,width);                
                dom = complementOf(hidden,dom);
            }
        }
        for (unsigned comp = 0; comp < dom.size(); comp++){
            assert(dom.at(comp).min() >=0 and dom.at(comp).max() < path_in.at(i).size());
            path_out.push_back(path_in.at(i).portion(dom.at(comp)));
        }
        
//         std::vector<Point> MV = path_in[0][0].pointAndDerivatives(crossingTable[0][0].getTime(0),2);
//         Point U = unit_vector(MV[1]);
//         Point N = U.cw();
//         Point A = MV[0]-10.*width*U, B = MV[0]+10*width*U;
        
//         Geom::Path cutter;
//         cutter = Geom::Path();
//         cutter.append( LineSegment(A+width*N,B+width*N));
//         path_out.push_back(cutter);
//         cutter = Geom::Path();
//         cutter.append( LineSegment(A-width*N,B-width*N));
//         path_out.push_back(cutter);

    }   
    return path_out;
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

Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPEKnot::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > & pwd2_in)
{
    using namespace Geom;


    Piecewise<D2<SBasis> > output;
    Piecewise<D2<SBasis> > m = addLinearEnds(pwd2_in);

    Piecewise<D2<SBasis> > v = derivative(pwd2_in);
    Piecewise<D2<SBasis> > n = unitVector(v);

// // -------- Pleins et delies vs courbure ou direction...
//     Piecewise<D2<SBasis> > a = derivative(v);
//     Piecewise<SBasis> a_cross_n = cross(a,n);
//     Piecewise<SBasis> v_dot_n = dot(v,n);
//     //Piecewise<D2<SBasis> > rfrac = sectionize(D2<Piecewise<SBasis> >(a_cross_n,v_dot_n));
//     //Piecewise<SBasis> h = atan2(rfrac)*interruption_width;
//     Piecewise<SBasis> h = reciprocal(curvature(pwd2_in))*interruption_width;
//
// //    Piecewise<D2<SBasis> > dir = Piecewise<D2<SBasis> >(D2<SBasis>(Linear(0),Linear(-1)));
// //    Piecewise<SBasis> h = dot(n,dir)+1.;
// //    h *= h*(interruption_width/4.);
//
//     n = rot90(n);
//     output = pwd2_in+h*n;
//     output.concat(pwd2_in-h*n);
//
// //-----------

    //output.concat(m);
    return output;
}
*/

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
