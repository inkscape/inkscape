/*
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#include <gtkmm.h>
#include "live_effects/lpe-bspline.h"
#include "ui/widget/scalar.h"
#include "display/curve.h"
#include "helper/geom-curves.h"
#include "sp-path.h"
#include "svg/svg.h"
#include "xml/repr.h"
#include "preferences.h"
#include "document-undo.h"
#include "verbs.h"
// TODO due to internal breakage in glibmm headers, this must be last:
#include <glibmm/i18n.h>

namespace Inkscape {
namespace LivePathEffect {

const double HANDLE_CUBIC_GAP = 0.001;
const double NO_POWER = 0.0;
const double DEFAULT_START_POWER = 0.333334;
const double DEFAULT_END_POWER = 0.666667;
Geom::PathVector hp;
void sp_bspline_drawHandle(Geom::Point p, double helper_size);

LPEBSpline::LPEBSpline(LivePathEffectObject *lpeobject)
    : Effect(lpeobject),
      steps(_("Steps with CTRL:"), _("Change number of steps with CTRL pressed"), "steps", &wr, this, 2),
      helper_size(_("Helper size:"), _("Helper size"), "helper_size", &wr, this, 0),
      apply_no_weight(_("Apply changes if weight = 0%"), _("Apply changes if weight = 0%"), "apply_no_weight", &wr, this, true),
      apply_with_weight(_("Apply changes if weight > 0%"), _("Apply changes if weight > 0%"), "apply_with_weight", &wr, this, true),
      only_selected(_("Change only selected nodes"), _("Change only selected nodes"), "only_selected", &wr, this, false),
      weight(_("Change weight %:"), _("Change weight percent of the effect"), "weight", &wr, this, DEFAULT_START_POWER * 100)
{
    registerParameter(&weight);
    registerParameter(&steps);
    registerParameter(&helper_size);
    registerParameter(&apply_no_weight);
    registerParameter(&apply_with_weight);
    registerParameter(&only_selected);

    weight.param_set_range(NO_POWER, 100.0);
    weight.param_set_increments(0.1, 0.1);
    weight.param_set_digits(4);
    weight.param_overwrite_widget(true);

    steps.param_set_range(1, 10);
    steps.param_set_increments(1, 1);
    steps.param_set_digits(0);
    steps.param_overwrite_widget(true);

    helper_size.param_set_range(0.0, 999.0);
    helper_size.param_set_increments(1, 1);
    helper_size.param_set_digits(2);
}

LPEBSpline::~LPEBSpline() {}

void LPEBSpline::doBeforeEffect (SPLPEItem const* /*lpeitem*/)
{
    if(!hp.empty()) {
        hp.clear();
    }
}


void LPEBSpline::doOnApply(SPLPEItem const* lpeitem)
{
    if (!SP_IS_SHAPE(lpeitem)) {
        g_warning("LPE BSpline can only be applied to shapes (not groups).");
        SPLPEItem * item = const_cast<SPLPEItem*>(lpeitem);
        item->removeCurrentPathEffect(false);
    }
}

void
LPEBSpline::addCanvasIndicators(SPLPEItem const */*lpeitem*/, std::vector<Geom::PathVector> &hp_vec)
{
    hp_vec.push_back(hp);
}

