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
    horizontal_mirror(_("Mirror movements in horizontal"), _("Mirror movements in horizontal"), "horizontal_mirror", &wr, this, false),
    vertical_mirror(_("Mirror movements in vertical"), _("Mirror movements in vertical"), "vertical_mirror", &wr, this, false),
    deform_type(_("Type"), _("Select the type of deformation"), "deform_type", DeformationTypeConverter, &wr, this, DEFORMATION_PERSPECTIVE),
    up_left_point(_("Top Left"), _("Top Left - <b>Ctrl+Alt+Click</b>: reset, <b>Ctrl</b>: move along axes"), "up_left_point", &wr, this),
    up_right_point(_("Top Right"), _("Top Right - <b>Ctrl+Alt+Click</b>: reset, <b>Ctrl</b>: move along axes"), "up_right_point", &wr, this),
    down_left_point(_("Down Left"), _("Down Left - <b>Ctrl+Alt+Click</b>: reset, <b>Ctrl</b>: move along axes"), "down_left_point", &wr, this),
    down_right_point(_("Down Right"), _("Down Right - <b>Ctrl+Alt+Click</b>: reset, <b>Ctrl</b>: move along axes"), "down_right_point", &wr, this)
{
    // register all your parameters here, so Inkscape knows which parameters this effect has:
    registerParameter(&deform_type);
    registerParameter(&horizontal_mirror);
    registerParameter(&vertical_mirror);
    registerParameter(&up_left_point);
    registerParameter(&up_right_point);
    registerParameter(&down_left_point);
    registerParameter(&down_right_point);
}

LPEPerspectiveEnvelope::~LPEPerspectiveEnvelope()
{
}

