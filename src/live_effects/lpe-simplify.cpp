/*
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtkmm.h>

#include "live_effects/lpe-simplify.h"
#include "display/curve.h"
#include "live_effects/parameter/parameter.h"
#include <glibmm/i18n.h>
#include "helper/geom.h"
#include "livarot/Path.h"
#include "splivarot.h"
#include <2geom/svg-path-parser.h>
#include "desktop.h"
#include "inkscape.h"
#include "svg/svg.h"
#include "ui/tools/node-tool.h"
#include <2geom/d2.h>
#include <2geom/generic-rect.h>
#include <2geom/interval.h>
#include "ui/icon-names.h"
#include "util/units.h"

namespace Inkscape {
namespace LivePathEffect {

LPESimplify::LPESimplify(LivePathEffectObject *lpeobject)
    : Effect(lpeobject),
      steps(_("Steps:"),_("Change number of simplify steps "), "steps", &wr, this,1),
      threshold(_("Roughly threshold:"), _("Roughly threshold:"), "threshold", &wr, this, 0.003),
      smooth_angles(_("Smooth angles:"), _("Max degree difference on handles to preform a smooth"), "smooth_angles", &wr, this, 20.),
      helper_size(_("Helper size:"), _("Helper size"), "helper_size", &wr, this, 5),
      simplify_individual_paths(_("Paths separately"), _("Simplifying paths (separately)"), "simplify_individual_paths", &wr, this, false,
                              "", INKSCAPE_ICON("on"), INKSCAPE_ICON("off")),
      simplify_just_coalesce(_("Just coalesce"), _("Simplify just coalesce"), "simplify_just_coalesce", &wr, this, false,
                           "", INKSCAPE_ICON("on"), INKSCAPE_ICON("off"))
{
    registerParameter(&steps);
    registerParameter(&threshold);
    registerParameter(&smooth_angles);
    registerParameter(&helper_size);
    registerParameter(&simplify_individual_paths);
    registerParameter(&simplify_just_coalesce);

    threshold.param_set_range(0.0001, Geom::infinity());
    threshold.param_set_increments(0.0001, 0.0001);
    threshold.param_set_digits(6);

    steps.param_set_range(0, 100);
    steps.param_set_increments(1, 1);
    steps.param_set_digits(0);

    smooth_angles.param_set_range(0.0, 365.0);
    smooth_angles.param_set_increments(10, 10);
    smooth_angles.param_set_digits(2);

    helper_size.param_set_range(0.0, 999.0);
    helper_size.param_set_increments(5, 5);
    helper_size.param_set_digits(2);

    radius_helper_nodes = 6.0;
}

LPESimplify::~LPESimplify() {}

void
LPESimplify::doBeforeEffect (SPLPEItem const* lpeitem)
{
    if(!hp.empty()) {
        hp.clear();
    }
    bbox = SP_ITEM(lpeitem)->visualBounds();
    SPLPEItem * item = const_cast<SPLPEItem*>(lpeitem);
    radius_helper_nodes = helper_size;
    item->apply_to_clippath(item);
    item->apply_to_mask(item);
}

Gtk::Widget *
LPESimplify::newWidget()
{
    // use manage here, because after deletion of Effect object, others might still be pointing to this widget.
    Gtk::VBox * vbox = Gtk::manage( new Gtk::VBox(Effect::newWidget()) );
    vbox->set_border_width(5);
    vbox->set_homogeneous(false);
    vbox->set_spacing(2);
    std::vector<Parameter *>::iterator it = param_vector.begin();
    Gtk::HBox * buttons = Gtk::manage(new Gtk::HBox(true,0));
    while (it != param_vector.end()) {
        if ((*it)->widget_is_visible) {
            Parameter * param = *it;
            Gtk::Widget * widg = dynamic_cast<Gtk::Widget *>(param->param_newWidget());
            if (param->param_key == "simplify_individual_paths" ||
                    param->param_key == "simplify_just_coalesce") {
                Glib::ustring * tip = param->param_getTooltip();
                if (widg) {
                    buttons->pack_start(*widg, true, true, 2);
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
                    Gtk::HBox * horizontal_box = dynamic_cast<Gtk::HBox *>(widg);
                    std::vector< Gtk::Widget* > child_list = horizontal_box->get_children();
                    Gtk::Entry* entry_widg = dynamic_cast<Gtk::Entry *>(child_list[1]);
                    entry_widg->set_width_chars(8);
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
    vbox->pack_start(*buttons,true, true, 2);
    return dynamic_cast<Gtk::Widget *>(vbox);
}

void
LPESimplify::doEffect(SPCurve *curve)
{
    Geom::PathVector const original_pathv = pathv_to_linear_and_cubic_beziers(curve->get_pathvector());
    gdouble size  = Geom::L2(bbox->dimensions());
    //size /= Geom::Affine(0,0,0,0,0,0).descrim();
    Path* pathliv = Path_for_pathvector(original_pathv);
    if(simplify_individual_paths) {
        size = Geom::L2(Geom::bounds_fast(original_pathv)->dimensions());
    }
    for (int unsigned i = 0; i < steps; i++) {
        if ( simplify_just_coalesce ) {
            pathliv->Coalesce(threshold * size);
        } else {
            pathliv->ConvertEvenLines(threshold * size);
            pathliv->Simplify(threshold * size);
        }
    }
    Geom::PathVector result = Geom::parse_svg_path(pathliv->svg_dump_path());
    generateHelperPathAndSmooth(result);
    curve->set_pathvector(result);
    SPDesktop* desktop = SP_ACTIVE_DESKTOP;
    if(desktop && INK_IS_NODE_TOOL(desktop->event_context)) {
        Inkscape::UI::Tools::NodeTool *nt = static_cast<Inkscape::UI::Tools::NodeTool*>(desktop->event_context);
        nt->update_helperpath();
    }
}

void
LPESimplify::generateHelperPathAndSmooth(Geom::PathVector &result)
{
    if(steps < 1) {
        return;
    }
    Geom::PathVector tmp_path;
    Geom::CubicBezier const *cubic = NULL;
    for (Geom::PathVector::iterator path_it = result.begin(); path_it != result.end(); ++path_it) {
        //Si está vacío...
        if (path_it->empty()) {
            continue;
        }
        //Itreadores
        Geom::Path::const_iterator curve_it1 = path_it->begin(); // incoming curve
        Geom::Path::const_iterator curve_it2 = ++(path_it->begin());// outgoing curve
        Geom::Path::const_iterator curve_endit = path_it->end_default(); // this determines when the loop has to stop
        SPCurve *nCurve = new SPCurve();
        if (path_it->closed()) {
            // if the path is closed, maybe we have to stop a bit earlier because the
            // closing line segment has zerolength.
            const Geom::Curve &closingline =
                path_it->back_closed(); // the closing line segment is always of type
            // Geom::LineSegment.
            if (are_near(closingline.initialPoint(), closingline.finalPoint())) {
                // closingline.isDegenerate() did not work, because it only checks for
                // *exact* zero length, which goes wrong for relative coordinates and
                // rounding errors...
                // the closing line segment has zero-length. So stop before that one!
                curve_endit = path_it->end_open();
            }
        }
        if(helper_size > 0) {
            drawNode(curve_it1->initialPoint());
        }
        nCurve->moveto(curve_it1->initialPoint());
        Geom::Point start = Geom::Point(0,0);
        while (curve_it1 != curve_endit) {
            cubic = dynamic_cast<Geom::CubicBezier const *>(&*curve_it1);
            Geom::Point point_at1 = curve_it1->initialPoint();
            Geom::Point point_at2 = curve_it1->finalPoint();
            Geom::Point point_at3 = curve_it1->finalPoint();
            Geom::Point point_at4 = curve_it1->finalPoint();
            if (cubic) {
                point_at1 = (*cubic)[1];
                point_at2 = (*cubic)[2];
            }
            if(start == Geom::Point(0,0)) {
                start = point_at1;
            }

            if(path_it->closed() && curve_it2 == curve_endit) {
                point_at4 = start;
            }
            if(curve_it2 != curve_endit) {
                cubic = dynamic_cast<Geom::CubicBezier const *>(&*curve_it2);
                if (cubic) {
                    point_at4 = (*cubic)[1];
                }
            }
            Geom::Ray ray1(point_at2, point_at3);
            Geom::Ray ray2(point_at3, point_at4);
            double angle1 = Geom::rad_to_deg(ray1.angle());
            double angle2 = Geom::rad_to_deg(ray2.angle());
            if((smooth_angles  >= angle2 - angle1) && !are_near(point_at4,point_at3) && !are_near(point_at2,point_at3)) {
                double dist = Geom::distance(point_at2,point_at3);
                Geom::Angle angleFixed = ray2.angle();
                angleFixed -= Geom::Angle::from_degrees(180.0);
                point_at2 =  Geom::Point::polar(angleFixed,dist) + point_at3;
            }
            nCurve->curveto(point_at1, point_at2, curve_it1->finalPoint());
            cubic = dynamic_cast<Geom::CubicBezier const *>(nCurve->last_segment());
            if (cubic) {
                point_at1 = (*cubic)[1];
                point_at2 = (*cubic)[2];
                if(helper_size > 0) {
                    if(!are_near((*cubic)[0],(*cubic)[1])) {
                        drawHandle((*cubic)[1]);
                        drawHandleLine((*cubic)[0],(*cubic)[1]);
                    }
                    if(!are_near((*cubic)[3],(*cubic)[2])) {
                        drawHandle((*cubic)[2]);
                        drawHandleLine((*cubic)[3],(*cubic)[2]);
                    }
                }
            }
            if(helper_size > 0) {
                drawNode(curve_it1->finalPoint());
            }
            ++curve_it1;
            ++curve_it2;
        }
        if (path_it->closed()) {
            nCurve->closepath_current();
        }
        tmp_path.push_back(nCurve->get_pathvector()[0]);
        nCurve->reset();
        delete nCurve;
    }
    result = tmp_path;
}

void
LPESimplify::drawNode(Geom::Point p)
{
    double r = radius_helper_nodes;
    char const * svgd;
    svgd = "M 0.55,0.5 A 0.05,0.05 0 0 1 0.5,0.55 0.05,0.05 0 0 1 0.45,0.5 0.05,0.05 0 0 1 0.5,0.45 0.05,0.05 0 0 1 0.55,0.5 Z M 0,0 1,0 1,1 0,1 Z";
    Geom::PathVector pathv = sp_svg_read_pathv(svgd);
    pathv *= Geom::Affine(r,0,0,r,0,0);
    pathv += p - Geom::Point(0.5*r,0.5*r);
    hp.push_back(pathv[0]);
    hp.push_back(pathv[1]);
}

void
LPESimplify::drawHandle(Geom::Point p)
{
    double r = radius_helper_nodes;
    char const * svgd;
    svgd = "M 0.7,0.35 A 0.35,0.35 0 0 1 0.35,0.7 0.35,0.35 0 0 1 0,0.35 0.35,0.35 0 0 1 0.35,0 0.35,0.35 0 0 1 0.7,0.35 Z";
    Geom::PathVector pathv = sp_svg_read_pathv(svgd);
    pathv *= Geom::Affine(r,0,0,r,0,0);
    pathv += p - Geom::Point(0.35*r,0.35*r);
    hp.push_back(pathv[0]);
}


void
LPESimplify::drawHandleLine(Geom::Point p,Geom::Point p2)
{
    Geom::Path path;
    path.start( p );
    double diameter = radius_helper_nodes;
    if(helper_size > 0 && Geom::distance(p,p2) > (diameter * 0.35)) {
        Geom::Ray ray2(p, p2);
        p2 =  p2 - Geom::Point::polar(ray2.angle(),(diameter * 0.35));
    }
    path.appendNew<Geom::LineSegment>( p2 );
    hp.push_back(path);
}

void
LPESimplify::addCanvasIndicators(SPLPEItem const */*lpeitem*/, std::vector<Geom::PathVector> &hp_vec)
{
    hp_vec.push_back(hp);
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
