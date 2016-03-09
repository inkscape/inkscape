/** \file
 * LPE <lattice> implementation
 
 */
/*
 * Authors:
 *   Johan Engelen <j.b.c.engelen@utwente.nl>
 *   Steren Giannini
 *   Noé Falzon
 *   Victor Navez
*
* Copyright (C) 2007-2008 Authors 
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-lattice.h"

#include "sp-shape.h"
#include "sp-item.h"
#include "sp-path.h"
#include "display/curve.h"
#include "svg/svg.h"

#include <2geom/sbasis.h>
#include <2geom/sbasis-2d.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/d2.h>
#include <2geom/piecewise.h>
#include <2geom/transforms.h>

#include "desktop.h" // TODO: should be factored out (see below)

using namespace Geom;

namespace Inkscape {
namespace LivePathEffect {

LPELattice::LPELattice(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    
    // initialise your parameters here:
    grid_point0(_("Control handle 0:"), _("Control handle 0"), "gridpoint0", &wr, this),
    grid_point1(_("Control handle 1:"), _("Control handle 1"), "gridpoint1", &wr, this),
    grid_point2(_("Control handle 2:"), _("Control handle 2"), "gridpoint2", &wr, this),
    grid_point3(_("Control handle 3:"), _("Control handle 3"), "gridpoint3", &wr, this),
    grid_point4(_("Control handle 4:"), _("Control handle 4"), "gridpoint4", &wr, this),
    grid_point5(_("Control handle 5:"), _("Control handle 5"), "gridpoint5", &wr, this),
    grid_point6(_("Control handle 6:"), _("Control handle 6"), "gridpoint6", &wr, this),
    grid_point7(_("Control handle 7:"), _("Control handle 7"), "gridpoint7", &wr, this),
    grid_point8(_("Control handle 8:"), _("Control handle 8"), "gridpoint8", &wr, this),
    grid_point9(_("Control handle 9:"), _("Control handle 9"), "gridpoint9", &wr, this),
    grid_point10(_("Control handle 10:"), _("Control handle 10"), "gridpoint10", &wr, this),
    grid_point11(_("Control handle 11:"), _("Control handle 11"), "gridpoint11", &wr, this),
    grid_point12(_("Control handle 12:"), _("Control handle 12"), "gridpoint12", &wr, this),
    grid_point13(_("Control handle 13:"), _("Control handle 13"), "gridpoint13", &wr, this),
    grid_point14(_("Control handle 14:"), _("Control handle 14"), "gridpoint14", &wr, this),
    grid_point15(_("Control handle 15:"), _("Control handle 15"), "gridpoint15", &wr, this)
    
{
    // register all your parameters here, so Inkscape knows which parameters this effect has:
    registerParameter( dynamic_cast<Parameter *>(&grid_point0) );
    registerParameter( dynamic_cast<Parameter *>(&grid_point1) );
    registerParameter( dynamic_cast<Parameter *>(&grid_point2) );
    registerParameter( dynamic_cast<Parameter *>(&grid_point3) );
    registerParameter( dynamic_cast<Parameter *>(&grid_point4) );
    registerParameter( dynamic_cast<Parameter *>(&grid_point5) );
    registerParameter( dynamic_cast<Parameter *>(&grid_point6) );
    registerParameter( dynamic_cast<Parameter *>(&grid_point7) );
    registerParameter( dynamic_cast<Parameter *>(&grid_point8) );
    registerParameter( dynamic_cast<Parameter *>(&grid_point9) );
    registerParameter( dynamic_cast<Parameter *>(&grid_point10) );
    registerParameter( dynamic_cast<Parameter *>(&grid_point11) );
    registerParameter( dynamic_cast<Parameter *>(&grid_point12) );
    registerParameter( dynamic_cast<Parameter *>(&grid_point13) );
    registerParameter( dynamic_cast<Parameter *>(&grid_point14) );
    registerParameter( dynamic_cast<Parameter *>(&grid_point15) );

    apply_to_clippath_and_mask = true;
}

LPELattice::~LPELattice()
{

}


Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPELattice::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    D2<SBasis2d> sb2;
    
    //Initialisation of the sb2
    for(unsigned dim = 0; dim < 2; dim++) {
        sb2[dim].us = 2;
        sb2[dim].vs = 2;
        const int depth = sb2[dim].us*sb2[dim].vs;
        sb2[dim].resize(depth, Linear2d(0));
    }

    //Grouping the point params in a convenient vector
    std::vector<Geom::Point *> handles(16);
    
    handles[0] = &grid_point0;
    handles[1] = &grid_point1;
    handles[2] = &grid_point2;
    handles[3] = &grid_point3;
    handles[4] = &grid_point4;
    handles[5] = &grid_point5;
    handles[6] = &grid_point6;
    handles[7] = &grid_point7;
    handles[8] = &grid_point8;
    handles[9] = &grid_point9;
    handles[10] = &grid_point10;
    handles[11] = &grid_point11;
    handles[12] = &grid_point12;
    handles[13] = &grid_point13;
    handles[14] = &grid_point14;
    handles[15] = &grid_point15;

    Geom::Point origin = Geom::Point(boundingbox_X.min(),boundingbox_Y.min());
      
    double width = boundingbox_X.extent();
    double height = boundingbox_Y.extent();

    //numbering is based on 4 rectangles.
    for(unsigned dim = 0; dim < 2; dim++) {
        Geom::Point dir(0,0);
        dir[dim] = 1;
        for(unsigned vi = 0; vi < sb2[dim].vs; vi++) {
            for(unsigned ui = 0; ui < sb2[dim].us; ui++) {
                for(unsigned iv = 0; iv < 2; iv++) {
                    for(unsigned iu = 0; iu < 2; iu++) {
                        unsigned corner = iu + 2*iv;
                        unsigned i = ui + vi*sb2[dim].us;
                        
                        //This is the offset from the Upperleft point
                        Geom::Point base(   (ui + iu*(3-2*ui))*width/3.,
                                            (vi + iv*(3-2*vi))*height/3.);
                       
                        //Special action for corners
                        if(vi == 0 && ui == 0) {
                            base = Geom::Point(0,0);
                        }
                        
                        // i = Upperleft corner of the considerated rectangle
                        // corner = actual corner of the rectangle
                        // origin = Upperleft point
                        double dl = dot((*handles[corner+4*i] - (base + origin)), dir)/dot(dir,dir);
                        sb2[dim][i][corner] = dl/( dim ? height : width )*pow(4.0,ui+vi);
                    }
                }
            }
        }
    }
   
    Piecewise<D2<SBasis> >  output;
    output.push_cut(0.);
    for(unsigned i = 0; i < pwd2_in.size(); i++) {
        D2<SBasis> B = pwd2_in[i];
        B -= origin;   
        B*= 1/width;
        //Here comes the magic
        D2<SBasis> tB = compose_each(sb2,B);
        tB = tB * width + origin;

        output.push(tB,i+1);
    }

    return output;
}

void
LPELattice::doBeforeEffect (SPLPEItem const* lpeitem)
{
    original_bbox(lpeitem);
}

void
LPELattice::resetDefaults(SPItem const* item)
{
    Effect::resetDefaults(item);

    original_bbox(SP_LPE_ITEM(item), false);
    
    // place the 16 control points
    grid_point0[Geom::X] = boundingbox_X.min();
    grid_point0[Geom::Y] = boundingbox_Y.min();
    
    grid_point1[Geom::X] = boundingbox_X.max();
    grid_point1[Geom::Y] = boundingbox_Y.min();
    
    grid_point2[Geom::X] = boundingbox_X.min();
    grid_point2[Geom::Y] = boundingbox_Y.max();
    
    grid_point3[Geom::X] = boundingbox_X.max();
    grid_point3[Geom::Y] = boundingbox_Y.max();
    
    grid_point4[Geom::X] = 1.0/3*boundingbox_X.max()+2.0/3*boundingbox_X.min();
    grid_point4[Geom::Y] = boundingbox_Y.min();
    
    grid_point5[Geom::X] = 2.0/3*boundingbox_X.max()+1.0/3*boundingbox_X.min();
    grid_point5[Geom::Y] = boundingbox_Y.min();
    
    grid_point6[Geom::X] = 1.0/3*boundingbox_X.max()+2.0/3*boundingbox_X.min();
    grid_point6[Geom::Y] = boundingbox_Y.max();
    
    grid_point7[Geom::X] = 2.0/3*boundingbox_X.max()+1.0/3*boundingbox_X.min();
    grid_point7[Geom::Y] = boundingbox_Y.max();
    
    grid_point8[Geom::X] = boundingbox_X.min();
    grid_point8[Geom::Y] = 1.0/3*boundingbox_Y.max()+2.0/3*boundingbox_Y.min();     

    grid_point9[Geom::X] = boundingbox_X.max();
    grid_point9[Geom::Y] = 1.0/3*boundingbox_Y.max()+2.0/3*boundingbox_Y.min();
        
    grid_point10[Geom::X] = boundingbox_X.min();
    grid_point10[Geom::Y] = 2.0/3*boundingbox_Y.max()+1.0/3*boundingbox_Y.min();  
    
    grid_point11[Geom::X] = boundingbox_X.max();
    grid_point11[Geom::Y] = 2.0/3*boundingbox_Y.max()+1.0/3*boundingbox_Y.min();  
    
    grid_point12[Geom::X] = 1.0/3*boundingbox_X.max()+2.0/3*boundingbox_X.min();
    grid_point12[Geom::Y] = 1.0/3*boundingbox_Y.max()+2.0/3*boundingbox_Y.min();
    
    grid_point13[Geom::X] = 2.0/3*boundingbox_X.max()+1.0/3*boundingbox_X.min();
    grid_point13[Geom::Y] = 1.0/3*boundingbox_Y.max()+2.0/3*boundingbox_Y.min();
    
    grid_point14[Geom::X] = 1.0/3*boundingbox_X.max()+2.0/3*boundingbox_X.min();
    grid_point14[Geom::Y] = 2.0/3*boundingbox_Y.max()+1.0/3*boundingbox_Y.min();
    
    grid_point15[Geom::X] = 2.0/3*boundingbox_X.max()+1.0/3*boundingbox_X.min();
    grid_point15[Geom::Y] = 2.0/3*boundingbox_Y.max()+1.0/3*boundingbox_Y.min();
    grid_point1.param_update_default(grid_point1);
    grid_point2.param_update_default(grid_point2);
    grid_point3.param_update_default(grid_point3);
    grid_point4.param_update_default(grid_point4);
    grid_point5.param_update_default(grid_point5);
    grid_point6.param_update_default(grid_point6);
    grid_point7.param_update_default(grid_point7);
    grid_point8.param_update_default(grid_point8);
    grid_point9.param_update_default(grid_point9);
    grid_point10.param_update_default(grid_point10);
    grid_point11.param_update_default(grid_point11);
    grid_point12.param_update_default(grid_point12);
    grid_point13.param_update_default(grid_point13);
    grid_point14.param_update_default(grid_point14);
    grid_point15.param_update_default(grid_point15);
}

/**
void
LPELattice::addHelperPathsImpl(SPLPEItem *lpeitem, SPDesktop *desktop)
{
    SPCurve *c = new SPCurve ();
    c->moveto(grid_point0);
    c->lineto(grid_point4);
    c->lineto(grid_point5);
    c->lineto(grid_point1);

    c->moveto(grid_point8);
    c->lineto(grid_point12);
    c->lineto(grid_point13);
    c->lineto(grid_point9);

    c->moveto(grid_point10);
    c->lineto(grid_point14);
    c->lineto(grid_point15);
    c->lineto(grid_point11);

    c->moveto(grid_point2);
    c->lineto(grid_point6);
    c->lineto(grid_point7);
    c->lineto(grid_point3);


    c->moveto(grid_point0);
    c->lineto(grid_point8);
    c->lineto(grid_point10);
    c->lineto(grid_point2);

    c->moveto(grid_point4);
    c->lineto(grid_point12);
    c->lineto(grid_point14);
    c->lineto(grid_point6);

    c->moveto(grid_point5);
    c->lineto(grid_point13);
    c->lineto(grid_point15);
    c->lineto(grid_point7);

    c->moveto(grid_point1);
    c->lineto(grid_point9);
    c->lineto(grid_point11);
    c->lineto(grid_point3);

    // TODO: factor this out (and remove the #include of desktop.h above)
    SPCanvasItem *canvasitem = sp_nodepath_generate_helperpath(desktop, c, lpeitem, 0x009000ff);
    Inkscape::Display::TemporaryItem* tmpitem = desktop->add_temporary_canvasitem (canvasitem, 0);
    lpeitem->lpe_helperpaths.push_back(tmpitem);

    c->unref();
}
**/

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
