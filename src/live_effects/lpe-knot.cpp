/** @file
 * @brief LPE knot effect implementation
 */
/* Authors:
 *   Jean-Francois Barraud <jf.barraud@gmail.com>
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-shape.h"
#include "display/curve.h"
#include "live_effects/lpe-knot.h"
#include "svg/svg.h"
#include "style.h"

#include <2geom/sbasis-to-bezier.h>
#include <2geom/sbasis.h>
#include <2geom/d2.h>
#include <2geom/d2-sbasis.h>
#include <2geom/piecewise.h>
#include <2geom/path.h>
#include <2geom/d2.h>
#include <2geom/crossing.h>
#include <2geom/path-intersection.h>
#include <2geom/elliptical-arc.h>

#include <exception>

namespace Inkscape {
namespace LivePathEffect {

class KnotHolderEntityCrossingSwitcher : public LPEKnotHolderEntity
{
public:
    virtual ~KnotHolderEntityCrossingSwitcher() {}

    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual Geom::Point knot_get();
    virtual void knot_click(guint state);
};


//---------------------------------------------------------------------------
//LPEKnot specific Interval manipulation.
//---------------------------------------------------------------------------

//remove an interval from an union of intervals.
//TODO: is it worth moving it to 2Geom?
static
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

//find the time interval during which patha is hidden by pathb near a given crossing.
// Warning: not accurate!
static
Geom::Interval
findShadowedTime(Geom::Path const &patha,
                 Geom::Path const &pathb,
                 Geom::Crossing const &crossing,
                 unsigned idx, double width){
    using namespace Geom;
    double curveidx, timeoncurve = modf(crossing.getOtherTime(idx),&curveidx);
    if(curveidx == pathb.size() ) { curveidx--; timeoncurve = 1.;}//FIXME: 0.99999; needed?
    assert(curveidx >= 0 && curveidx < pathb.size());

    std::vector<Point> MV = pathb[unsigned(curveidx)].pointAndDerivatives(timeoncurve,1);
    Point T = unit_vector(MV.at(1));
    Point N = T.cw();
    Point A = MV.at(0)-3*width*T, B = MV.at(0)+3*width*T;

    std::vector<Geom::Path> cutter;
    Geom::Path cutterPath(A-width*N);
    cutterPath.appendNew<LineSegment> (B-width*N);
    cutterPath.appendNew<LineSegment> (B+width*N);
    cutterPath.appendNew<LineSegment> (A+width*N);
    cutterPath.close();
    //cutterPath.appendNew<LineSegment> (A-width*N);
    cutter.push_back(cutterPath);

    std::vector<Geom::Path> patha_as_vect = std::vector<Geom::Path>(1,patha);

    CrossingSet crossingTable = crossings (patha_as_vect, cutter);
    double t0 = crossing.getTime(idx);
    double tmin = 0,tmax = patha.size();//-0.00001;FIXME: FIXED?
    assert(crossingTable.size()>=1);
    for (unsigned c=0; c<crossingTable.front().size(); c++){
        double t = crossingTable.front().at(c).ta;
        assert(crossingTable.front().at(c).a==0);
        if (t>tmin and t<t0) tmin = t;
        if (t<tmax and t>t0) tmax = t;
    }
    return Interval(tmin,tmax);
}

// TODO: Fix all this in 2geom!!!!
//---------------------------------------------------------------------------
// some 2Geom work around.
//---------------------------------------------------------------------------

//Cubic Bezier curves might self intersect; the 2geom code used to miss them.
//This is a quick work around; maybe not needed anymore? -- TODO: check!
//TODO/TOCHECK/TOFIX: I think the BUG is in path-intersection.cpp -> curve_mono_split(...):
//the derivative of the curve should be used instead of the curve itself.
std::vector<Geom::Path>
split_at_horiz_vert_tgt (std::vector<Geom::Path> const & path_in){
    std::vector<Geom::Path> ret;
    
    using namespace Geom;

    Piecewise<D2<SBasis> > f = paths_to_pw(path_in);
    D2<Piecewise<SBasis> > df = make_cuts_independent(derivative(f));
    std::vector<double> xyroots = roots(df[X]);
    std::vector<double> yroots = roots(df[Y]);
    xyroots.insert(xyroots.end(), yroots.begin(), yroots.end());
    std::sort(xyroots.begin(),xyroots.end());
    Piecewise<D2<SBasis> > newf = partition(f,xyroots);
    ret = path_from_piecewise(newf,LPE_CONVERSION_TOLERANCE);

    return ret;
}

//TODO: Fix this in 2Geom; I think CrossingSets should not contain duplicates.
Geom::CrossingSet crossingSet_remove_double(Geom::CrossingSet const &input){
    Geom::CrossingSet result(input.size());
    //Yeah, I know, there is a "unique" algorithm for that...
    //Note: I'm not sure the duplicates are always consecutive!! (can be first and last, I think)
    //Note: I also found crossings c with c.a==c.b and c.ta==c.tb . Is it normal?
    //Note: I also found crossings c with c.ta or c.tb not in path[a] or path[b] domain. This is definitely not normal.
    Geom::Crossing last;
    for( unsigned i=0; i<input.size(); i++){
        for( unsigned j=0; j<input[i].size(); j++){
            bool dup = false;
            for ( unsigned k=0; k<result[i].size(); k++){
                if ( input[i][j]==result[i][k] ){
                    dup = true;
                    g_warning("Duplicate found in a Geom::CrossingSet!");
                    break;
                }
            }
            if (!dup) {
                result[i].push_back( input[i][j] );
            }
        }
    }
    return result;
}

//---------------------------------------------------------------------------
//LPEKnot specific Crossing Data manipulation.
//---------------------------------------------------------------------------

//TODO: evaluate how usefull/lpeknot specific that is. Worth being moved to 2geom? (I doubt it)
namespace LPEKnotNS {

//Yet another crossing data representation. Not sure at all it is usefull!
// +: >Given a point, you immediately know which strings are meeting there,
//    and the index of the crossing along each string. This makes it easy to check 
//    topology change. In a CrossingSet, you have to do some search to know the index
//    of the crossing along "the second" string (i.e. find the symetric crossing)... 
//    >Each point is stored only once.    
//    However, we don't have so many crossing points in general, so none of these points might be relevant.
//    
// -: one more clumsy data representation, and "parallelism" failures to expect...
//    (in particular, duplicates are hateful with this respect...)

CrossingPoints::CrossingPoints(Geom::CrossingSet const &input, std::vector<Geom::Path> const &path) : std::vector<CrossingPoint>()
{
    using namespace Geom;
    //g_print("DBG>\nCrossing set content:\n");
    for( unsigned i=0; i<input.size(); i++){
        Crossings i_crossings = input[i];
        for( unsigned n=0; n<i_crossings.size(); n++ ){
            Crossing c = i_crossings[n];
            //g_print("DBG> [%u,%u]:(%u,%u) at times (%f,%f) ----->",i,n,c.a,c.b,c.ta,c.tb);
            unsigned j = c.getOther(i);
            if (i<j || (i==j && c.ta<=c.tb) ){//FIXME: equality should not happen, but does happen.
                CrossingPoint cp;
                double ti = c.getTime(i);
                //FIXME: times in crossing are sometimes out of range!!
                //if (0<ti || ti > 1)g_print("oops! -->");
                if (ti > 1) ti=1;
                if (ti < 0) ti=0;

                cp.pt = path[i].pointAt(c.getTime(i));
                cp.i = i;
                cp.j = j;
                cp.ni = n;
                Crossing c_bar = c;
                if (i==j){
                    c_bar.a  = c.b;
                    c_bar.b  = c.a; 
                    c_bar.ta = c.tb;
                    c_bar.tb = c.ta;
                    c_bar.dir = !c.dir;
                }
                cp.nj = std::find(input[j].begin(),input[j].end(),c_bar)-input[j].begin();
                cp.sign = 1;
                push_back(cp);
                //g_print("i=%u, ni=%u, j=%u, nj=%u\n",cp.i,cp.ni,cp.j,cp.nj);
            }/*
               else{
                //debug purpose only: 
                //This crossing is already registered in output. Just make sure it has a "mirror".
                g_print("deja trouve?");
                get(i,n);
                bool found = false;
                for( unsigned ii=0; ii<input.size(); ii++){
                    Crossings ii_crossings = input[ii];
                    for( unsigned nn=0; nn<ii_crossings.size(); nn++ ){
                        Crossing cc = ii_crossings[nn];
                        if (cc.b==c.a && cc.a==c.b && cc.ta==c.tb && cc.tb==c.ta) found = true;
                        if ( (ii!=i || nn!=n) && 
                             ( (cc.b==c.a && cc.a==c.b && cc.ta==c.tb && cc.tb==c.ta) ||
                               (cc.a==c.a && cc.b==c.b && cc.ta==c.ta && cc.tb==c.tb) 
                                 ) ) found = true;
                    }
                }
                assert( found );
                g_print("  oui!\n");
            }
             */
        }
    }

    //g_print("CrossingPoints reslut:\n");
    //for (unsigned k=0; k<size(); k++){
    //    g_print("cpts[%u]: i=%u, ni=%u, j=%u, nj=%u\n",k,(*this)[k].i,(*this)[k].ni,(*this)[k].j,(*this)[k].nj);
    //}

}

