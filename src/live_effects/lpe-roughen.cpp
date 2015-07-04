/**
 * @file
 * Roughen LPE implementation. Creates roughen paths.
 */
/* Authors:
 *   Jabier Arraiza Cenoz <jabier.arraiza@marker.es>
 *
 * Thanks to all people involved specialy to Josh Andler for the idea and to the
 * original extensions authors.
 *
 * Copyright (C) 2014 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtkmm.h>
#include "desktop.h"
#include "live_effects/lpe-roughen.h"
#include "display/curve.h"
#include "live_effects/parameter/parameter.h"
#include "helper/geom.h"
#include <glibmm/i18n.h>
#include <cmath>

namespace Inkscape {
namespace LivePathEffect {

static const Util::EnumData<DivisionMethod> DivisionMethodData[DM_END] = {
    { DM_SEGMENTS, N_("By number of segments"), "segments" },
    { DM_SIZE, N_("By max. segment size"), "size" }
};
static const Util::EnumDataConverter<DivisionMethod>
DMConverter(DivisionMethodData, DM_END);

LPERoughen::LPERoughen(LivePathEffectObject *lpeobject)
    : Effect(lpeobject),
      // initialise your parameters here:
      method(_("Method"), _("Division method"), "method", DMConverter, &wr,
             this, DM_SEGMENTS),
      max_segment_size(_("Max. segment size"), _("Max. segment size"),
                     "max_segment_size", &wr, this, 10.),
      segments(_("Number of segments"), _("Number of segments"), "segments",
               &wr, this, 2),
      displace_x(_("Max. displacement in X"), _("Max. displacement in X"),
                "displace_x", &wr, this, 10.),
      displace_y(_("Max. displacement in Y"), _("Max. displacement in Y"),
                "displace_y", &wr, this, 10.),
      global_randomize(_("Global randomize"), _("Global randomize"),
                      "global_randomize", &wr, this, 1.),
      shift_nodes(_("Shift nodes"), _("Shift nodes"), "shift_nodes", &wr, this,
                 true),
      shift_node_handles(_("Shift node handles"), _("Shift node handles"),
                       "shift_node_handles", &wr, this, true)
{
    registerParameter(&method);
    registerParameter(&max_segment_size);
    registerParameter(&segments);
    registerParameter(&displace_x);
    registerParameter(&displace_y);
    registerParameter(&global_randomize);
    registerParameter(&shift_nodes);
    registerParameter(&shift_node_handles);
    displace_x.param_set_range(0., Geom::infinity());
    displace_y.param_set_range(0., Geom::infinity());
    global_randomize.param_set_range(0., Geom::infinity());
    max_segment_size.param_set_range(0., Geom::infinity());
    max_segment_size.param_set_increments(1, 1);
    max_segment_size.param_set_digits(1);
    segments.param_set_range(1, Geom::infinity());
    segments.param_set_increments(1, 1);
    segments.param_set_digits(0);
}

LPERoughen::~LPERoughen() {}

void LPERoughen::doBeforeEffect(SPLPEItem const *lpeitem)
{
    displace_x.resetRandomizer();
    displace_y.resetRandomizer();
    global_randomize.resetRandomizer();
    srand(1);
    SPLPEItem * item = const_cast<SPLPEItem*>(lpeitem);
    item->apply_to_clippath(item);
    item->apply_to_mask(item);
}

Gtk::Widget *LPERoughen::newWidget()
{
    Gtk::VBox *vbox = Gtk::manage(new Gtk::VBox(Effect::newWidget()));
    vbox->set_border_width(5);
    vbox->set_homogeneous(false);
    vbox->set_spacing(2);
    std::vector<Parameter *>::iterator it = param_vector.begin();
    while (it != param_vector.end()) {
        if ((*it)->widget_is_visible) {
            Parameter *param = *it;
            Gtk::Widget *widg = dynamic_cast<Gtk::Widget *>(param->param_newWidget());
            if (param->param_key == "method") {
                Gtk::Label *method_label = Gtk::manage(new Gtk::Label(
                        Glib::ustring(_("<b>Add nodes</b> Subdivide each segment")),
                        Gtk::ALIGN_START));
                method_label->set_use_markup(true);
                vbox->pack_start(*method_label, false, false, 2);
                vbox->pack_start(*Gtk::manage(new Gtk::HSeparator()),
                                 Gtk::PACK_EXPAND_WIDGET);
            }
            if (param->param_key == "displace_x") {
                Gtk::Label *displace_x_label = Gtk::manage(new Gtk::Label(
                                                 Glib::ustring(_("<b>Jitter nodes</b> Move nodes/handles")),
                                                 Gtk::ALIGN_START));
                displace_x_label->set_use_markup(true);
                vbox->pack_start(*displace_x_label, false, false, 2);
                vbox->pack_start(*Gtk::manage(new Gtk::HSeparator()),
                                 Gtk::PACK_EXPAND_WIDGET);
            }
            if (param->param_key == "global_randomize") {
                Gtk::Label *global_rand = Gtk::manage(new Gtk::Label(
                                                 Glib::ustring(_("<b>Extra roughen</b> Add a extra layer of rough")),
                                                 Gtk::ALIGN_START));
                global_rand->set_use_markup(true);
                vbox->pack_start(*global_rand, false, false, 2);
                vbox->pack_start(*Gtk::manage(new Gtk::HSeparator()),
                                 Gtk::PACK_EXPAND_WIDGET);
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

double LPERoughen::sign(double random_number)
{
    if (rand() % 100 < 49) {
        random_number *= -1.;
    }
    return random_number;
}

Geom::Point LPERoughen::randomize()
{
    double displace_x_parsed = displace_x * global_randomize;
    double displace_y_parsed = displace_y * global_randomize;

    Geom::Point output = Geom::Point(sign(displace_x_parsed), sign(displace_y_parsed));
    return output;
}

void LPERoughen::doEffect(SPCurve *curve)
{
    Geom::PathVector const original_pathv =
        pathv_to_linear_and_cubic_beziers(curve->get_pathvector());
    curve->reset();
    for (Geom::PathVector::const_iterator path_it = original_pathv.begin();
            path_it != original_pathv.end(); ++path_it) {
        if (path_it->empty())
            continue;

        Geom::Path::const_iterator curve_it1 = path_it->begin();
        Geom::Path::const_iterator curve_it2 = ++(path_it->begin());
        Geom::Path::const_iterator curve_endit = path_it->end_default();
        SPCurve *nCurve = new SPCurve();
        if (path_it->closed()) {
            const Geom::Curve &closingline =
                path_it->back_closed();
            if (are_near(closingline.initialPoint(), closingline.finalPoint())) {
                curve_endit = path_it->end_open();
            }
        }
        Geom::Point initialMove(0, 0);
        if (shift_nodes) {
            initialMove = randomize();
        }
        Geom::Point initialPoint = curve_it1->initialPoint() + initialMove;
        nCurve->moveto(initialPoint);
        Geom::Point point0(0, 0);
        Geom::Point point1(0, 0);
        Geom::Point point2(0, 0);
        Geom::Point point3(0, 0);
        bool first = true;
        while (curve_it1 != curve_endit) {
            Geom::CubicBezier const *cubic = NULL;
            point0 = curve_it1->initialPoint();
            point1 = curve_it1->initialPoint();
            point2 = curve_it1->finalPoint();
            point3 = curve_it1->finalPoint();
            cubic = dynamic_cast<Geom::CubicBezier const *>(&*curve_it1);
            if (cubic) {
                point1 = (*cubic)[1];
                if (shift_nodes && first) {
                    point1 = (*cubic)[1] + initialMove;
                }
                point2 = (*cubic)[2];
                nCurve->curveto(point1, point2, point3);
            } else {
                nCurve->lineto(point3);
            }
            double length = curve_it1->length(0.001);
            std::size_t splits = 0;
            if (method == DM_SEGMENTS) {
                splits = segments;
            } else {
                splits = ceil(length / max_segment_size);
            }
            for (unsigned int t = splits; t >= 1; t--) {
                if (t == 1 && splits != 1) {
                    continue;
                }
                const SPCurve *tmp;
                if (splits == 1) {
                    tmp = jitter(nCurve->last_segment());
                } else {
                    tmp = addNodesAndJitter(nCurve->last_segment(), 1. / t);
                }
                if (nCurve->get_segment_count() > 1) {
                    nCurve->backspace();
                    nCurve->append_continuous(tmp, 0.001);
                } else {
                    nCurve = tmp->copy();
                }
                delete tmp;
            }
            ++curve_it1;
            if(curve_it2 != curve_endit) {
                ++curve_it2;
            }
            first = false;
        }
        if (path_it->closed()) {
            nCurve->closepath_current();
        }
        curve->append(nCurve, false);
        nCurve->reset();
        delete nCurve;
    }
}

SPCurve *LPERoughen::addNodesAndJitter(const Geom::Curve *A, double t)
{
    SPCurve *out = new SPCurve();
    Geom::CubicBezier const *cubic = dynamic_cast<Geom::CubicBezier const *>(&*A);
    Geom::Point point1(0, 0);
    Geom::Point point2(0, 0);
    Geom::Point point3(0, 0);
    Geom::Point point_b1(0, 0);
    Geom::Point point_b2(0, 0);
    Geom::Point point_b3(0, 0);
    if (shift_nodes) {
        point3 = randomize();
        point_b3 = randomize();
    }
    if (shift_node_handles) {
        point1 = randomize();
        point2 = randomize();
        point_b1 = randomize();
        point_b2 = randomize();
    } else {
        point2 = point3;
        point_b1 = point3;
        point_b2 = point_b3;
    }
    if (cubic) {
        std::pair<Geom::CubicBezier, Geom::CubicBezier> div = cubic->subdivide(t);
        std::vector<Geom::Point> seg1 = div.first.controlPoints(),
                                 seg2 = div.second.controlPoints();
        out->moveto(seg1[0]);
        out->curveto(seg1[1] + point1, seg1[2] + point2, seg1[3] + point3);
        out->curveto(seg2[1] + point_b1, seg2[2], seg2[3]);
    } else if (shift_node_handles) {
        out->moveto(A->initialPoint());
        out->curveto(A->pointAt(t / 3) + point1, A->pointAt((t / 3) * 2) + point2,
                     A->pointAt(t) + point3);
        out->curveto(A->pointAt(t + (t / 3)) + point_b1, A->pointAt(t + ((t / 3) * 2)),
                     A->finalPoint());
    } else {
        out->moveto(A->initialPoint());
        out->lineto(A->pointAt(t) + point3);
        out->lineto(A->finalPoint());
    }
    return out;
}

SPCurve *LPERoughen::jitter(const Geom::Curve *A)
{
    SPCurve *out = new SPCurve();
    Geom::CubicBezier const *cubic = dynamic_cast<Geom::CubicBezier const *>(&*A);
    Geom::Point point1(0, 0);
    Geom::Point point2(0, 0);
    Geom::Point point3(0, 0);
    if (shift_nodes) {
        point3 = randomize();
    }
    if (shift_node_handles) {
        point1 = randomize();
        point2 = randomize();
    } else {
        point2 = point3;
    }
    if (cubic) {
        out->moveto((*cubic)[0]);
        out->curveto((*cubic)[1] + point1, (*cubic)[2] + point2, (*cubic)[3] + point3);
    } else if (shift_node_handles) {
        out->moveto(A->initialPoint());
        out->curveto(A->pointAt(0.3333) + point1, A->pointAt(0.6666) + point2,
                     A->finalPoint() + point3);
    } else {
        out->moveto(A->initialPoint());
        out->lineto(A->finalPoint() + point3);
    }
    return out;
}

Geom::Point LPERoughen::tPoint(Geom::Point A, Geom::Point B, double t)
{
    using Geom::X;
    using Geom::Y;
    return Geom::Point(A[X] + t * (B[X] - A[X]), A[Y] + t * (B[Y] - A[Y]));
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
