/** \file
 * LPE <dynastroke> implementation
 */
/*
 * Authors:
 *   JF Barraud
*
* Copyright (C) JF Barraud 2007 <jf.barraud@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-dynastroke.h"
#include "display/curve.h"
//# include <libnr/n-art-bpath.h>

#include <2geom/path.h>
#include <2geom/sbasis.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/d2.h>
#include <2geom/sbasis-math.h>
#include <2geom/piecewise.h>

namespace Inkscape {
namespace LivePathEffect {
//TODO: growfor/fadefor can be expressed in unit of width.
//TODO: make round/sharp end choices independant for start and end.
//TODO: define more styles like in calligtool.
//TODO: allow fancy ends.

static const Util::EnumData<DynastrokeMethod> DynastrokeMethodData[DSM_END] = {
    {DSM_ELLIPTIC_PEN,     N_("Elliptic Pen"),        "elliptic_pen"},
    {DSM_THICKTHIN_FAST, N_("Thick-Thin strokes (fast)"),    "thickthin_fast"},
    {DSM_THICKTHIN_SLOW, N_("Thick-Thin strokes (slow)"),    "thickthin_slow"}
};
static const Util::EnumDataConverter<DynastrokeMethod> DSMethodConverter(DynastrokeMethodData, DSM_END);

static const Util::EnumData<DynastrokeCappingType> DynastrokeCappingTypeData[DSCT_END] = {
    {DSCT_SHARP, N_("Sharp"),    "sharp"},
    {DSCT_ROUND, N_("Round"),    "round"},
};
static const Util::EnumDataConverter<DynastrokeCappingType> DSCTConverter(DynastrokeCappingTypeData, DSCT_END);

LPEDynastroke::LPEDynastroke(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    // initialise your parameters here:
    method(_("Method:"), _("Choose pen type"), "method", DSMethodConverter, &wr, this, DSM_THICKTHIN_FAST),
    width(_("Pen width:"), _("Maximal stroke width"), "width", &wr, this, 25),
    roundness(_("Pen roundness:"), _("Min/Max width ratio"), "roundness", &wr, this, .2),
    angle(_("Angle:"), _("direction of thickest strokes (opposite = thinnest)"), "angle", &wr, this, 45),
//    modulo_pi(_("modulo pi"), _("Give forward and backward moves in one direction the same thickness "), "modulo_pi", &wr, this, false),
    start_cap(_("Start:"), _("Choose start capping type"), "start_cap", DSCTConverter, &wr, this, DSCT_SHARP),
    end_cap(_("End:"), _("Choose end capping type"), "end_cap", DSCTConverter, &wr, this, DSCT_SHARP),
    growfor(_("Grow for:"), _("Make the stroke thinner near it's start"), "growfor", &wr, this, 100),
    fadefor(_("Fade for:"), _("Make the stroke thinner near it's end"), "fadefor", &wr, this, 100),
    round_ends(_("Round ends"), _("Strokes end with a round end"), "round_ends", &wr, this, false),
    capping(_("Capping:"), _("left capping"), "capping", &wr, this, "M 100,5 C 50,5 0,0 0,0 0,0 50,-5 100,-5")
{

    registerParameter( dynamic_cast<Parameter *>(& method) );
    registerParameter( dynamic_cast<Parameter *>(& width) );
    registerParameter( dynamic_cast<Parameter *>(& roundness) );
    registerParameter( dynamic_cast<Parameter *>(& angle) );
    //registerParameter( dynamic_cast<Parameter *>(& modulo_pi) );
    registerParameter( dynamic_cast<Parameter *>(& start_cap) );
    registerParameter( dynamic_cast<Parameter *>(& growfor) );
    registerParameter( dynamic_cast<Parameter *>(& end_cap) );
    registerParameter( dynamic_cast<Parameter *>(& fadefor) );
    registerParameter( dynamic_cast<Parameter *>(& round_ends) );
    registerParameter( dynamic_cast<Parameter *>(& capping) );

    width.param_set_range(0, Geom::infinity());
    roundness.param_set_range(0.01, 1);
    angle.param_set_range(-360, 360);
    growfor.param_set_range(0, Geom::infinity());
    fadefor.param_set_range(0, Geom::infinity());

    show_orig_path = true;
}

LPEDynastroke::~LPEDynastroke()
{

}

Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPEDynastroke::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    using namespace Geom;

//     std::cout<<"do effect: debut\n";
    
     Piecewise<D2<SBasis> > output;
     Piecewise<D2<SBasis> > m = pwd2_in;
     Piecewise<D2<SBasis> > v = derivative(m);;
     Piecewise<D2<SBasis> > n = unitVector(v);
     n = rot90(n);
     Piecewise<D2<SBasis> > n1,n2;

//      for (unsigned i=0; i<n.size(); i++){
//          std::cout<<n[i][X]<<"\n";
//      }
//      return m + unitVector(v);

#if 0
     Piecewise<SBasis> k = curvature(m);
     OptInterval mag = bounds_exact(k);
     //TODO test if mag is non empty...
     k = (k-mag->min())*width/mag->extent() + (roundness*width);
     Piecewise<D2<SBasis> > left = m + k*n;
     Piecewise<D2<SBasis> > right = m - k*n;
     right = compose(right,Linear(right.cuts.back(),right.cuts.front()));
     D2<SBasis> line;
     line[X] = Linear(left.lastValue()[X],right.firstValue()[X]);
     line[Y] = Linear(left.lastValue()[Y],right.firstValue()[Y]);
     output = left;
     output.concat(Piecewise<D2<SBasis> >(line));
     output.concat(right);
     line[X] = Linear(right.lastValue()[X],left.firstValue()[X]);
     line[Y] = Linear(right.lastValue()[Y],left.firstValue()[Y]);
     output.concat(Piecewise<D2<SBasis> >(line));
     return output;
#else

     double angle_rad = angle*M_PI/180.;//TODO: revert orientation?...
     Piecewise<SBasis> w;
     
     // std::vector<double> corners = find_corners(m);
    
     DynastrokeMethod stroke_method = method.get_value();
     if (roundness==1.) {
//         std::cout<<"round pen.\n";
         n1 = n*double(width);
         n2 =-n1;
     }else{
         switch(stroke_method) {
             case DSM_ELLIPTIC_PEN:{
//                 std::cout<<"ellptic pen\n";
                 //FIXME: roundness=0???
                 double c = cos(angle_rad), s = sin(angle_rad); 
                 Affine rot,slant;
                 rot = Affine(c, -s, s,  c, 0, 0 );
                 slant = Affine(double(width)*roundness, 0, 0,  double(width), 0, 0 );
                 Piecewise<D2<SBasis> > nn = unitVector(v * ( rot * slant ) );
                 slant = Affine( 0,-roundness, 1, 0, 0, 0 );
                 rot = Affine(-s, -c, c, -s, 0, 0 );
                 nn = nn * (slant * rot );

                 n1 = nn*double(width);
                 n2 =-n1;
                 break;
             }    
             case DSM_THICKTHIN_FAST:{
//                 std::cout<<"fast thick thin pen\n";
                 D2<Piecewise<SBasis> > n_xy = make_cuts_independent(n);
                 w = n_xy[X]*sin(angle_rad) - n_xy[Y]*cos(angle_rad);
                 w = w * ((1 - roundness)*width/2.) + ((1 + roundness)*width/2.);
                 n1 = w*n;
                 n2 = -n1;
                 break;
             }
             case DSM_THICKTHIN_SLOW:{
//                 std::cout<<"slow thick thin pen\n";
                 D2<Piecewise<SBasis> > n_xy = make_cuts_independent(n);
                 w = n_xy[X]*cos(angle_rad)+ n_xy[Y]*sin(angle_rad);
                 w = w * ((1 - roundness)*width/2.) + ((1 + roundness)*width/2.);
                 //->Slower and less stable, but more accurate .
                 //  General  formula:  n1 = w*u with ||u||=1 and u.v = -dw/dt            
                 Piecewise<SBasis> dw = derivative(w);
                 Piecewise<SBasis> ncomp = sqrt(dot(v,v)-dw*dw,.1,3);
                 //FIXME: is force continuity usefull? compatible with corners?
//                 std::cout<<"ici\n";
                 n1 = -dw*v + ncomp*rot90(v);
                 n1 = w*force_continuity(unitVector(n1),.1);
                 n2 = -dw*v - ncomp*rot90(v);
                 n2 = w*force_continuity(unitVector(n2),.1);
//                 std::cout<<"ici2\n";
                 break;
             }
             default:{
                 n1 = n*double(width);
                 n2 = n1*(-.5);
                 break;
             }
         }//case
     }//if/else
     
     //
     //TODO: insert relevant stitch at each corner!!
     //

     Piecewise<D2<SBasis> > left, right;
     if ( m.segs.front().at0() == m.segs.back().at1()){
         // if closed:
//         std::cout<<"closed input.\n";
         left  = m + n1;//+ n;
         right = m + n2;//- n;
     } else {
         //if not closed, shape the ends:
         //TODO: allow fancy ends...
//         std::cout<<"shaping the ends\n";
         double grow_length = growfor;// * width;
         double fade_length = fadefor;// * width;
         Piecewise<SBasis > s = arcLengthSb(m);
         double totlength = s.segs.back().at1();
         
         //scale factor for a sharp start
         SBasis join = SBasis(2,Linear(0,1));
         join[1] = Linear(1,1);
         Piecewise<SBasis > factor_in = Piecewise<SBasis >(join);
         factor_in.cuts[1]=grow_length;
         if (grow_length < totlength){
             factor_in.concat(Piecewise<SBasis >(Linear(1)));
             factor_in.cuts[2]=totlength;
         }
//         std::cout<<"shaping the ends ici\n";
         //scale factor for a sharp end
         join[0] = Linear(1,0);
         join[1] = Linear(1,1);
         Piecewise<SBasis > factor_out;
         if (fade_length < totlength){
             factor_out = Piecewise<SBasis >(Linear(1));
             factor_out.cuts[1] = totlength-fade_length;
             factor_out.concat(Piecewise<SBasis >(join));
             factor_out.cuts[2] = totlength;
         }else{
             factor_out = Piecewise<SBasis >(join);
             factor_out.setDomain(Interval(totlength-fade_length,totlength));
         }
//         std::cout<<"shaping the ends ici ici\n";
         
         Piecewise<SBasis > factor = factor_in*factor_out;
         n1 = compose(factor,s)*n1;
         n2 = compose(factor,s)*n2;
         
         left  = m + n1;
         right = m + n2;
//         std::cout<<"shaping the ends ici ici ici\n";
         
         if (start_cap.get_value() == DSCT_ROUND){
//             std::cout<<"shaping round start\n";
             SBasis tau(2,Linear(0));
             tau[1] = Linear(-1,0);
             Piecewise<SBasis > hbump;
             hbump.concat(Piecewise<SBasis >(tau*grow_length));
             hbump.concat(Piecewise<SBasis >(Linear(0)));
             hbump.cuts[0]=0;
             hbump.cuts[1]=fmin(grow_length,totlength*grow_length/(grow_length+fade_length));
             hbump.cuts[2]=totlength;
             hbump = compose(hbump,s);
             
             left  += - hbump * rot90(n);
             right += - hbump * rot90(n);
         }
         if (end_cap.get_value() == DSCT_ROUND){
//             std::cout<<"shaping round end\n";
             SBasis tau(2,Linear(0));
             tau[1] = Linear(0,1);
             Piecewise<SBasis > hbump;
             hbump.concat(Piecewise<SBasis >(Linear(0)));
             hbump.concat(Piecewise<SBasis >(tau*fade_length));
             hbump.cuts[0]=0;
             hbump.cuts[1]=fmax(totlength-fade_length, totlength*grow_length/(grow_length+fade_length));
             hbump.cuts[2]=totlength;
             hbump = compose(hbump,s);
             
             left  += - hbump * rot90(n);
             right += - hbump * rot90(n);
         }
     }
     
     left = force_continuity(left);
     right = force_continuity(right);
          
//     std::cout<<"gathering result: left";
     output = left;
//     std::cout<<" + reverse(right)";
     output.concat(reverse(right));
//     std::cout<<". done\n";

//-----------
        return output;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