CrossingPoints::CrossingPoints(std::vector<double> const &input) : std::vector<CrossingPoint>()
{
    if (input.size()>0 && input.size()%7 ==0){
        using namespace Geom;
        for( unsigned n=0; n<input.size();  ){
            CrossingPoint cp;
            cp.pt[X] = input[n++];
            cp.pt[Y] = input[n++];
            cp.i = input[n++];
            cp.j = input[n++];
            cp.ni = input[n++];
            cp.nj = input[n++];
            cp.sign = input[n++];
            push_back(cp);
        }
    }
}

std::vector<double>
CrossingPoints::to_vector()
{
    using namespace Geom;
    std::vector<double> result;
    for( unsigned n=0; n<size(); n++){
        CrossingPoint cp = (*this)[n];
        result.push_back(cp.pt[X]);
        result.push_back(cp.pt[Y]);
        result.push_back(double(cp.i));
        result.push_back(double(cp.j));
        result.push_back(double(cp.ni));
        result.push_back(double(cp.nj));
        result.push_back(double(cp.sign));
    }
    return result;
}

//FIXME: rewrite to check success: return bool, put result in arg.
CrossingPoint
CrossingPoints::get(unsigned const i, unsigned const ni)
{
    for (unsigned k=0; k<size(); k++){
        if (
            ((*this)[k].i==i && (*this)[k].ni==ni) ||
            ((*this)[k].j==i && (*this)[k].nj==ni)
            ) return (*this)[k];
    }
    g_warning("LPEKnotNS::CrossingPoints::get error. %uth crossing along string %u not found.",ni,i);
    assert(false);//debug purpose...
    return CrossingPoint();
}

