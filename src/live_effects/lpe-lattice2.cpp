/** \file
 * LPE <lattice2> implementation
 
 */
/*
 * Authors:
 *   Johan Engelen <j.b.c.engelen@utwente.nl>
 *   Steren Giannini
 *   Noé Falzon
 *   Victor Navez
 *   ~suv
 *   Jabiertxo Arraiza
*
* Copyright (C) 2007-2008 Authors 
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-lattice2.h"

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

using namespace Geom;

namespace Inkscape {
namespace LivePathEffect {

LPELattice2::LPELattice2(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    horizontalMirror(_("Mirror movements in horizontal"), _("Mirror movements in horizontal"), "horizontalMirror", &wr, this, false),
    verticalMirror(_("Mirror movements in vertical"), _("Mirror movements in vertical"), "verticalMirror", &wr, this, false),
    grid_point0(_("Control handle 0:"), _("Control handle 0 - <b>Ctrl+Alt+Click</b>: reset, <b>Ctrl</b>: move along axes"), "gridpoint0", &wr, this),
    grid_point1(_("Control handle 1:"), _("Control handle 1 - <b>Ctrl+Alt+Click</b>: reset, <b>Ctrl</b>: move along axes"), "gridpoint1", &wr, this),
    grid_point2(_("Control handle 2:"), _("Control handle 2 - <b>Ctrl+Alt+Click</b>: reset, <b>Ctrl</b>: move along axes"), "gridpoint2", &wr, this),
    grid_point3(_("Control handle 3:"), _("Control handle 3 - <b>Ctrl+Alt+Click</b>: reset, <b>Ctrl</b>: move along axes"), "gridpoint3", &wr, this),
    grid_point4(_("Control handle 4:"), _("Control handle 4 - <b>Ctrl+Alt+Click</b>: reset, <b>Ctrl</b>: move along axes"), "gridpoint4", &wr, this),
    grid_point5(_("Control handle 5:"), _("Control handle 5 - <b>Ctrl+Alt+Click</b>: reset, <b>Ctrl</b>: move along axes"), "gridpoint5", &wr, this),
    grid_point6(_("Control handle 6:"), _("Control handle 6 - <b>Ctrl+Alt+Click</b>: reset, <b>Ctrl</b>: move along axes"), "gridpoint6", &wr, this),
    grid_point7(_("Control handle 7:"), _("Control handle 7 - <b>Ctrl+Alt+Click</b>: reset, <b>Ctrl</b>: move along axes"), "gridpoint7", &wr, this),
    grid_point8x9(_("Control handle 8x9:"), _("Control handle 8x9 - <b>Ctrl+Alt+Click</b>: reset, <b>Ctrl</b>: move along axes"), "gridpoint8x9", &wr, this),
    grid_point10x11(_("Control handle 10x11:"), _("Control handle 10x11 - <b>Ctrl+Alt+Click</b>: reset, <b>Ctrl</b>: move along axes"), "gridpoint10x11", &wr, this),
    grid_point12(_("Control handle 12:"), _("Control handle 12 - <b>Ctrl+Alt+Click</b>: reset, <b>Ctrl</b>: move along axes"), "gridpoint12", &wr, this),
    grid_point13(_("Control handle 13:"), _("Control handle 13 - <b>Ctrl+Alt+Click</b>: reset, <b>Ctrl</b>: move along axes"), "gridpoint13", &wr, this),
    grid_point14(_("Control handle 14:"), _("Control handle 14 - <b>Ctrl+Alt+Click</b>: reset, <b>Ctrl</b>: move along axes"), "gridpoint14", &wr, this),
    grid_point15(_("Control handle 15:"), _("Control handle 15 - <b>Ctrl+Alt+Click</b>: reset, <b>Ctrl</b>: move along axes"), "gridpoint15", &wr, this),
    grid_point16(_("Control handle 16:"), _("Control handle 16 - <b>Ctrl+Alt+Click</b>: reset, <b>Ctrl</b>: move along axes"), "gridpoint16", &wr, this),
    grid_point17(_("Control handle 17:"), _("Control handle 17 - <b>Ctrl+Alt+Click</b>: reset, <b>Ctrl</b>: move along axes"), "gridpoint17", &wr, this),
    grid_point18(_("Control handle 18:"), _("Control handle 18 - <b>Ctrl+Alt+Click</b>: reset, <b>Ctrl</b>: move along axes"), "gridpoint18", &wr, this),
    grid_point19(_("Control handle 19:"), _("Control handle 19 - <b>Ctrl+Alt+Click</b>: reset, <b>Ctrl</b>: move along axes"), "gridpoint19", &wr, this),
    grid_point20x21(_("Control handle 20x21:"), _("Control handle 20x21 - <b>Ctrl+Alt+Click</b>: reset, <b>Ctrl</b>: move along axes"), "gridpoint20x21", &wr, this),
    grid_point22x23(_("Control handle 22x23:"), _("Control handle 22x23 - <b>Ctrl+Alt+Click</b>: reset, <b>Ctrl</b>: move along axes"), "gridpoint22x23", &wr, this),
    grid_point24x26(_("Control handle 24x26:"), _("Control handle 24x26 - <b>Ctrl+Alt+Click</b>: reset, <b>Ctrl</b>: move along axes"), "gridpoint24x26", &wr, this),
    grid_point25x27(_("Control handle 25x27:"), _("Control handle 25x27 - <b>Ctrl+Alt+Click</b>: reset, <b>Ctrl</b>: move along axes"), "gridpoint25x27", &wr, this),
    grid_point28x30(_("Control handle 28x30:"), _("Control handle 28x30 - <b>Ctrl+Alt+Click</b>: reset, <b>Ctrl</b>: move along axes"), "gridpoint28x30", &wr, this),
    grid_point29x31(_("Control handle 29x31:"), _("Control handle 29x31 - <b>Ctrl+Alt+Click</b>: reset, <b>Ctrl</b>: move along axes"), "gridpoint29x31", &wr, this),
    grid_point32x33x34x35(_("Control handle 32x33x34x35:"), _("Control handle 32x33x34x35 - <b>Ctrl+Alt+Click</b>: reset, <b>Ctrl</b>: move along axes"), "gridpoint32x33x34x35", &wr, this)

    
{
    // register all your parameters here, so Inkscape knows which parameters this effect has:
    registerParameter(&horizontalMirror);
    registerParameter(&verticalMirror);
    registerParameter(&grid_point0);
    registerParameter(&grid_point1);
    registerParameter(&grid_point2);
    registerParameter(&grid_point3);
    registerParameter(&grid_point4);
    registerParameter(&grid_point5);
    registerParameter(&grid_point6);
    registerParameter(&grid_point7);
    registerParameter(&grid_point8x9);
    registerParameter(&grid_point10x11);
    registerParameter(&grid_point12);
    registerParameter(&grid_point13);
    registerParameter(&grid_point14);
    registerParameter(&grid_point15);
    registerParameter(&grid_point16);
    registerParameter(&grid_point17);
    registerParameter(&grid_point18);
    registerParameter(&grid_point19);
    registerParameter(&grid_point20x21);
    registerParameter(&grid_point22x23);
    registerParameter(&grid_point24x26);
    registerParameter(&grid_point25x27);
    registerParameter(&grid_point28x30);
    registerParameter(&grid_point29x31);
    registerParameter(&grid_point32x33x34x35);
}

LPELattice2::~LPELattice2()
{
}

Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPELattice2::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    D2<SBasis2d> sb2;
    
    //Initialisation of the sb2
    for(unsigned dim = 0; dim < 2; dim++) {
        sb2[dim].us = 3;
        sb2[dim].vs = 3;
        const int depth = sb2[dim].us*sb2[dim].vs;
        sb2[dim].resize(depth, Linear2d(0));
    }

    //Grouping the point params in a convenient vector
    std::vector<Geom::Point *> handles(36);
    
    handles[0] = &grid_point0;
    handles[1] = &grid_point1;
    handles[2] = &grid_point2;
    handles[3] = &grid_point3;
    handles[4] = &grid_point4;
    handles[5] = &grid_point5;
    handles[6] = &grid_point6;
    handles[7] = &grid_point7;
    handles[8] = &grid_point8x9;
    handles[9] = &grid_point8x9;
    handles[10] = &grid_point10x11;
    handles[11] = &grid_point10x11;
    handles[12] = &grid_point12;
    handles[13] = &grid_point13;
    handles[14] = &grid_point14;
    handles[15] = &grid_point15;
    handles[16] = &grid_point16;
    handles[17] = &grid_point17;
    handles[18] = &grid_point18;
    handles[19] = &grid_point19;
    handles[20] = &grid_point20x21;
    handles[21] = &grid_point20x21;
    handles[22] = &grid_point22x23;
    handles[23] = &grid_point22x23;
    handles[24] = &grid_point24x26;
    handles[25] = &grid_point25x27;
    handles[26] = &grid_point24x26;
    handles[27] = &grid_point25x27;
    handles[28] = &grid_point28x30;
    handles[29] = &grid_point29x31;
    handles[30] = &grid_point28x30;
    handles[31] = &grid_point29x31;
    handles[32] = &grid_point32x33x34x35;
    handles[33] = &grid_point32x33x34x35;
    handles[34] = &grid_point32x33x34x35;
    handles[35] = &grid_point32x33x34x35;

    Geom::Point origin = Geom::Point(boundingbox_X.min(),boundingbox_Y.min());
      
    double width = boundingbox_X.extent();
    double height = boundingbox_Y.extent();

    //numbering is based on 4 rectangles.16
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
                        Geom::Point base(   (ui + iu*(4-2*ui))*width/4.,
                                            (vi + iv*(4-2*vi))*height/4.);
                        
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
        B[Geom::X] -= origin[Geom::X];
        B[Geom::X]*= 1/width;
        B[Geom::Y] -= origin[Geom::Y];
        B[Geom::Y]*= 1/height;
        //Here comes the magic
        D2<SBasis> tB = compose_each(sb2,B);
        tB[Geom::X] = tB[Geom::X] * width + origin[Geom::X];
        tB[Geom::Y] = tB[Geom::Y] * height + origin[Geom::Y];

        output.push(tB,i+1);
    }
    return output;
}

Gtk::Widget *
LPELattice2::newWidget()
{
    // use manage here, because after deletion of Effect object, others might still be pointing to this widget.
    Gtk::VBox * vbox = Gtk::manage( new Gtk::VBox(Effect::newWidget()) );

    vbox->set_border_width(5);
    vbox->set_homogeneous(false);
    vbox->set_spacing(6);
    Gtk::HBox * hbox = Gtk::manage(new Gtk::HBox(false,0));
    Gtk::Button* resetButton = Gtk::manage(new Gtk::Button(Glib::ustring(_("Reset grid"))));
    resetButton->signal_clicked().connect(sigc::mem_fun (*this,&LPELattice2::resetGrid));
    resetButton->set_size_request(140,30);
    vbox->pack_start(*hbox, true,true,2);
    hbox->pack_start(*resetButton, false, false,2);
    std::vector<Parameter *>::iterator it = param_vector.begin();
    while (it != param_vector.end()) {
        if ((*it)->widget_is_visible) {
            Parameter * param = *it;
            Gtk::Widget * widg = dynamic_cast<Gtk::Widget *>(param->param_newWidget());
            if(param->param_key == "grid"){
                widg = NULL;
            }
            Glib::ustring * tip = param->param_getTooltip();
            if (widg) {
                vbox->pack_start(*widg, true, true, 2);
                if (tip) {
                    widg->set_tooltip_text(*tip);
                } else {
                    widg->set_tooltip_text("");
                    widg->set_has_tooltip(false);
                }
            }
        }

        ++it;
    }
    return dynamic_cast<Gtk::Widget *>(vbox);
}

void
LPELattice2::vertical(PointParam &paramA, PointParam &paramB, Geom::Line vert){
    Geom::Point A = paramA;
    Geom::Point B = paramB;
    double Y = (A[Geom::Y] + B[Geom::Y])/2;
    A[Geom::Y] = Y;
    B[Geom::Y] = Y;
    Geom::Point nearest = vert.pointAt(vert.nearestPoint(A));
    double distA = Geom::distance(A,nearest);
    double distB = Geom::distance(B,nearest);
    double distanceMed = (distA + distB)/2;
    A[Geom::X] = nearest[Geom::X] - distanceMed;
    B[Geom::X] = nearest[Geom::X] + distanceMed;
    paramA.param_set_and_write_new_value(A);
    paramB.param_set_and_write_new_value(B);
}

void
LPELattice2::horizontal(PointParam &paramA, PointParam &paramB, Geom::Line horiz){
    Geom::Point A = paramA;
    Geom::Point B = paramB;
    double X = (A[Geom::X] + B[Geom::X])/2;
    A[Geom::X] = X;
    B[Geom::X] = X;
    Geom::Point nearest = horiz.pointAt(horiz.nearestPoint(A));
    double distA = Geom::distance(A,nearest);
    double distB = Geom::distance(B,nearest);
    double distanceMed = (distA + distB)/2;
    A[Geom::Y] = nearest[Geom::Y] - distanceMed;
    B[Geom::Y] = nearest[Geom::Y] + distanceMed;
    paramA.param_set_and_write_new_value(A);
    paramB.param_set_and_write_new_value(B);
}

void
LPELattice2::doBeforeEffect (SPLPEItem const* lpeitem)
{
    original_bbox(lpeitem);
    Geom::Line vert(grid_point8x9,grid_point10x11);
    Geom::Line horiz(grid_point24x26,grid_point25x27);
    if(verticalMirror){
        vertical(grid_point0, grid_point1,vert);
        vertical(grid_point2, grid_point3,vert);
        vertical(grid_point4, grid_point5,vert);
        vertical(grid_point6, grid_point7,vert);
        vertical(grid_point12, grid_point13,vert);
        vertical(grid_point14, grid_point15,vert);
        vertical(grid_point16, grid_point17,vert);
        vertical(grid_point18, grid_point19,vert);
        vertical(grid_point24x26, grid_point25x27,vert);
        vertical(grid_point28x30, grid_point29x31,vert);
    }
    if(horizontalMirror){
        horizontal(grid_point0, grid_point2,horiz);
        horizontal(grid_point1, grid_point3,horiz);
        horizontal(grid_point4, grid_point6,horiz);
        horizontal(grid_point5, grid_point7,horiz);
        horizontal(grid_point8x9, grid_point10x11,horiz);
        horizontal(grid_point12, grid_point14,horiz);
        horizontal(grid_point13, grid_point15,horiz);
        horizontal(grid_point16, grid_point18,horiz);
        horizontal(grid_point17, grid_point19,horiz);
        horizontal(grid_point20x21, grid_point22x23,horiz);
    }
    setDefaults();
    SPLPEItem * item = const_cast<SPLPEItem*>(lpeitem);
    item->apply_to_clippath(item);
    item->apply_to_mask(item);
}

void
LPELattice2::setDefaults()
{
    Geom::Point gp0((boundingbox_X.max()-boundingbox_X.min())/4*0+boundingbox_X.min(),
                    (boundingbox_Y.max()-boundingbox_Y.min())/4*0+boundingbox_Y.min());

    Geom::Point gp1((boundingbox_X.max()-boundingbox_X.min())/4*4+boundingbox_X.min(),
                    (boundingbox_Y.max()-boundingbox_Y.min())/4*0+boundingbox_Y.min());

    Geom::Point gp2((boundingbox_X.max()-boundingbox_X.min())/4*0+boundingbox_X.min(),
                    (boundingbox_Y.max()-boundingbox_Y.min())/4*4+boundingbox_Y.min());

    Geom::Point gp3((boundingbox_X.max()-boundingbox_X.min())/4*4+boundingbox_X.min(),
                    (boundingbox_Y.max()-boundingbox_Y.min())/4*4+boundingbox_Y.min());

    Geom::Point gp4((boundingbox_X.max()-boundingbox_X.min())/4*1+boundingbox_X.min(),
                    (boundingbox_Y.max()-boundingbox_Y.min())/4*0+boundingbox_Y.min());

    Geom::Point gp5((boundingbox_X.max()-boundingbox_X.min())/4*3+boundingbox_X.min(),
                    (boundingbox_Y.max()-boundingbox_Y.min())/4*0+boundingbox_Y.min());

    Geom::Point gp6((boundingbox_X.max()-boundingbox_X.min())/4*1+boundingbox_X.min(),
                    (boundingbox_Y.max()-boundingbox_Y.min())/4*4+boundingbox_Y.min());

    Geom::Point gp7((boundingbox_X.max()-boundingbox_X.min())/4*3+boundingbox_X.min(),
                    (boundingbox_Y.max()-boundingbox_Y.min())/4*4+boundingbox_Y.min());

    Geom::Point gp8x9((boundingbox_X.max()-boundingbox_X.min())/4*2+boundingbox_X.min(),
                    (boundingbox_Y.max()-boundingbox_Y.min())/4*0+boundingbox_Y.min());

    Geom::Point gp10x11((boundingbox_X.max()-boundingbox_X.min())/4*2+boundingbox_X.min(),
                    (boundingbox_Y.max()-boundingbox_Y.min())/4*4+boundingbox_Y.min());

    Geom::Point gp12((boundingbox_X.max()-boundingbox_X.min())/4*0+boundingbox_X.min(),
                    (boundingbox_Y.max()-boundingbox_Y.min())/4*1+boundingbox_Y.min());

    Geom::Point gp13((boundingbox_X.max()-boundingbox_X.min())/4*4+boundingbox_X.min(),
                    (boundingbox_Y.max()-boundingbox_Y.min())/4*1+boundingbox_Y.min());

    Geom::Point gp14((boundingbox_X.max()-boundingbox_X.min())/4*0+boundingbox_X.min(),
                    (boundingbox_Y.max()-boundingbox_Y.min())/4*3+boundingbox_Y.min());

    Geom::Point gp15((boundingbox_X.max()-boundingbox_X.min())/4*4+boundingbox_X.min(),
                    (boundingbox_Y.max()-boundingbox_Y.min())/4*3+boundingbox_Y.min());

    Geom::Point gp16((boundingbox_X.max()-boundingbox_X.min())/4*1+boundingbox_X.min(),
                    (boundingbox_Y.max()-boundingbox_Y.min())/4*1+boundingbox_Y.min());

    Geom::Point gp17((boundingbox_X.max()-boundingbox_X.min())/4*3+boundingbox_X.min(),
                    (boundingbox_Y.max()-boundingbox_Y.min())/4*1+boundingbox_Y.min());

    Geom::Point gp18((boundingbox_X.max()-boundingbox_X.min())/4*1+boundingbox_X.min(),
                    (boundingbox_Y.max()-boundingbox_Y.min())/4*3+boundingbox_Y.min());

    Geom::Point gp19((boundingbox_X.max()-boundingbox_X.min())/4*3+boundingbox_X.min(),
                    (boundingbox_Y.max()-boundingbox_Y.min())/4*3+boundingbox_Y.min());

    Geom::Point gp20x21((boundingbox_X.max()-boundingbox_X.min())/4*2+boundingbox_X.min(),
                    (boundingbox_Y.max()-boundingbox_Y.min())/4*1+boundingbox_Y.min());

    Geom::Point gp22x23((boundingbox_X.max()-boundingbox_X.min())/4*2+boundingbox_X.min(),
                    (boundingbox_Y.max()-boundingbox_Y.min())/4*3+boundingbox_Y.min());

    Geom::Point gp24x26((boundingbox_X.max()-boundingbox_X.min())/4*0+boundingbox_X.min(),
                    (boundingbox_Y.max()-boundingbox_Y.min())/4*2+boundingbox_Y.min());

    Geom::Point gp25x27((boundingbox_X.max()-boundingbox_X.min())/4*4+boundingbox_X.min(),
                    (boundingbox_Y.max()-boundingbox_Y.min())/4*2+boundingbox_Y.min());

    Geom::Point gp28x30((boundingbox_X.max()-boundingbox_X.min())/4*1+boundingbox_X.min(),
                    (boundingbox_Y.max()-boundingbox_Y.min())/4*2+boundingbox_Y.min());

    Geom::Point gp29x31((boundingbox_X.max()-boundingbox_X.min())/4*3+boundingbox_X.min(),
                    (boundingbox_Y.max()-boundingbox_Y.min())/4*2+boundingbox_Y.min());

    Geom::Point gp32x33x34x35((boundingbox_X.max()-boundingbox_X.min())/4*2+boundingbox_X.min(),
                    (boundingbox_Y.max()-boundingbox_Y.min())/4*2+boundingbox_Y.min());

    grid_point0.param_update_default(gp0);
    grid_point1.param_update_default(gp1);
    grid_point2.param_update_default(gp2);
    grid_point3.param_update_default(gp3);
    grid_point4.param_update_default(gp4);
    grid_point5.param_update_default(gp5);
    grid_point6.param_update_default(gp6);
    grid_point7.param_update_default(gp7);
    grid_point8x9.param_update_default(gp8x9);
    grid_point10x11.param_update_default(gp10x11);
    grid_point12.param_update_default(gp12);
    grid_point13.param_update_default(gp13);
    grid_point14.param_update_default(gp14);
    grid_point15.param_update_default(gp15);
    grid_point16.param_update_default(gp16);
    grid_point17.param_update_default(gp17);
    grid_point18.param_update_default(gp18);
    grid_point19.param_update_default(gp19);
    grid_point20x21.param_update_default(gp20x21);
    grid_point22x23.param_update_default(gp22x23);
    grid_point24x26.param_update_default(gp24x26);
    grid_point25x27.param_update_default(gp25x27);
    grid_point28x30.param_update_default(gp28x30);
    grid_point29x31.param_update_default(gp29x31);
    grid_point32x33x34x35.param_update_default(gp32x33x34x35);
}

void
LPELattice2::resetGrid()
{
    grid_point0.param_set_and_write_default();
    grid_point1.param_set_and_write_default();
    grid_point2.param_set_and_write_default();
    grid_point3.param_set_and_write_default();
    grid_point4.param_set_and_write_default();
    grid_point5.param_set_and_write_default();
    grid_point6.param_set_and_write_default();
    grid_point7.param_set_and_write_default();
    grid_point8x9.param_set_and_write_default();
    grid_point10x11.param_set_and_write_default();
    grid_point12.param_set_and_write_default();
    grid_point13.param_set_and_write_default();
    grid_point14.param_set_and_write_default();
    grid_point15.param_set_and_write_default();
    grid_point16.param_set_and_write_default();
    grid_point17.param_set_and_write_default();
    grid_point18.param_set_and_write_default();
    grid_point19.param_set_and_write_default();
    grid_point20x21.param_set_and_write_default();
    grid_point22x23.param_set_and_write_default();
    grid_point24x26.param_set_and_write_default();
    grid_point25x27.param_set_and_write_default();
    grid_point28x30.param_set_and_write_default();
    grid_point29x31.param_set_and_write_default();
    grid_point32x33x34x35.param_set_and_write_default();
    if(sp_lpe_item){
        sp_lpe_item_update_patheffect(sp_lpe_item, false, false);
    }
}

void
LPELattice2::resetDefaults(SPItem const* item)
{
    Effect::resetDefaults(item);
    original_bbox(SP_LPE_ITEM(item));
    setDefaults();
    resetGrid();
}

void
LPELattice2::calculateCurve(Geom::Point a,Geom::Point b, SPCurve* c, bool horizontal, bool move)
{
    using Geom::X;
    using Geom::Y;
    if(move) c->moveto(a);
    Geom::Point cubic1 = a + (1./3)* (b - a);
    Geom::Point cubic2 = b + (1./3)* (a - b);
    if(horizontal) c->curveto(Geom::Point(cubic1[X],a[Y]),Geom::Point(cubic2[X],b[Y]),b);
    else c->curveto(Geom::Point(a[X],cubic1[Y]),Geom::Point(b[X],cubic2[Y]),b);
}

void
LPELattice2::addCanvasIndicators(SPLPEItem const */*lpeitem*/, std::vector<Geom::PathVector> &hp_vec)
{
    hp_vec.clear();

    SPCurve *c = new SPCurve();
    calculateCurve(grid_point0,grid_point4, c,true, true);
    calculateCurve(grid_point4,grid_point8x9, c,true, false);
    calculateCurve(grid_point8x9,grid_point5, c,true, false);
    calculateCurve(grid_point5,grid_point1, c,true, false);

    calculateCurve(grid_point12,grid_point16, c,true, true);
    calculateCurve(grid_point16,grid_point20x21, c,true, false);
    calculateCurve(grid_point20x21,grid_point17, c,true, false);
    calculateCurve(grid_point17,grid_point13, c,true, false);

    calculateCurve(grid_point24x26,grid_point28x30, c,true, true);
    calculateCurve(grid_point28x30,grid_point32x33x34x35, c,true, false);
    calculateCurve(grid_point32x33x34x35,grid_point29x31, c,true, false);
    calculateCurve(grid_point29x31,grid_point25x27, c,true, false);

    calculateCurve(grid_point14,grid_point18, c,true, true);
    calculateCurve(grid_point18,grid_point22x23, c,true, false);
    calculateCurve(grid_point22x23,grid_point19, c,true, false);
    calculateCurve(grid_point19,grid_point15, c,true, false);

    calculateCurve(grid_point2,grid_point6, c,true, true);
    calculateCurve(grid_point6,grid_point10x11, c,true, false);
    calculateCurve(grid_point10x11,grid_point7, c,true, false);
    calculateCurve(grid_point7,grid_point3, c,true, false);

    calculateCurve(grid_point0,grid_point12, c,false, true);
    calculateCurve(grid_point12,grid_point24x26, c,false, false);
    calculateCurve(grid_point24x26,grid_point14, c,false, false);
    calculateCurve(grid_point14,grid_point2, c,false, false);

    calculateCurve(grid_point4,grid_point16, c,false, true);
    calculateCurve(grid_point16,grid_point28x30, c,false, false);
    calculateCurve(grid_point28x30,grid_point18, c,false, false);
    calculateCurve(grid_point18,grid_point6, c,false, false);

    calculateCurve(grid_point8x9,grid_point20x21, c,false, true);
    calculateCurve(grid_point20x21,grid_point32x33x34x35, c,false, false);
    calculateCurve(grid_point32x33x34x35,grid_point22x23, c,false, false);
    calculateCurve(grid_point22x23,grid_point10x11, c,false, false);

    calculateCurve(grid_point5,grid_point17, c, false, true);
    calculateCurve(grid_point17,grid_point29x31, c,false, false);
    calculateCurve(grid_point29x31,grid_point19, c,false, false);
    calculateCurve(grid_point19,grid_point7, c,false, false);

    calculateCurve(grid_point1,grid_point13, c, false, true);
    calculateCurve(grid_point13,grid_point25x27, c,false, false);
    calculateCurve(grid_point25x27,grid_point15, c,false, false);
    calculateCurve(grid_point15,grid_point3, c, false, false);
    hp_vec.push_back(c->get_pathvector());
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