void LPEPerspectiveEnvelope::doEffect(SPCurve *curve)
{
    using Geom::X;
    using Geom::Y;
    if(are_near(up_left_point, up_right_point) &&
            are_near(up_right_point, down_left_point) &&
            are_near(down_left_point, down_right_point)) {
        g_warning("Perspective/Envelope LPE::doBeforeEffect - lpeobj with invalid parameter, the same value in 4 handles!");
        resetGrid();
        return;
    }
    double projmatrix[3][3];
    if(deform_type == DEFORMATION_PERSPECTIVE) {
        std::vector<Geom::Point> handles(4);
        handles[0] = down_left_point;
        handles[1] = up_left_point;
        handles[2] = up_right_point;
        handles[3] = down_right_point;
        std::vector<Geom::Point> source_handles(4);
        source_handles[0] = Geom::Point(boundingbox_X.min(), boundingbox_Y.max());
        source_handles[1] = Geom::Point(boundingbox_X.min(), boundingbox_Y.min());
        source_handles[2] = Geom::Point(boundingbox_X.max(), boundingbox_Y.min());
        source_handles[3] = Geom::Point(boundingbox_X.max(), boundingbox_Y.max());
        double solmatrix[8][8] = {{0}};
        double free_term[8] = {0};
        double gslSolmatrix[64];
        for(unsigned int i = 0; i < 4; ++i) {
            solmatrix[i][0] = source_handles[i][X];
            solmatrix[i][1] = source_handles[i][Y];
            solmatrix[i][2] = 1;
            solmatrix[i][6] = -handles[i][X] * source_handles[i][X];
            solmatrix[i][7] = -handles[i][X] * source_handles[i][Y];
            solmatrix[i+4][3] = source_handles[i][X];
            solmatrix[i+4][4] = source_handles[i][Y];
            solmatrix[i+4][5] = 1;
            solmatrix[i+4][6] = -handles[i][Y] * source_handles[i][X];
            solmatrix[i+4][7] = -handles[i][Y] * source_handles[i][Y];
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
                if(h==8) {
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
    Geom::Point point_at1(0, 0);
    Geom::Point point_at2(0, 0);
    Geom::Point point_at3(0, 0);
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
        if(deform_type == DEFORMATION_PERSPECTIVE) {
            nCurve->moveto(projectPoint(curve_it1->initialPoint(),projmatrix));
        } else {
            nCurve->moveto(projectPoint(curve_it1->initialPoint()));
        }
        while (curve_it1 != curve_endit) {
            cubic = dynamic_cast<Geom::CubicBezier const *>(&*curve_it1);
            if (cubic) {
                point_at1 = (*cubic)[1];
                point_at2 = (*cubic)[2];
            } else {
                point_at1 = curve_it1->initialPoint();
                point_at2 = curve_it1->finalPoint();
            }
            point_at3 = curve_it1->finalPoint();
            if(deform_type == DEFORMATION_PERSPECTIVE) {
                point_at1 = projectPoint(point_at1,projmatrix);
                point_at2 = projectPoint(point_at2,projmatrix);
                point_at3 = projectPoint(point_at3,projmatrix);
            } else {
                point_at1 = projectPoint(point_at1);
                point_at2 = projectPoint(point_at2);
                point_at3 = projectPoint(point_at3);
            }
            nCurve->curveto(point_at1, point_at2, point_at3);
            ++curve_it1;
            if(curve_it2 != curve_endit) {
                ++curve_it2;
            }
        }
        //y cerramos la curva
        if (path_it->closed()) {
            nCurve->move_endpoints(point_at3, point_at3);
            nCurve->closepath_current();
        }
        curve->append(nCurve, false);
        nCurve->reset();
        delete nCurve;
    }
}

Geom::Point
LPEPerspectiveEnvelope::projectPoint(Geom::Point p)
{
    double width = boundingbox_X.extent();
    double height = boundingbox_Y.extent();
    double delta_x = boundingbox_X.min() - p[X];
    double delta_y = boundingbox_Y.max() - p[Y];
    Geom::Coord x_ratio = (delta_x * -1) / width;
    Geom::Coord y_ratio = delta_y / height;
    Geom::Line* horiz = new Geom::Line();
    Geom::Line* vert = new Geom::Line();
    vert->setPoints (pointAtRatio(y_ratio,down_left_point,up_left_point),pointAtRatio(y_ratio,down_right_point,up_right_point));
    horiz->setPoints (pointAtRatio(x_ratio,down_left_point,down_right_point),pointAtRatio(x_ratio,up_left_point,up_right_point));

    OptCrossing crossPoint = intersection(*horiz,*vert);
    if(crossPoint) {
        return horiz->pointAt(Geom::Coord(crossPoint->ta));
    } else {
        return p;
    }
}

Geom::Point
LPEPerspectiveEnvelope::projectPoint(Geom::Point p, double m[][3])
{
    Geom::Coord x = p[0];
    Geom::Coord y = p[1];
    return Geom::Point(
               Geom::Coord((x*m[0][0] + y*m[0][1] + m[0][2])/(x*m[2][0]+y*m[2][1]+m[2][2])),
               Geom::Coord((x*m[1][0] + y*m[1][1] + m[1][2])/(x*m[2][0]+y*m[2][1]+m[2][2])));
}

Geom::Point
LPEPerspectiveEnvelope::pointAtRatio(Geom::Coord ratio,Geom::Point A, Geom::Point B)
{
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
    Gtk::HBox * hbox_up_handles = Gtk::manage(new Gtk::HBox(false,0));
    Gtk::HBox * hbox_down_handles = Gtk::manage(new Gtk::HBox(false,0));
    while (it != param_vector.end()) {
        if ((*it)->widget_is_visible) {
            Parameter * param = *it;
            Gtk::Widget * widg = dynamic_cast<Gtk::Widget *>(param->param_newWidget());
            if (param->param_key == "up_left_point" ||
                    param->param_key == "up_right_point" ||
                    param->param_key == "down_left_point" ||
                    param->param_key == "down_right_point") {
                Gtk::HBox * point_hbox = dynamic_cast<Gtk::HBox *>(widg);
                std::vector< Gtk::Widget* > child_list = point_hbox->get_children();
                Gtk::HBox * point_hboxHBox = dynamic_cast<Gtk::HBox *>(child_list[0]);
                std::vector< Gtk::Widget* > child_list2 = point_hboxHBox->get_children();
                point_hboxHBox->remove(child_list2[0][0]);
                Glib::ustring * tip = param->param_getTooltip();
                if (widg) {
                    if(param->param_key == "up_left_point") {
                        Gtk::Label* handles = Gtk::manage(new Gtk::Label(Glib::ustring(_("Handles:")),Gtk::ALIGN_START));
                        vbox->pack_start(*handles, false, false, 2);
                        hbox_up_handles->pack_start(*widg, true, true, 2);
                        hbox_up_handles->pack_start(*Gtk::manage(new Gtk::VSeparator()), Gtk::PACK_EXPAND_WIDGET);
                    } else if(param->param_key == "up_right_point") {
                        hbox_up_handles->pack_start(*widg, true, true, 2);
                    } else if(param->param_key == "down_left_point") {
                        hbox_down_handles->pack_start(*widg, true, true, 2);
                        hbox_down_handles->pack_start(*Gtk::manage(new Gtk::VSeparator()), Gtk::PACK_EXPAND_WIDGET);
                    } else {
                        hbox_down_handles->pack_start(*widg, true, true, 2);
                    }
                    if (tip) {
                        widg->set_tooltip_text(*tip);
                    } else {
                        widg->set_tooltip_text("");
                        widg->set_has_tooltip(false);
                    }
                }
            } else {
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
    vbox->pack_start(*hbox_up_handles,true, true, 2);
    Gtk::HBox * hbox_middle = Gtk::manage(new Gtk::HBox(true,2));
    hbox_middle->pack_start(*Gtk::manage(new Gtk::HSeparator()), Gtk::PACK_EXPAND_WIDGET);
    hbox_middle->pack_start(*Gtk::manage(new Gtk::HSeparator()), Gtk::PACK_EXPAND_WIDGET);
    vbox->pack_start(*hbox_middle, false, true, 2);
    vbox->pack_start(*hbox_down_handles, true, true, 2);
    Gtk::HBox * hbox = Gtk::manage(new Gtk::HBox(false,0));
    Gtk::Button* reset_button = Gtk::manage(new Gtk::Button(Gtk::Stock::CLEAR));
    reset_button->signal_clicked().connect(sigc::mem_fun (*this,&LPEPerspectiveEnvelope::resetGrid));
    reset_button->set_size_request(140,30);
    vbox->pack_start(*hbox, true,true,2);
    hbox->pack_start(*reset_button, false, false,2);
    return dynamic_cast<Gtk::Widget *>(vbox);
}

void
LPEPerspectiveEnvelope::vertical(PointParam &param_one, PointParam &param_two, Geom::Line vert)
{
    Geom::Point A = param_one;
    Geom::Point B = param_two;
    double Y = (A[Geom::Y] + B[Geom::Y])/2;
    A[Geom::Y] = Y;
    B[Geom::Y] = Y;
    Geom::Point nearest = vert.pointAt(vert.nearestTime(A));
    double distance_one = Geom::distance(A,nearest);
    double distance_two = Geom::distance(B,nearest);
    double distance_middle = (distance_one + distance_two)/2;
    if(A[Geom::X] > B[Geom::X]) {
        distance_middle *= -1;
    }
    A[Geom::X] = nearest[Geom::X] - distance_middle;
    B[Geom::X] = nearest[Geom::X] + distance_middle;
    param_one.param_setValue(A, true);
    param_two.param_setValue(B, true);
}

void
LPEPerspectiveEnvelope::horizontal(PointParam &param_one, PointParam &param_two, Geom::Line horiz)
{
    Geom::Point A = param_one;
    Geom::Point B = param_two;
    double X = (A[Geom::X] + B[Geom::X])/2;
    A[Geom::X] = X;
    B[Geom::X] = X;
    Geom::Point nearest = horiz.pointAt(horiz.nearestTime(A));
    double distance_one = Geom::distance(A,nearest);
    double distance_two = Geom::distance(B,nearest);
    double distance_middle = (distance_one + distance_two)/2;
    if(A[Geom::Y] > B[Geom::Y]) {
        distance_middle *= -1;
    }
    A[Geom::Y] = nearest[Geom::Y] - distance_middle;
    B[Geom::Y] = nearest[Geom::Y] + distance_middle;
    param_one.param_setValue(A, true);
    param_two.param_setValue(B, true);
}

void
LPEPerspectiveEnvelope::doBeforeEffect (SPLPEItem const* lpeitem)
{
    original_bbox(lpeitem);
    Geom::Line vert(Geom::Point(boundingbox_X.middle(),boundingbox_Y.max()), Geom::Point(boundingbox_X.middle(), boundingbox_Y.min()));
    Geom::Line horiz(Geom::Point(boundingbox_X.min(),boundingbox_Y.middle()), Geom::Point(boundingbox_X.max(), boundingbox_Y.middle()));
    if(vertical_mirror) {
        vertical(up_left_point, up_right_point,vert);
        vertical(down_left_point, down_right_point,vert);
    }
    if(horizontal_mirror) {
        horizontal(up_left_point, down_left_point,horiz);
        horizontal(up_right_point, down_right_point,horiz);
    }
    SPLPEItem * item = const_cast<SPLPEItem*>(lpeitem);
    item->apply_to_clippath(item);
    item->apply_to_mask(item);
    setDefaults();
}

void
LPEPerspectiveEnvelope::setDefaults()
{
    Geom::Point up_left(boundingbox_X.min(), boundingbox_Y.min());
    Geom::Point up_right(boundingbox_X.max(), boundingbox_Y.min());
    Geom::Point down_left(boundingbox_X.min(), boundingbox_Y.max());
    Geom::Point down_right(boundingbox_X.max(), boundingbox_Y.max());

    up_left_point.param_update_default(up_left);
    up_right_point.param_update_default(up_right);
    down_right_point.param_update_default(down_right);
    down_left_point.param_update_default(down_left);
}

void
LPEPerspectiveEnvelope::resetGrid()
{
    up_left_point.param_set_default();
    up_right_point.param_set_default();
    down_right_point.param_set_default();
    down_left_point.param_set_default();
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
    c->moveto(up_left_point);
    c->lineto(up_right_point);
    c->lineto(down_right_point);
    c->lineto(down_left_point);
    c->lineto(up_left_point);
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