unsigned
idx_of_nearest(CrossingPoints const &cpts, Geom::Point const &p)
{
    double dist=-1;
    unsigned result = cpts.size();
    for (unsigned k=0; k<cpts.size(); k++){
        double dist_k = Geom::L2(p-cpts[k].pt);
        if (dist<0 || dist>dist_k){
            result = k;
            dist = dist_k;
        }
    }
    return result;
}

//TODO: Find a way to warn the user when the topology changes.
//TODO: be smarter at guessing the signs when the topology changed?
void
CrossingPoints::inherit_signs(CrossingPoints const &other, int default_value)
{
    bool topo_changed = false;
    for (unsigned n=0; n<size(); n++){
        if ( n<other.size() &&
             other[n].i  == (*this)[n].i  &&
             other[n].j  == (*this)[n].j  &&
             other[n].ni == (*this)[n].ni &&
             other[n].nj == (*this)[n].nj    )
        {
            (*this)[n].sign = other[n].sign;
        }else{
            topo_changed = true;
            break;
        }
    }
    if (topo_changed){
        //TODO: Find a way to warn the user!!
        for (unsigned n=0; n<size(); n++){
            Geom::Point p = (*this)[n].pt;
            unsigned idx = idx_of_nearest(other,p);
            if (idx<other.size()){
                (*this)[n].sign = other[idx].sign;
            }else{
                (*this)[n].sign = default_value;
            }
        }
    }
}

}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//LPEKnot effect.
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------


