/** \file
 * LPE <perspective-envelope> implementation
 
 */
/*
 * Authors:
 *   Jabiertxof Code migration from python extensions envelope and perspective
 *   Aaron Spike, aaron@ekips.org from envelope and perspective phyton code
 *   Dmitry Platonov, shadowjack@mail.ru, 2006 perspective approach & math
 *   Jose Hevia (freon) Transform algorithm from envelope
 *   
 * Copyright (C) 2007-2014 Authors 
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtkmm.h>
#include "live_effects/lpe-perspective-envelope.h"
#include "helper/geom.h"
#include "display/curve.h"
#include "svg/svg.h"
#include <gsl/gsl_linalg.h>


using namespace Geom;

namespace Inkscape {
namespace LivePathEffect {

enum DeformationType {
  DEFORMATION_PERSPECTIVE,
  DEFORMATION_ENVELOPE
};

static const Util::EnumData<unsigned> DeformationTypeData[] = {
    {DEFORMATION_PERSPECTIVE          , N_("Perspective"), "Perspective"},
    {DEFORMATION_ENVELOPE          , N_("Envelope deformation"), "Envelope deformation"}
};

static const Util::EnumDataConverter<unsigned> DeformationTypeConverter(DeformationTypeData, sizeof(DeformationTypeData)/sizeof(*DeformationTypeData));

LPEPerspectiveEnvelope::LPEPerspectiveEnvelope(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    // initialise your parameters here:
    deform_type(_("Type"), _("Select the type of deformation"), "deform_type", DeformationTypeConverter, &wr, this, DEFORMATION_PERSPECTIVE),
    Up_Left_Point(_("Top Left"), _("Top Left - <b>Ctrl+Alt+Click</b>: reset, <b>Ctrl</b>: move along axes"), "Up_Left_Point", &wr, this),
    Up_Right_Point(_("Top Right"), _("Top Right - <b>Ctrl+Alt+Click</b>: reset, <b>Ctrl</b>: move along axes"), "Up_Right_Point", &wr, this),
    Down_Left_Point(_("Down Left"), _("Down Left - <b>Ctrl+Alt+Click</b>: reset, <b>Ctrl</b>: move along axes"), "Down_Left_Point", &wr, this),
    Down_Right_Point(_("Down Right"), _("Down Right - <b>Ctrl+Alt+Click</b>: reset, <b>Ctrl</b>: move along axes"), "Down_Right_Point", &wr, this)
{
    // register all your parameters here, so Inkscape knows which parameters this effect has:
    registerParameter( dynamic_cast<Parameter *>(&deform_type));
    registerParameter( dynamic_cast<Parameter *>(&Up_Left_Point) );
    registerParameter( dynamic_cast<Parameter *>(&Up_Right_Point) );
    registerParameter( dynamic_cast<Parameter *>(&Down_Left_Point) );
    registerParameter( dynamic_cast<Parameter *>(&Down_Right_Point) );
}

LPEPerspectiveEnvelope::~LPEPerspectiveEnvelope()
{
}

void LPEPerspectiveEnvelope::doEffect(SPCurve *curve) {
    using Geom::X;
    using Geom::Y;
    double projmatrix[3][3];
    if(deform_type == DEFORMATION_PERSPECTIVE){
        std::vector<Geom::Point> handles(4);
        handles[0] = Down_Left_Point;
        handles[1] = Up_Left_Point;
        handles[2] = Up_Right_Point;
        handles[3] = Down_Right_Point;
        std::vector<Geom::Point> sourceHandles(4);
        sourceHandles[0] = Geom::Point(boundingbox_X.min(), boundingbox_Y.max());
        sourceHandles[1] = Geom::Point(boundingbox_X.min(), boundingbox_Y.min());
        sourceHandles[2] = Geom::Point(boundingbox_X.max(), boundingbox_Y.min());
        sourceHandles[3] = Geom::Point(boundingbox_X.max(), boundingbox_Y.max());
        double solmatrix[8][8] = {{0}};
        double free_term[8] = {0};
        double gslSolmatrix[64];
        for(unsigned int i = 0; i < 4; ++i){
            solmatrix[i][0] = sourceHandles[i][X];
            solmatrix[i][1] = sourceHandles[i][Y];
            solmatrix[i][2] = 1;
            solmatrix[i][6] = -handles[i][X] * sourceHandles[i][X];
            solmatrix[i][7] = -handles[i][X] * sourceHandles[i][Y];
            solmatrix[i+4][3] = sourceHandles[i][X];
            solmatrix[i+4][4] = sourceHandles[i][Y];
            solmatrix[i+4][5] = 1;
            solmatrix[i+4][6] = -handles[i][Y] * sourceHandles[i][X];
            solmatrix[i+4][7] = -handles[i][Y] * sourceHandles[i][Y];
            free_term[i] = handles[i][X];
            free_term[i+4] = handles[i][Y];
        }
        int h = 0;
        for( int i = 0; i < 8; i++ ) {
            for( int j = 0; j < 8; j++ ) {
                gslSolmatrix[h] = solmatrix[i][j];
                h++;
            }
        }
        //this is get by this page:
        //http://www.gnu.org/software/gsl/manual/html_node/Linear-Algebra-Examples.html#Linear-Algebra-Examples
        gsl_matrix_view m = gsl_matrix_view_array (gslSolmatrix, 8, 8);
        gsl_vector_view b = gsl_vector_view_array (free_term, 8);
        gsl_vector *x = gsl_vector_alloc (8);
        int s;
        gsl_permutation * p = gsl_permutation_alloc (8);
        gsl_linalg_LU_decomp (&m.matrix, p, &s);
        gsl_linalg_LU_solve (&m.matrix, p, &b.vector, x);
        h = 0;
        for( int i = 0; i < 3; i++ ) {
            for( int j = 0; j < 3; j++ ) {
                if(h==8){
                    projmatrix[2][2] = 1.0;
                    continue;
                }
                projmatrix[i][j] = gsl_vector_get(x, h);
                h++;
            }
        }
        gsl_permutation_free (p);
        gsl_vector_free (x);
    }
    Geom::PathVector const original_pathv = pathv_to_linear_and_cubic_beziers(curve->get_pathvector());
    curve->reset();
    Geom::CubicBezier const *cubic = NULL;
    Geom::Point pointAt1(0, 0);
    Geom::Point pointAt2(0, 0);
    Geom::Point pointAt3(0, 0);
    for (Geom::PathVector::const_iterator path_it = original_pathv.begin(); path_it != original_pathv.end(); ++path_it) {
        //Si está vacío...
        if (path_it->empty())
          continue;
        //Itreadores
        SPCurve *nCurve = new SPCurve();
        Geom::Path::const_iterator curve_it1 = path_it->begin();
        Geom::Path::const_iterator curve_it2 = ++(path_it->begin());
        Geom::Path::const_iterator curve_endit = path_it->end_default();
        
        if (path_it->closed()) {
            const Geom::Curve &closingline =
                path_it->back_closed();
            if (are_near(closingline.initialPoint(), closingline.finalPoint())) {
                curve_endit = path_it->end_open();
            }
        }
        if(deform_type == DEFORMATION_PERSPECTIVE){
            nCurve->moveto(project_point(curve_it1->initialPoint(),projmatrix));
        }else{
            nCurve->moveto(project_point(curve_it1->initialPoint()));
        }
        while (curve_it1 != curve_endit) {
          cubic = dynamic_cast<Geom::CubicBezier const *>(&*curve_it1);
          if (cubic) {
            pointAt1 = (*cubic)[1];
            pointAt2 = (*cubic)[2];
          } else {
            pointAt1 = curve_it1->initialPoint();
            pointAt2 = curve_it1->finalPoint();
          }
          pointAt3 = curve_it1->finalPoint();
          if(deform_type == DEFORMATION_PERSPECTIVE){
            pointAt1 = project_point(pointAt1,projmatrix);
            pointAt2 = project_point(pointAt2,projmatrix);
            pointAt3 = project_point(pointAt3,projmatrix);
          }else{
            pointAt1 = project_point(pointAt1);
            pointAt2 = project_point(pointAt2);
            pointAt3 = project_point(pointAt3);
          }
          nCurve->curveto(pointAt1, pointAt2, pointAt3);
          ++curve_it1;
          if(curve_it2 != curve_endit) {
            ++curve_it2;
          }
        }
        //y cerramos la curva
        if (path_it->closed()) {
          nCurve->move_endpoints(pointAt3, pointAt3);
          nCurve->closepath_current();
        }
        curve->append(nCurve, false);
        nCurve->reset();
        delete nCurve;
    }
}

Geom::Point 
LPEPerspectiveEnvelope::project_point(Geom::Point p){
    double width = boundingbox_X.extent();
    double height = boundingbox_Y.extent();
    double delta_x = boundingbox_X.min() - p[X];
    double delta_y = boundingbox_Y.max() - p[Y];
    Geom::Coord xratio = (delta_x * -1) / width;
    Geom::Coord yratio = delta_y / height;
    Geom::Line* horiz = new Geom::Line();
    Geom::Line* vert = new Geom::Line();
    vert->setPoints (pointAtRatio(yratio,Down_Left_Point,Up_Left_Point),pointAtRatio(yratio,Down_Right_Point,Up_Right_Point));
    horiz->setPoints (pointAtRatio(xratio,Down_Left_Point,Down_Right_Point),pointAtRatio(xratio,Up_Left_Point,Up_Right_Point));

    OptCrossing crossPoint = intersection(*horiz,*vert);
    if(crossPoint){
        return horiz->pointAt(Geom::Coord(crossPoint->ta));
    }else{
        return p;
    }
}

Geom::Point 
LPEPerspectiveEnvelope::project_point(Geom::Point p, double m[][3]){
    Geom::Coord x = p[0];
    Geom::Coord y = p[1];
    return Geom::Point(
                Geom::Coord((x*m[0][0] + y*m[0][1] + m[0][2])/(x*m[2][0]+y*m[2][1]+m[2][2])),
                Geom::Coord((x*m[1][0] + y*m[1][1] + m[1][2])/(x*m[2][0]+y*m[2][1]+m[2][2])));
}

Geom::Point
LPEPerspectiveEnvelope::pointAtRatio(Geom::Coord ratio,Geom::Point A, Geom::Point B){
        Geom::Coord x = A[X] + (ratio * (B[X]-A[X]));
        Geom::Coord y = A[Y]+ (ratio * (B[Y]-A[Y]));
        return Point(x, y);
}


Gtk::Widget *
LPEPerspectiveEnvelope::newWidget()
{
    // use manage here, because after deletion of Effect object, others might still be pointing to this widget.
    Gtk::VBox * vbox = Gtk::manage( new Gtk::VBox(Effect::newWidget()) );

    vbox->set_border_width(5);
    vbox->set_homogeneous(false);
    vbox->set_spacing(6);
    std::vector<Parameter *>::iterator it = param_vector.begin();
    Gtk::HBox * hboxUpHandles = Gtk::manage(new Gtk::HBox(false,0));
    Gtk::HBox * hboxDownHandles = Gtk::manage(new Gtk::HBox(false,0));
    while (it != param_vector.end()) {
        if ((*it)->widget_is_visible) {
            Parameter * param = *it;
            Gtk::Widget * widg = dynamic_cast<Gtk::Widget *>(param->param_newWidget());
            if (param->param_key == "Up_Left_Point" ||
                param->param_key == "Up_Right_Point" ||
                param->param_key == "Down_Left_Point" ||
                param->param_key == "Down_Right_Point")
            {
                Gtk::HBox * pointParameter = dynamic_cast<Gtk::HBox *>(widg);
                std::vector< Gtk::Widget* > childList = pointParameter->get_children();
                Gtk::HBox * pointParameterHBox = dynamic_cast<Gtk::HBox *>(childList[0]);
                std::vector< Gtk::Widget* > childList2 = pointParameterHBox->get_children();
                pointParameterHBox->remove(childList2[0][0]);
                Glib::ustring * tip = param->param_getTooltip();
                if (widg) {
                    if(param->param_key == "Up_Left_Point"){
                        Gtk::Label* handles = Gtk::manage(new Gtk::Label(Glib::ustring(_("Handles:")),Gtk::ALIGN_START));
                        vbox->pack_start(*handles, false, false, 2);
                        hboxUpHandles->pack_start(*widg, true, true, 2);
                        hboxUpHandles->pack_start(*Gtk::manage(new Gtk::VSeparator()), Gtk::PACK_EXPAND_WIDGET);
                    }else if(param->param_key == "Up_Right_Point"){
                        hboxUpHandles->pack_start(*widg, true, true, 2);
                    }else if(param->param_key == "Down_Left_Point"){
                        hboxDownHandles->pack_start(*widg, true, true, 2);
                        hboxDownHandles->pack_start(*Gtk::manage(new Gtk::VSeparator()), Gtk::PACK_EXPAND_WIDGET);
                    }else{
                        hboxDownHandles->pack_start(*widg, true, true, 2);
                    }
                    if (tip) {
                        widg->set_tooltip_text(*tip);
                    } else {
                        widg->set_tooltip_text("");
                        widg->set_has_tooltip(false);
                    }
                }
            }else{
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
        }

        ++it;
    }
    vbox->pack_start(*hboxUpHandles,true, true, 2);
    Gtk::HBox * hboxMiddle = Gtk::manage(new Gtk::HBox(true,2));
    hboxMiddle->pack_start(*Gtk::manage(new Gtk::HSeparator()), Gtk::PACK_EXPAND_WIDGET);
    hboxMiddle->pack_start(*Gtk::manage(new Gtk::HSeparator()), Gtk::PACK_EXPAND_WIDGET);
    vbox->pack_start(*hboxMiddle, false, true, 2);
    vbox->pack_start(*hboxDownHandles, true, true, 2);
    Gtk::HBox * hbox = Gtk::manage(new Gtk::HBox(false,0));
    Gtk::Button* resetButton = Gtk::manage(new Gtk::Button(Gtk::Stock::CLEAR));
    resetButton->signal_clicked().connect(sigc::mem_fun (*this,&LPEPerspectiveEnvelope::resetGrid));
    resetButton->set_size_request(140,30);
    vbox->pack_start(*hbox, true,true,2);
    hbox->pack_start(*resetButton, false, false,2);
    return dynamic_cast<Gtk::Widget *>(vbox);
}

void
LPEPerspectiveEnvelope::doBeforeEffect (SPLPEItem const* lpeitem)
{
    original_bbox(lpeitem);
    SPLPEItem * item = const_cast<SPLPEItem*>(lpeitem);
    item->apply_to_clippath(item);
    item->apply_to_mask(item);
    setDefaults();
}

void
LPEPerspectiveEnvelope::setDefaults()
{
    Geom::Point Up_Left(boundingbox_X.min(), boundingbox_Y.min());
    Geom::Point Up_Right(boundingbox_X.max(), boundingbox_Y.min());
    Geom::Point Down_Left(boundingbox_X.min(), boundingbox_Y.max());
    Geom::Point Down_Right(boundingbox_X.max(), boundingbox_Y.max());

    Up_Left_Point.param_update_default(Up_Left);
    Up_Right_Point.param_update_default(Up_Right);
    Down_Right_Point.param_update_default(Down_Right);
    Down_Left_Point.param_update_default(Down_Left);
}

void
LPEPerspectiveEnvelope::resetGrid()
{
    Up_Left_Point.param_set_and_write_default();
    Up_Right_Point.param_set_and_write_default();
    Down_Right_Point.param_set_and_write_default();
    Down_Left_Point.param_set_and_write_default();
}

void
LPEPerspectiveEnvelope::resetDefaults(SPItem const* item)
{
    Effect::resetDefaults(item);
    original_bbox(SP_LPE_ITEM(item));
    setDefaults();
    resetGrid();
}

void
LPEPerspectiveEnvelope::addCanvasIndicators(SPLPEItem const */*lpeitem*/, std::vector<Geom::PathVector> &hp_vec)
{
    hp_vec.clear();

    SPCurve *c = new SPCurve();
    c->reset();
    c->moveto(Up_Left_Point);
    c->lineto(Up_Right_Point);
    c->lineto(Down_Right_Point);
    c->lineto(Down_Left_Point);
    c->lineto(Up_Left_Point);
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
// vim: file_type=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