Gtk::Widget *LPEBSpline::newWidget()
{
    // use manage here, because after deletion of Effect object, others might
    // still be pointing to this widget.
    Gtk::VBox *vbox = Gtk::manage(new Gtk::VBox(Effect::newWidget()));

    vbox->set_border_width(5);
    std::vector<Parameter *>::iterator it = param_vector.begin();
    while (it != param_vector.end()) {
        if ((*it)->widget_is_visible) {
            Parameter *param = *it;
            Gtk::Widget *widg = dynamic_cast<Gtk::Widget *>(param->param_newWidget());
            if (param->param_key == "weight") {
                Gtk::HBox * buttons = Gtk::manage(new Gtk::HBox(true,0));
                Gtk::Button *default_weight =
                    Gtk::manage(new Gtk::Button(Glib::ustring(_("Default weight"))));
                default_weight->signal_clicked()
                .connect(sigc::mem_fun(*this, &LPEBSpline::toDefaultWeight));
                buttons->pack_start(*default_weight, true, true, 2);
                Gtk::Button *make_cusp =
                    Gtk::manage(new Gtk::Button(Glib::ustring(_("Make cusp"))));
                make_cusp->signal_clicked()
                .connect(sigc::mem_fun(*this, &LPEBSpline::toMakeCusp));
                buttons->pack_start(*make_cusp, true, true, 2);
                vbox->pack_start(*buttons, true, true, 2);
            }
            if (param->param_key == "weight" || param->param_key == "steps") {
                Inkscape::UI::Widget::Scalar *widg_registered =
                    Gtk::manage(dynamic_cast<Inkscape::UI::Widget::Scalar *>(widg));
                widg_registered->signal_value_changed()
                .connect(sigc::mem_fun(*this, &LPEBSpline::toWeight));
                widg = dynamic_cast<Gtk::Widget *>(widg_registered);
                if (widg) {
                    Gtk::HBox * hbox_weight_steps = dynamic_cast<Gtk::HBox *>(widg);
                    std::vector< Gtk::Widget* > childList = hbox_weight_steps->get_children();
                    Gtk::Entry* entry_widget = dynamic_cast<Gtk::Entry *>(childList[1]);
                    entry_widget->set_width_chars(9);
                }
            }
            if (param->param_key == "only_selected" || param->param_key == "apply_no_weight" || param->param_key == "apply_with_weight") {
                Gtk::CheckButton *widg_registered =
                    Gtk::manage(dynamic_cast<Gtk::CheckButton *>(widg));
                widg = dynamic_cast<Gtk::Widget *>(widg_registered);
            }
            Glib::ustring *tip = param->param_getTooltip();
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

void LPEBSpline::toDefaultWeight()
{
    changeWeight(DEFAULT_START_POWER * 100);
    DocumentUndo::done(getSPDoc(), SP_VERB_DIALOG_LIVE_PATH_EFFECT, _("Change to default weight"));
}

void LPEBSpline::toMakeCusp()
{
    changeWeight(NO_POWER);
    DocumentUndo::done(getSPDoc(), SP_VERB_DIALOG_LIVE_PATH_EFFECT, _("Change to 0 weight"));
}

void LPEBSpline::toWeight()
{
    changeWeight(weight);
    DocumentUndo::done(getSPDoc(), SP_VERB_DIALOG_LIVE_PATH_EFFECT, _("Change scalar parameter"));
}

void LPEBSpline::changeWeight(double weight_ammount)
{
    SPPath *path = dynamic_cast<SPPath *>(sp_lpe_item);
    if(path) {
        SPCurve *curve = path->get_curve_for_edit();
        doBSplineFromWidget(curve, weight_ammount/100.0);
        gchar *str = sp_svg_write_path(curve->get_pathvector());
        path->getRepr()->setAttribute("inkscape:original-d", str);
    }
}

void LPEBSpline::doEffect(SPCurve *curve)
{
    sp_bspline_do_effect(curve, helper_size);
}

void sp_bspline_do_effect(SPCurve *curve, double helper_size)
{
    if (curve->get_segment_count() < 1) {
        return;
    }
    Geom::PathVector const original_pathv = curve->get_pathvector();
    curve->reset();
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    for (Geom::PathVector::const_iterator path_it = original_pathv.begin();
            path_it != original_pathv.end(); ++path_it) {
        if (path_it->empty()) {
            continue;
        }
        if (!prefs->getBool("/tools/nodes/show_outline", true)){
            hp.push_back(*path_it);
        }
        Geom::Path::const_iterator curve_it1 = path_it->begin();
        Geom::Path::const_iterator curve_it2 = ++(path_it->begin());
        Geom::Path::const_iterator curve_endit = path_it->end_default();
        SPCurve *curve_n = new SPCurve();
        Geom::Point previousNode(0, 0);
        Geom::Point node(0, 0);
        Geom::Point point_at1(0, 0);
        Geom::Point point_at2(0, 0);
        Geom::Point next_point_at1(0, 0);
        Geom::D2<Geom::SBasis> sbasis_in;
        Geom::D2<Geom::SBasis> sbasis_out;
        Geom::D2<Geom::SBasis> sbasis_helper;
        Geom::CubicBezier const *cubic = NULL;
        curve_n->moveto(curve_it1->initialPoint());
        if (path_it->closed()) {
          const Geom::Curve &closingline = path_it->back_closed(); 
          // the closing line segment is always of type 
          // Geom::LineSegment.
          if (are_near(closingline.initialPoint(), closingline.finalPoint())) {
            // closingline.isDegenerate() did not work, because it only checks for
            // *exact* zero length, which goes wrong for relative coordinates and
            // rounding errors...
            // the closing line segment has zero-length. So stop before that one!
            curve_endit = path_it->end_open();
          }
        }
        while (curve_it1 != curve_endit) {
            SPCurve *in = new SPCurve();
            in->moveto(curve_it1->initialPoint());
            in->lineto(curve_it1->finalPoint());
            cubic = dynamic_cast<Geom::CubicBezier const *>(&*curve_it1);
            if (cubic) {
                sbasis_in = in->first_segment()->toSBasis();
                if(are_near((*cubic)[1],(*cubic)[0]) && !are_near((*cubic)[2],(*cubic)[3])) {
                    point_at1 = sbasis_in.valueAt(DEFAULT_START_POWER);
                } else {
                    point_at1 = sbasis_in.valueAt(Geom::nearest_time((*cubic)[1], *in->first_segment()));
                }
                if(are_near((*cubic)[2],(*cubic)[3]) && !are_near((*cubic)[1],(*cubic)[0])) {
                    point_at2 = sbasis_in.valueAt(DEFAULT_END_POWER);
                } else {
                    point_at2 = sbasis_in.valueAt(Geom::nearest_time((*cubic)[2], *in->first_segment()));
                }
            } else {
                point_at1 = in->first_segment()->initialPoint();
                point_at2 = in->first_segment()->finalPoint();
            }
            in->reset();
            delete in;
            if ( curve_it2 != curve_endit ) {
                SPCurve *out = new SPCurve();
                out->moveto(curve_it2->initialPoint());
                out->lineto(curve_it2->finalPoint());
                cubic = dynamic_cast<Geom::CubicBezier const *>(&*curve_it2);
                if (cubic) {
                    sbasis_out = out->first_segment()->toSBasis();
                    if(are_near((*cubic)[1],(*cubic)[0]) && !are_near((*cubic)[2],(*cubic)[3])) {
                        next_point_at1 = sbasis_in.valueAt(DEFAULT_START_POWER);
                    } else {
                        next_point_at1 = sbasis_out.valueAt(Geom::nearest_time((*cubic)[1], *out->first_segment()));
                    }
                } else {
                    next_point_at1 = out->first_segment()->initialPoint();
                }
                out->reset();
                delete out;
            }
            if (path_it->closed() && curve_it2 == curve_endit) {
                SPCurve *start = new SPCurve();
                start->moveto(path_it->begin()->initialPoint());
                start->lineto(path_it->begin()->finalPoint());
                Geom::D2<Geom::SBasis> sbasis_start = start->first_segment()->toSBasis();
                SPCurve *line_helper = new SPCurve();
                cubic = dynamic_cast<Geom::CubicBezier const *>(&*path_it->begin());
                if (cubic) {
                    line_helper->moveto(sbasis_start.valueAt(
                                           Geom::nearest_time((*cubic)[1], *start->first_segment())));
                } else {
                    line_helper->moveto(start->first_segment()->initialPoint());
                }
                start->reset();
                delete start;

                SPCurve *end = new SPCurve();
                end->moveto(curve_it1->initialPoint());
                end->lineto(curve_it1->finalPoint());
                Geom::D2<Geom::SBasis> sbasis_end = end->first_segment()->toSBasis();
                cubic = dynamic_cast<Geom::CubicBezier const *>(&*curve_it1);
                if (cubic) {
                    line_helper->lineto(sbasis_end.valueAt(
                                           Geom::nearest_time((*cubic)[2], *end->first_segment())));
                } else {
                    line_helper->lineto(end->first_segment()->finalPoint());
                }
                end->reset();
                delete end;
                sbasis_helper = line_helper->first_segment()->toSBasis();
                line_helper->reset();
                delete line_helper;
                node = sbasis_helper.valueAt(0.5);
                curve_n->curveto(point_at1, point_at2, node);
                curve_n->move_endpoints(node, node);
            } else if ( curve_it2 == curve_endit) {
                curve_n->curveto(point_at1, point_at2, curve_it1->finalPoint());
                curve_n->move_endpoints(path_it->begin()->initialPoint(), curve_it1->finalPoint());
            } else {
                SPCurve *line_helper = new SPCurve();
                line_helper->moveto(point_at2);
                line_helper->lineto(next_point_at1);
                sbasis_helper = line_helper->first_segment()->toSBasis();
                line_helper->reset();
                delete line_helper;
                previousNode = node;
                node = sbasis_helper.valueAt(0.5);
                Geom::CubicBezier const *cubic2 = dynamic_cast<Geom::CubicBezier const *>(&*curve_it1);
                if((cubic && are_near((*cubic)[0],(*cubic)[1])) || (cubic2 && are_near((*cubic2)[2],(*cubic2)[3]))) {
                    node = curve_it1->finalPoint();
                }
                curve_n->curveto(point_at1, point_at2, node);
            }
            if(!are_near(node,curve_it1->finalPoint()) && helper_size > 0.0) {
                sp_bspline_drawHandle(node, helper_size);
            }
            ++curve_it1;
            ++curve_it2;
        }
        if (path_it->closed()) {
            curve_n->closepath_current();
        }
        curve->append(curve_n, false);
        curve_n->reset();
        delete curve_n;
    }
    if(helper_size > 0.0) {
        Geom::PathVector const pathv = curve->get_pathvector();
        hp.push_back(pathv[0]);
    }
}


void sp_bspline_drawHandle(Geom::Point p, double helper_size)
{
    char const * svgd = "M 1,0.5 A 0.5,0.5 0 0 1 0.5,1 0.5,0.5 0 0 1 0,0.5 0.5,0.5 0 0 1 0.5,0 0.5,0.5 0 0 1 1,0.5 Z";
    Geom::PathVector pathv = sp_svg_read_pathv(svgd);
    Geom::Affine aff = Geom::Affine();
    aff *= Geom::Scale(helper_size);
    pathv *= aff;
    pathv *= Geom::Translate(p - Geom::Point(0.5*helper_size, 0.5*helper_size));
    hp.push_back(pathv[0]);
}

void LPEBSpline::doBSplineFromWidget(SPCurve *curve, double weight_ammount)
{
    using Geom::X;
    using Geom::Y;

    if (curve->get_segment_count() < 1)
        return;
    // Make copy of old path as it is changed during processing
    Geom::PathVector const original_pathv = curve->get_pathvector();
    curve->reset();

    for (Geom::PathVector::const_iterator path_it = original_pathv.begin();
            path_it != original_pathv.end(); ++path_it) {

        if (path_it->empty()) {
            continue;
        }
        Geom::Path::const_iterator curve_it1 = path_it->begin();
        Geom::Path::const_iterator curve_it2 = ++(path_it->begin());
        Geom::Path::const_iterator curve_endit = path_it->end_default();

        SPCurve *curve_n = new SPCurve();
        Geom::Point point_at0(0, 0);
        Geom::Point point_at1(0, 0);
        Geom::Point point_at2(0, 0);
        Geom::Point point_at3(0, 0);
        Geom::D2<Geom::SBasis> sbasis_in;
        Geom::D2<Geom::SBasis> sbasis_out;
        Geom::CubicBezier const *cubic = NULL;
        curve_n->moveto(curve_it1->initialPoint());
        if (path_it->closed()) {
          const Geom::Curve &closingline = path_it->back_closed(); 
          // the closing line segment is always of type 
          // Geom::LineSegment.
          if (are_near(closingline.initialPoint(), closingline.finalPoint())) {
            // closingline.isDegenerate() did not work, because it only checks for
            // *exact* zero length, which goes wrong for relative coordinates and
            // rounding errors...
            // the closing line segment has zero-length. So stop before that one!
            curve_endit = path_it->end_open();
          }
        }
        while (curve_it1 != curve_endit) {
            SPCurve *in = new SPCurve();
            in->moveto(curve_it1->initialPoint());
            in->lineto(curve_it1->finalPoint());
            cubic = dynamic_cast<Geom::CubicBezier const *>(&*curve_it1);
            point_at0 = in->first_segment()->initialPoint();
            point_at3 = in->first_segment()->finalPoint();
            sbasis_in = in->first_segment()->toSBasis();
            if (cubic) {
                if ((apply_no_weight && apply_with_weight) ||
                    (apply_no_weight && Geom::are_near((*cubic)[1], point_at0)) ||
                    (apply_with_weight && !Geom::are_near((*cubic)[1], point_at0)))
                {
                    if (isNodePointSelected(point_at0) || !only_selected) {
                        point_at1 = sbasis_in.valueAt(weight_ammount);
                        if (weight_ammount != NO_POWER) {
                            point_at1 =
                                Geom::Point(point_at1[X] + HANDLE_CUBIC_GAP, point_at1[Y] + HANDLE_CUBIC_GAP);
                        }
                    } else {
                        point_at1 = (*cubic)[1];
                    }
                } else {
                    point_at1 = (*cubic)[1];
                }
                if ((apply_no_weight && apply_with_weight) ||
                    (apply_no_weight && Geom::are_near((*cubic)[2], point_at3)) ||
                    (apply_with_weight && !Geom::are_near((*cubic)[2], point_at3)))
                {
                    if (isNodePointSelected(point_at3) || !only_selected) {
                        point_at2 = sbasis_in.valueAt(1 - weight_ammount);
                        if (weight_ammount != NO_POWER) {
                            point_at2 =
                                Geom::Point(point_at2[X] + HANDLE_CUBIC_GAP, point_at2[Y] + HANDLE_CUBIC_GAP);
                        }
                    } else {
                        point_at2 = (*cubic)[2];
                    }
                } else {
                    point_at2 = (*cubic)[2];
                }
            } else {
                if ((apply_no_weight && apply_with_weight) || 
                    (apply_no_weight && weight_ammount == NO_POWER) ||
                    (apply_with_weight && weight_ammount != NO_POWER))
                {
                    if (isNodePointSelected(point_at0) || !only_selected) {
                        point_at1 = sbasis_in.valueAt(weight_ammount);
                        point_at1 =
                            Geom::Point(point_at1[X] + HANDLE_CUBIC_GAP, point_at1[Y] + HANDLE_CUBIC_GAP);
                    } else {
                        point_at1 = in->first_segment()->initialPoint();
                    }
                    if (isNodePointSelected(point_at3) || !only_selected) {
                        point_at2 = sbasis_in.valueAt(1 - weight_ammount);
                        point_at2 =
                            Geom::Point(point_at2[X] + HANDLE_CUBIC_GAP, point_at2[Y] + HANDLE_CUBIC_GAP);
                    } else {
                        point_at2 = in->first_segment()->finalPoint();
                    }
                } else {
                    point_at1 = in->first_segment()->initialPoint();
                    point_at2 = in->first_segment()->finalPoint();
                }
            }
            in->reset();
            delete in;
            curve_n->curveto(point_at1, point_at2, point_at3);
            ++curve_it1;
            ++curve_it2;
        }
        if (path_it->closed()) {
            curve_n->move_endpoints(path_it->begin()->initialPoint(),
                                   path_it->begin()->initialPoint());
        } else {
            curve_n->move_endpoints(path_it->begin()->initialPoint(), point_at3);
        }
        if (path_it->closed()) {
            curve_n->closepath_current();
        }
        curve->append(curve_n, false);
        curve_n->reset();
        delete curve_n;
    }
}

}; //namespace LivePathEffect
}; /* namespace Inkscape */

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
