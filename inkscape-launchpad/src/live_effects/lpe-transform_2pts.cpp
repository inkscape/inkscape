/** \file
 * LPE "Transform through 2 points" implementation
 */

/*
 * Authors:
 *   Jabier Arraiza Cenoz<jabier.arraiza@marker.es>
 *
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtkmm.h>

#include "live_effects/lpe-transform_2pts.h"
#include "display/curve.h"
#include <2geom/transforms.h>
#include <2geom/pathvector.h>
#include "sp-path.h"
#include "ui/icon-names.h"
#include "svg/svg.h"
#include "verbs.h"
// TODO due to internal breakage in glibmm headers, this must be last:
#include <glibmm/i18n.h>

namespace Inkscape {
namespace LivePathEffect {

LPETransform2Pts::LPETransform2Pts(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    elastic(_("Elastic"), _("Elastic transform mode"), "elastic", &wr, this, false,"", INKSCAPE_ICON("on"), INKSCAPE_ICON("off")),
    from_original_width(_("From original width"), _("From original width"), "from_original_width", &wr, this, false,"", INKSCAPE_ICON("on"), INKSCAPE_ICON("off")),
    lock_lenght(_("Lock length"), _("Lock length to current distance"), "lock_lenght", &wr, this, false,"", INKSCAPE_ICON("on"), INKSCAPE_ICON("off")),
    lock_angle(_("Lock angle"), _("Lock angle"), "lock_angle", &wr, this, false,"", INKSCAPE_ICON("on"), INKSCAPE_ICON("off")),
    flip_horizontal(_("Flip horizontal"), _("Flip horizontal"), "flip_horizontal", &wr, this, false,"", INKSCAPE_ICON("on"), INKSCAPE_ICON("off")),
    flip_vertical(_("Flip vertical"), _("Flip vertical"), "flip_vertical", &wr, this, false,"", INKSCAPE_ICON("on"), INKSCAPE_ICON("off")),
    start(_("Start"), _("Start point"), "start", &wr, this, "Start point"),
    end(_("End"), _("End point"), "end", &wr, this, "End point"),
    strech(_("Stretch"), _("Stretch the result"), "strech", &wr, this, 1),
    offset(_("Offset"), _("Offset from knots"), "offset", &wr, this, 0),
    first_knot(_("First Knot"), _("First Knot"), "first_knot", &wr, this, 1),
    last_knot(_("Last Knot"), _("Last Knot"), "last_knot", &wr, this, 1),
    helper_size(_("Helper size:"), _("Rotation helper size"), "helper_size", &wr, this, 3),
    from_original_width_toggler(false),
    point_a(Geom::Point()),
    point_b(Geom::Point()),
    pathvector(),
    append_path(false),
    previous_angle(Geom::deg_to_rad(0)),
    previous_start(Geom::Point()),
    previous_lenght(-1)
{

    registerParameter(&first_knot);
    registerParameter(&last_knot);
    registerParameter(&helper_size);
    registerParameter(&strech);
    registerParameter(&offset);
    registerParameter(&start);
    registerParameter(&end);
    registerParameter(&elastic);
    registerParameter(&from_original_width);
    registerParameter(&flip_vertical);
    registerParameter(&flip_horizontal);
    registerParameter(&lock_lenght);
    registerParameter(&lock_angle);

    first_knot.param_make_integer(true);
    first_knot.param_overwrite_widget(true);
    last_knot.param_make_integer(true);
    last_knot.param_overwrite_widget(true);
    helper_size.param_set_range(0, 999);
    helper_size.param_set_increments(1, 1);
    helper_size.param_set_digits(0);
    offset.param_set_range(-999999.0, 999999.0);
    offset.param_set_increments(1, 1);
    offset.param_set_digits(2);
    strech.param_set_range(0, 999.0);
    strech.param_set_increments(0.01, 0.01);
    strech.param_set_digits(4);
}

LPETransform2Pts::~LPETransform2Pts()
{
}

void
LPETransform2Pts::doOnApply(SPLPEItem const* lpeitem)
{
    using namespace Geom;
    original_bbox(lpeitem);

    point_a = Point(boundingbox_X.min(), boundingbox_Y.middle());
    point_b = Point(boundingbox_X.max(), boundingbox_Y.middle());
    SPLPEItem * splpeitem = const_cast<SPLPEItem *>(lpeitem);
    SPPath *sp_path = dynamic_cast<SPPath *>(splpeitem);
    if (sp_path) {
        pathvector = sp_path->get_original_curve()->get_pathvector();
    }
    if(!pathvector.empty()) {
        point_a = pathvector.initialPoint();
        point_b = pathvector.finalPoint();
        if(are_near(point_a,point_b)) {
            point_b = pathvector.back().finalCurve().initialPoint();
        }
        size_t nnodes = nodeCount(pathvector);
        last_knot.param_set_value(nnodes);
    }

    previous_lenght = Geom::distance(point_a,point_b);
    Geom::Ray transformed(point_a,point_b);
    previous_angle = transformed.angle();
    start.param_update_default(point_a);
    start.param_set_default();
    end.param_update_default(point_b);
    end.param_set_default();
}

void
LPETransform2Pts::doBeforeEffect (SPLPEItem const* lpeitem)
{
    using namespace Geom;
    original_bbox(lpeitem);
    point_a = Point(boundingbox_X.min(), boundingbox_Y.middle());
    point_b = Point(boundingbox_X.max(), boundingbox_Y.middle());

    SPLPEItem * splpeitem = const_cast<SPLPEItem *>(lpeitem);
    SPPath *sp_path = dynamic_cast<SPPath *>(splpeitem);
    if (sp_path) {
        pathvector = sp_path->get_original_curve()->get_pathvector();
    }
    if(from_original_width_toggler != from_original_width) {
        from_original_width_toggler = from_original_width;
        reset();
    }
    if(!pathvector.empty() && !from_original_width) {
        append_path = false;
        point_a = pointAtNodeIndex(pathvector,(size_t)first_knot-1);
        point_b = pointAtNodeIndex(pathvector,(size_t)last_knot-1);
        size_t nnodes = nodeCount(pathvector);
        first_knot.param_set_range(1, last_knot-1);
        last_knot.param_set_range(first_knot+1, nnodes);
        from_original_width.param_setValue(false);
    } else {
        first_knot.param_set_value(1);
        last_knot.param_set_value(2);
        first_knot.param_set_range(1,1);
        last_knot.param_set_range(2,2);
        from_original_width.param_setValue(true);
        append_path = false;
    }
    if(lock_lenght && !lock_angle && previous_lenght != -1) {
        Geom::Ray transformed((Geom::Point)start,(Geom::Point)end);
        if(previous_start == start || previous_angle == Geom::deg_to_rad(0)) {
            previous_angle = transformed.angle();
        }
    } else if(lock_angle && !lock_lenght && previous_angle != Geom::deg_to_rad(0)) {
        if(previous_start == start){
            previous_lenght = Geom::distance((Geom::Point)start, (Geom::Point)end);
        }
    }
    if(lock_lenght || lock_angle ) {
        Geom::Point end_point = Geom::Point::polar(previous_angle, previous_lenght) + (Geom::Point)start;
        end.param_setValue(end_point);
    }
    Geom::Ray transformed((Geom::Point)start,(Geom::Point)end);
    previous_angle = transformed.angle();
    previous_lenght = Geom::distance((Geom::Point)start, (Geom::Point)end);
    previous_start = start;
    splpeitem->apply_to_clippath(splpeitem);
    splpeitem->apply_to_mask(splpeitem);
}

void
LPETransform2Pts::updateIndex()
{
    SPLPEItem * splpeitem = const_cast<SPLPEItem *>(sp_lpe_item);
    SPPath *sp_path = dynamic_cast<SPPath *>(splpeitem);
    if (sp_path) {
        pathvector = sp_path->get_original_curve()->get_pathvector();
    }
    if(pathvector.empty()) {
        return;
    }
    if(!from_original_width) {
        point_a = pointAtNodeIndex(pathvector,(size_t)first_knot-1);
        point_b = pointAtNodeIndex(pathvector,(size_t)last_knot-1);
        start.param_update_default(point_a);
        start.param_set_default();
        end.param_update_default(point_b);
        end.param_set_default();
        start.param_update_default(point_a);
        end.param_update_default(point_b);
        start.param_set_default();
        end.param_set_default();
    }
    DocumentUndo::done(getSPDoc(), SP_VERB_DIALOG_LIVE_PATH_EFFECT, _("Change index of knot"));
}
//todo migrate to PathVector class?
size_t
LPETransform2Pts::nodeCount(Geom::PathVector pathvector) const
{
    size_t n = 0;
    for (Geom::PathVector::iterator it = pathvector.begin(); it != pathvector.end(); ++it) {
        n += it->size_closed();
    }
    return n;
}
//todo migrate to PathVector class?
Geom::Point
LPETransform2Pts::pointAtNodeIndex(Geom::PathVector pathvector, size_t index) const
{
    size_t n = 0;
    for (Geom::PathVector::iterator pv_it = pathvector.begin(); pv_it != pathvector.end(); ++pv_it) {
        for (Geom::Path::iterator curve_it = pv_it->begin(); curve_it != pv_it->end_closed(); ++curve_it) {
            if(index == n) {
                return curve_it->initialPoint();
            }
            n++;
        }
    }
    return Geom::Point();
}
//todo migrate to PathVector class? Not used
Geom::Path
LPETransform2Pts::pathAtNodeIndex(Geom::PathVector pathvector, size_t index) const
{
    size_t n = 0;
    for (Geom::PathVector::iterator pv_it = pathvector.begin(); pv_it != pathvector.end(); ++pv_it) {
        for (Geom::Path::iterator curve_it = pv_it->begin(); curve_it != pv_it->end_closed(); ++curve_it) {
            if(index == n) {
                return *pv_it;
            }
            n++;
        }
    }
    return Geom::Path();
}


void
LPETransform2Pts::reset()
{
    point_a = Geom::Point(boundingbox_X.min(), boundingbox_Y.middle());
    point_b = Geom::Point(boundingbox_X.max(), boundingbox_Y.middle());
    if(!pathvector.empty() && !from_original_width) {
        size_t nnodes = nodeCount(pathvector);
        first_knot.param_set_range(1, last_knot-1);
        last_knot.param_set_range(first_knot+1, nnodes);
        first_knot.param_set_value(1);
        last_knot.param_set_value(nnodes);
        point_a = pathvector.initialPoint();
        point_b = pathvector.finalPoint();
    } else {
        first_knot.param_set_value(1);
        last_knot.param_set_value(2);
    }
    Geom::Ray transformed(point_a, point_b);
    previous_angle = transformed.angle();
    previous_lenght = Geom::distance(point_a, point_b);
    start.param_update_default(point_a);
    end.param_update_default(point_b);
    start.param_set_default();
    end.param_set_default();
}

Gtk::Widget *LPETransform2Pts::newWidget()
{
    // use manage here, because after deletion of Effect object, others might
    // still be pointing to this widget.
    Gtk::VBox *vbox = Gtk::manage(new Gtk::VBox(Effect::newWidget()));

    vbox->set_border_width(5);
    vbox->set_homogeneous(false);
    vbox->set_spacing(6);

    std::vector<Parameter *>::iterator it = param_vector.begin();
    Gtk::HBox * button1 = Gtk::manage(new Gtk::HBox(true,0));
    Gtk::HBox * button2 = Gtk::manage(new Gtk::HBox(true,0));
    Gtk::HBox * button3 = Gtk::manage(new Gtk::HBox(true,0));
    Gtk::HBox * button4 = Gtk::manage(new Gtk::HBox(true,0));
    while (it != param_vector.end()) {
        if ((*it)->widget_is_visible) {
            Parameter *param = *it;
            Gtk::Widget *widg = dynamic_cast<Gtk::Widget *>(param->param_newWidget());
            Glib::ustring *tip = param->param_getTooltip();
            if (param->param_key == "first_knot" || param->param_key == "last_knot") {
                Inkscape::UI::Widget::Scalar *registered_widget = Gtk::manage(dynamic_cast<Inkscape::UI::Widget::Scalar *>(widg));
                registered_widget->signal_value_changed().connect(sigc::mem_fun(*this, &LPETransform2Pts::updateIndex));
                widg = registered_widget;
                if (widg) {
                    Gtk::HBox *hbox_scalar = dynamic_cast<Gtk::HBox *>(widg);
                    std::vector<Gtk::Widget *> child_list = hbox_scalar->get_children();
                    Gtk::Entry *entry_widget = dynamic_cast<Gtk::Entry *>(child_list[1]);
                    entry_widget->set_width_chars(3);
                    vbox->pack_start(*widg, true, true, 2);
                    if (tip) {
                        widg->set_tooltip_text(*tip);
                    } else {
                        widg->set_tooltip_text("");
                        widg->set_has_tooltip(false);
                    }
                }
            } else if (param->param_key == "from_original_width" || param->param_key == "elastic") {
                Glib::ustring * tip = param->param_getTooltip();
                if (widg) {
                    button1->pack_start(*widg, true, true, 2);
                    if (tip) {
                        widg->set_tooltip_text(*tip);
                    } else {
                        widg->set_tooltip_text("");
                        widg->set_has_tooltip(false);
                    }
                }
            } else if (param->param_key == "flip_horizontal" || param->param_key == "flip_vertical") {
                Glib::ustring * tip = param->param_getTooltip();
                if (widg) {
                    button2->pack_start(*widg, true, true, 2);
                    if (tip) {
                        widg->set_tooltip_text(*tip);
                    } else {
                        widg->set_tooltip_text("");
                        widg->set_has_tooltip(false);
                    }
                }
            } else if (param->param_key == "lock_angle" || param->param_key == "lock_lenght") {
                Glib::ustring * tip = param->param_getTooltip();
                if (widg) {
                    button3->pack_start(*widg, true, true, 2);
                    if (tip) {
                        widg->set_tooltip_text(*tip);
                    } else {
                        widg->set_tooltip_text("");
                        widg->set_has_tooltip(false);
                    }
                }
            } else if (widg) {
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
    Gtk::Button *reset = Gtk::manage(new Gtk::Button(Glib::ustring(_("Reset"))));
    reset->signal_clicked().connect(sigc::mem_fun(*this, &LPETransform2Pts::reset));
    button4->pack_start(*reset, true, true, 2);
    vbox->pack_start(*button1, true, true, 2);
    vbox->pack_start(*button2, true, true, 2);
    vbox->pack_start(*button3, true, true, 2);
    vbox->pack_start(*button4, true, true, 2);
    return dynamic_cast<Gtk::Widget *>(vbox);
}

Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPETransform2Pts::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    Geom::Piecewise<Geom::D2<Geom::SBasis> > output;
    double sca = Geom::distance((Geom::Point)start,(Geom::Point)end)/Geom::distance(point_a,point_b);
    Geom::Ray original(point_a,point_b);
    Geom::Ray transformed((Geom::Point)start,(Geom::Point)end);
    double rot = transformed.angle() - original.angle();
    Geom::Path helper;
    helper.start(point_a);
    helper.appendNew<Geom::LineSegment>(point_b);
    Geom::Affine m;
    Geom::Angle original_angle = original.angle();
    if(flip_horizontal && flip_vertical){
        m *= Geom::Rotate(-original_angle);
        m *= Geom::Scale(-1,-1);
        m *= Geom::Rotate(original_angle);
    } else if(flip_vertical){
        m *= Geom::Rotate(-original_angle);
        m *= Geom::Scale(1,-1);
        m *= Geom::Rotate(original_angle);
    } else if(flip_horizontal){
        m *= Geom::Rotate(-original_angle);
        m *= Geom::Scale(-1,1);
        m *= Geom::Rotate(original_angle);
    }
    if(strech != 1){
        m *= Geom::Rotate(-original_angle);
        m *= Geom::Scale(1,strech);
        m *= Geom::Rotate(original_angle);
    }
    if(elastic) {
        m *= Geom::Rotate(-original_angle);
        if(sca > 1){
            m *= Geom::Scale(sca, 1.0);
        } else {
            m *= Geom::Scale(sca, 1.0-((1.0-sca)/2.0));
        }
        m *= Geom::Rotate(transformed.angle());
    } else {
        m *= Geom::Scale(sca);
        m *= Geom::Rotate(rot);
    }
    helper *= m;
    Geom::Point trans = (Geom::Point)start - helper.initialPoint();
    if(flip_horizontal){
        trans = (Geom::Point)end - helper.initialPoint();
    }
    if(offset != 0){
        trans = Geom::Point::polar(transformed.angle() + Geom::deg_to_rad(-90),offset) + trans;
    }
    m *= Geom::Translate(trans);

    output.concat(pwd2_in * m);

    return output;
}

void
LPETransform2Pts::addCanvasIndicators(SPLPEItem const */*lpeitem*/, std::vector<Geom::PathVector> &hp_vec)
{
    using namespace Geom;
    hp_vec.clear();
    Geom::Path hp;
    hp.start((Geom::Point)start);
    hp.appendNew<Geom::LineSegment>((Geom::Point)end);
    Geom::PathVector pathv;
    pathv.push_back(hp);
    double r = helper_size*.1;
    if(lock_lenght || lock_angle ) {
        char const * svgd;
        svgd = "M -5.39,8.78 -9.13,5.29 -10.38,10.28 Z M -7.22,7.07 -3.43,3.37 m -1.95,-12.16 -3.74,3.5 -1.26,-5 z m -1.83,1.71 3.78,3.7 M 5.24,8.78 8.98,5.29 10.24,10.28 Z M 7.07,7.07 3.29,3.37 M 5.24,-8.78 l 3.74,3.5 1.26,-5 z M 7.07,-7.07 3.29,-3.37";
        PathVector pathv_move = sp_svg_read_pathv(svgd);
        pathv_move *= Affine(r,0,0,r,0,0) * Translate(Geom::Point(start));
        hp_vec.push_back(pathv_move);
    }
    if(!lock_angle && lock_lenght) {
        char const * svgd;
        svgd = "m 7.07,7.07 c -3.9,3.91 -10.24,3.91 -14.14,0 -3.91,-3.9 -3.91,-10.24 0,-14.14 3.9,-3.91 10.24,-3.91 14.14,0 l -2.83,-4.24 -0.7,2.12";
        PathVector pathv_turn = sp_svg_read_pathv(svgd);
        pathv_turn *= Geom::Rotate(previous_angle);
        pathv_turn *= Affine(r,0,0,r,0,0) * Translate(Geom::Point(end));
        hp_vec.push_back(pathv_turn);
    }
    hp_vec.push_back(pathv);
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
