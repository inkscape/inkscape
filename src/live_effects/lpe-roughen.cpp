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
#include <util/units.h>
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
      unit(_("Unit"), _("Unit"), "unit", &wr, this),
      method(_("Method"), _("Division method"), "method", DMConverter, &wr,
             this, DM_SEGMENTS),
      maxSegmentSize(_("Max. segment size"), _("Max. segment size"),
                     "maxSegmentSize", &wr, this, 10.),
      segments(_("Number of segments"), _("Number of segments"), "segments",
               &wr, this, 2),
      displaceX(_("Max. displacement in X"), _("Max. displacement in X"),
                "displaceX", &wr, this, 10.),
      displaceY(_("Max. displacement in Y"), _("Max. displacement in Y"),
                "displaceY", &wr, this, 10.),
      shiftNodes(_("Shift nodes"), _("Shift nodes"), "shiftNodes", &wr, this,
                 true),
      shiftNodeHandles(_("Shift node handles"), _("Shift node handles"),
                       "shiftNodeHandles", &wr, this, true)
{
    registerParameter(&unit);
    registerParameter(&method);
    registerParameter(&maxSegmentSize);
    registerParameter(&segments);
    registerParameter(&displaceX);
    registerParameter(&displaceY);
    registerParameter(&shiftNodes);
    registerParameter(&shiftNodeHandles);
    displaceX.param_set_range(0., Geom::infinity());
    displaceY.param_set_range(0., Geom::infinity());
    maxSegmentSize.param_set_range(0., Geom::infinity());
    maxSegmentSize.param_set_increments(1, 1);
    maxSegmentSize.param_set_digits(1);
    segments.param_set_range(1, Geom::infinity());
    segments.param_set_increments(1, 1);
    segments.param_set_digits(0);
}

LPERoughen::~LPERoughen() {}