LPEKnot::LPEKnot(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    // initialise your parameters here:
    interruption_width(_("Interruption width"), _("Size of hidden region of lower string"), "interruption_width", &wr, this, 3),
    prop_to_stroke_width(_("unit of stroke width"), _("Consider 'Gap width' as a ratio of stroke width."), "prop_to_stroke_width", &wr, this, true),
    switcher_size(_("Switcher size"), _("Orientation indicator/switcher size"), "switcher_size", &wr, this, 15),
    crossing_points_vector(_("Crossing Signs"), _("Crossings signs"), "crossing_points_vector", &wr, this)
{
    // register all your parameters here, so Inkscape knows which parameters this effect has:
    registerParameter( dynamic_cast<Parameter *>(&interruption_width) );
    registerParameter( dynamic_cast<Parameter *>(&prop_to_stroke_width) );
    registerParameter( dynamic_cast<Parameter *>(&switcher_size) );
    registerParameter( dynamic_cast<Parameter *>(&crossing_points_vector) );

    registerKnotHolderHandle(new KnotHolderEntityCrossingSwitcher(), _("Drag to select a crossing, click to flip it"));
    crossing_points = LPEKnotNS::CrossingPoints();
    selectedCrossing = 0;
    switcher = Geom::Point(0,0);
}

LPEKnot::~LPEKnot()
{

}

void
LPEKnot::doOnApply(SPLPEItem *lpeitem)
{
    //SPCurve *curve = SP_SHAPE(lpeitem)->curve;
    // //TODO: where should the switcher be initialized? (it shows up here if there is no crossing at all)
    // //The best would be able to hide it when there is no crossing!
    //Geom::Point A = *(curve->first_point());
    //Geom::Point B = *(curve->last_point());
    //switcher = (A+B)*.5;
}

void
LPEKnot::updateSwitcher(){
    if (selectedCrossing < crossing_points.size()){
        switcher = crossing_points[selectedCrossing].pt;
    }else if (crossing_points.size()>0){
        selectedCrossing = 0;
        switcher = crossing_points[selectedCrossing].pt;
    }else{
        //TODO: is there a way to properly hide the helper.
        //switcher = Geom::Point(Geom::infinity(),Geom::infinity());
        switcher = Geom::Point(1e10,1e10);
    }
}

std::vector<Geom::Path>
LPEKnot::doEffect_path (std::vector<Geom::Path> const &input_path)
{
    using namespace Geom;
    std::vector<Geom::Path> path_out;
    //double width = interruption_width;
    double width = interruption_width;
    if ( prop_to_stroke_width.get_value() ) {
        width *= stroke_width;
    }

    LPEKnotNS::CrossingPoints old_crdata(crossing_points_vector.data());

    std::vector<Geom::Path> path_in = split_at_horiz_vert_tgt(input_path);

    CrossingSet crossingTable = crossings_among(path_in);

    crossingTable = crossingSet_remove_double(crossingTable);

    crossing_points = LPEKnotNS::CrossingPoints(crossingTable, path_in);
    crossing_points.inherit_signs(old_crdata);
    crossing_points_vector.param_set_and_write_new_value(crossing_points.to_vector());
    updateSwitcher();

    if (crossingTable.size()==0){
        return input_path;
    }

    for (unsigned i = 0; i < crossingTable.size(); i++){
        std::vector<Interval> dom;
        dom.push_back(Interval(0.,path_in.at(i).size()));//-0.00001));FIX ME: this should not be needed anymore.
        for (unsigned n = 0; n < crossingTable.at(i).size(); n++){
            Crossing crossing = crossingTable.at(i).at(n);
            unsigned j = crossing.getOther(i);

            //FIXME: check success...
            LPEKnotNS::CrossingPoint crpt;
            crpt = crossing_points.get(i,n);
            int sign_code = crpt.sign;

            if (sign_code!=0){
                bool sign = (sign_code>0 ? true : false);
                Interval hidden;
                if (((crossing.dir==sign) and crossing.a==i) or ((crossing.dir!=sign) and crossing.b==i)){
                    if (i==j and (crossing.dir!=sign)) {
                    double temp = crossing.ta;
                    crossing.ta = crossing.tb;
                    crossing.tb = temp;
                    crossing.dir = not crossing.dir;
                    }
                    hidden = findShadowedTime(path_in.at(i),path_in.at(j),crossing,i,width);
                }
                dom = complementOf(hidden,dom);
            }
        }
        for (unsigned comp = 0; comp < dom.size(); comp++){
            assert(dom.at(comp).min() >=0 and dom.at(comp).max() <= path_in.at(i).size());
            path_out.push_back(path_in.at(i).portion(dom.at(comp)));
        }
    }
    return path_out;
}



void
LPEKnot::doBeforeEffect (SPLPEItem *lpeitem)
{
    using namespace Geom;
    //FIXME: do we have to be more carefull to access stroke width?
    stroke_width = lpeitem->style->stroke_width.computed;
}


static LPEKnot *
get_effect(SPItem *item)
{
    Effect *effect = sp_lpe_item_get_current_lpe(SP_LPE_ITEM(item));
    if (effect->effectType() != KNOT) {
        g_print ("Warning: Effect is not of type LPEKnot!\n");
        return NULL;
    }
    return static_cast<LPEKnot *>(effect);
}

void
LPEKnot::addCanvasIndicators(SPLPEItem */*lpeitem*/, std::vector<Geom::PathVector> &hp_vec)
{
    using namespace Geom;
     double r = switcher_size*.1;
    char const * svgd;
    //TODO: use a nice path!
    if (selectedCrossing >= crossing_points.size()||crossing_points[selectedCrossing].sign > 0){
        //svgd = "M -10,0 A 10 10 0 1 0 0,-10 l  5,-1 -1,2";
        svgd = "m -7.07,7.07 c 3.9,3.91 10.24,3.91 14.14,0 3.91,-3.9 3.91,-10.24 0,-14.14 -3.9,-3.91 -10.24,-3.91 -14.14,0 l 2.83,-4.24 0.7,2.12";
    }else if (crossing_points[selectedCrossing].sign < 0){
        //svgd = "M  10,0 A 10 10 0 1 1 0,-10 l -5,-1  1,2";
        svgd = "m 7.07,7.07 c -3.9,3.91 -10.24,3.91 -14.14,0 -3.91,-3.9 -3.91,-10.24 0,-14.14 3.9,-3.91 10.24,-3.91 14.14,0 l -2.83,-4.24 -0.7,2.12";
    }else{
        //svgd = "M 10,0 A 10 10 0 1 0 -10,0 A 10 10 0 1 0 10,0 ";
        svgd = "M 10,0 C 10,5.52 5.52,10 0,10 -5.52,10 -10,5.52 -10,0 c 0,-5.52 4.48,-10 10,-10 5.52,0 10,4.48 10,10 z";
    }
    PathVector pathv = sp_svg_read_pathv(svgd);
    pathv *= Matrix(r,0,0,r,0,0);
    pathv+=switcher;
    hp_vec.push_back(pathv);
}

void
KnotHolderEntityCrossingSwitcher::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint state)
{
    LPEKnot* lpe = get_effect(item);

    lpe->selectedCrossing = idx_of_nearest(lpe->crossing_points,p);
    lpe->updateSwitcher();
//    if(lpe->selectedCrossing < lpe->crossing_points.size())
//        lpe->switcher = lpe->crossing_points[lpe->selectedCrossing].pt;
//    else
//        lpe->switcher = p;
    // FIXME: this should not directly ask for updating the item. It should write to SVG, which triggers updating.
    sp_lpe_item_update_patheffect (SP_LPE_ITEM(item), false, true);
}

Geom::Point
KnotHolderEntityCrossingSwitcher::knot_get()
{
    LPEKnot* lpe = get_effect(item);
    return snap_knot_position(lpe->switcher);
}

void
KnotHolderEntityCrossingSwitcher::knot_click(guint state)
{
    LPEKnot* lpe = get_effect(item);
    unsigned s = lpe->selectedCrossing;
    if (s < lpe->crossing_points.size()){
        if (state & GDK_SHIFT_MASK){
            lpe->crossing_points[s].sign = 1;
        }else{
            int sign = lpe->crossing_points[s].sign;
            lpe->crossing_points[s].sign = ((sign+2)%3)-1;
        }
        lpe->crossing_points_vector.param_set_and_write_new_value(lpe->crossing_points.to_vector());

        // FIXME: this should not directly ask for updating the item. It should write to SVG, which triggers updating.
        sp_lpe_item_update_patheffect (SP_LPE_ITEM(item), false, true);
    }
}


/* ######################## */

} // namespace LivePathEffect
} // namespace Inkscape

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