void LPERoughen::doBeforeEffect(SPLPEItem const *lpeitem)
{
    displaceX.resetRandomizer();
    displaceY.resetRandomizer();
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
            if (param->param_key == "unit") {
                Gtk::Label *unitLabel = Gtk::manage(new Gtk::Label(
                                                        Glib::ustring(_("<b>Roughen unit</b>")), Gtk::ALIGN_START));
                unitLabel->set_use_markup(true);
                vbox->pack_start(*unitLabel, false, false, 2);
                vbox->pack_start(*Gtk::manage(new Gtk::HSeparator()),
                                 Gtk::PACK_EXPAND_WIDGET);
            }
            if (param->param_key == "method") {
                Gtk::Label *methodLabel = Gtk::manage(new Gtk::Label(
                        Glib::ustring(_("<b>Add nodes</b> Subdivide each segment")),
                        Gtk::ALIGN_START));
                methodLabel->set_use_markup(true);
                vbox->pack_start(*methodLabel, false, false, 2);
                vbox->pack_start(*Gtk::manage(new Gtk::HSeparator()),
                                 Gtk::PACK_EXPAND_WIDGET);
            }
            if (param->param_key == "displaceX") {
                Gtk::Label *displaceXLabel = Gtk::manage(new Gtk::Label(
                                                 Glib::ustring(_("<b>Jitter nodes</b> Move nodes/handles")),
                                                 Gtk::ALIGN_START));
                displaceXLabel->set_use_markup(true);
                vbox->pack_start(*displaceXLabel, false, false, 2);
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

double LPERoughen::sign(double randNumber)
{
    if (rand() % 100 < 49) {
        randNumber *= -1.;
    }
    return randNumber;
}

Geom::Point LPERoughen::randomize()
{
    Inkscape::Util::Unit const *doc_units = SP_ACTIVE_DESKTOP->namedview->doc_units;
    double displaceXParsed = Inkscape::Util::Quantity::convert(
                                 displaceX, unit.get_abbreviation(), doc_units->abbr);
    double displaceYParsed = Inkscape::Util::Quantity::convert(
                                 displaceY, unit.get_abbreviation(), doc_units->abbr);

    Geom::Point output = Geom::Point(sign(displaceXParsed), sign(displaceYParsed));
    return output;
}

void LPERoughen::doEffect(SPCurve *curve)
{
    Geom::PathVector const original_pathv =
        pathv_to_linear_and_cubic_beziers(curve->get_pathvector());
    curve->reset();
    Inkscape::Util::Unit const *doc_units = SP_ACTIVE_DESKTOP->namedview->doc_units;
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
        if (shiftNodes) {
            initialMove = randomize();
        }
        Geom::Point initialPoint = curve_it1->initialPoint() + initialMove;
        nCurve->moveto(initialPoint);
        Geom::Point A0(0, 0);
        Geom::Point A1(0, 0);
        Geom::Point A2(0, 0);
        Geom::Point A3(0, 0);
        while (curve_it1 != curve_endit) {
            Geom::CubicBezier const *cubic = NULL;
            A0 = curve_it1->initialPoint();
            A1 = curve_it1->initialPoint();
            A2 = curve_it1->finalPoint();
            A3 = curve_it1->finalPoint();
            cubic = dynamic_cast<Geom::CubicBezier const *>(&*curve_it1);
            if (cubic) {
                A1 = (*cubic)[1];
                if (shiftNodes) {
                    A1 = (*cubic)[1] + initialMove;
                }
                A2 = (*cubic)[2];
                nCurve->curveto(A1, A2, A3);
            } else {
                nCurve->lineto(A3);
            }
            double length = Inkscape::Util::Quantity::convert(
                                curve_it1->length(0.001), doc_units->abbr, unit.get_abbreviation());
            std::size_t splits = 0;
            if (method == DM_SEGMENTS) {
                splits = segments;
            } else {
                splits = ceil(length / maxSegmentSize);
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
    Geom::Point A1(0, 0);
    Geom::Point A2(0, 0);
    Geom::Point A3(0, 0);
    Geom::Point B1(0, 0);
    Geom::Point B2(0, 0);
    Geom::Point B3(0, 0);
    if (shiftNodes) {
        A3 = randomize();
        B3 = randomize();
    }
    if (shiftNodeHandles) {
        A1 = randomize();
        A2 = randomize();
        B1 = randomize();
        B2 = randomize();
    } else {
        A2 = A3;
        B1 = A3;
        B2 = B3;
    }
    if (cubic) {
        std::pair<Geom::CubicBezier, Geom::CubicBezier> div = cubic->subdivide(t);
        std::vector<Geom::Point> seg1 = div.first.points(),
                                 seg2 = div.second.points();
        out->moveto(seg1[0]);
        out->curveto(seg1[1] + A1, seg1[2] + A2, seg1[3] + A3);
        out->curveto(seg2[1] + B1, seg2[2], seg2[3]);
    } else if (shiftNodeHandles) {
        out->moveto(A->initialPoint());
        out->curveto(A->pointAt(t / 3) + A1, A->pointAt((t / 3) * 2) + A2,
                     A->pointAt(t) + A3);
        out->curveto(A->pointAt(t + (t / 3)) + B1, A->pointAt(t + ((t / 3) * 2)),
                     A->finalPoint());
    } else {
        out->moveto(A->initialPoint());
        out->lineto(A->pointAt(t) + A3);
        out->lineto(A->finalPoint());
    }
    return out;
}

SPCurve *LPERoughen::jitter(const Geom::Curve *A)
{
    SPCurve *out = new SPCurve();
    Geom::CubicBezier const *cubic = dynamic_cast<Geom::CubicBezier const *>(&*A);
    Geom::Point A1(0, 0);
    Geom::Point A2(0, 0);
    Geom::Point A3(0, 0);
    if (shiftNodes) {
        A3 = randomize();
    }
    if (shiftNodeHandles) {
        A1 = randomize();
        A2 = randomize();
    } else {
        A2 = A3;
    }
    if (cubic) {
        out->moveto((*cubic)[0]);
        out->curveto((*cubic)[1] + A1, (*cubic)[2] + A2, (*cubic)[3] + A3);
    } else if (shiftNodeHandles) {
        out->moveto(A->initialPoint());
        out->curveto(A->pointAt(0.3333) + A1, A->pointAt(0.6666) + A2,
                     A->finalPoint() + A3);
    } else {
        out->moveto(A->initialPoint());
        out->lineto(A->finalPoint() + A3);
    }
    return out;
}

Geom::Point LPERoughen::tpoint(Geom::Point A, Geom::Point B, double t)
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
