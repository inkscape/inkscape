/*
 * Authors:
 *   Jabier Arraiza Cenoz
*
* Copyright (C) Jabier Arraiza Cenoz 2014 <jabier.arraiza@marker.es>
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtkmm.h>
#include <glibmm/i18n.h>
#include "live_effects/lpe-show_handles.h"
#include "live_effects/parameter/parameter.h"
#include <2geom/sbasis-to-bezier.h>
#include <2geom/svg-path-parser.h>
#include "helper/geom.h"
#include "desktop-style.h"
#include "style.h"
#include "svg/svg.h"

namespace Inkscape {
namespace LivePathEffect {

LPEShowHandles::LPEShowHandles(LivePathEffectObject *lpeobject)
    : Effect(lpeobject),
      nodes(_("Show nodes"), _("Show nodes"), "nodes", &wr, this, true),
      handles(_("Show handles"), _("Show handles"), "handles", &wr, this, true),
      originalPath(_("Show path"), _("Show path"), "originalPath", &wr, this, true),
      scaleNodesAndHandles(_("Scale nodes and handles"), _("Scale nodes and handles"), "scaleNodesAndHandles", &wr, this, 10)
{
    registerParameter(dynamic_cast<Parameter *>(&nodes));
    registerParameter(dynamic_cast<Parameter *>(&handles));
    registerParameter(dynamic_cast<Parameter *>(&originalPath));
    registerParameter(dynamic_cast<Parameter *>(&scaleNodesAndHandles));
    scaleNodesAndHandles.param_set_range(0, 500.);
    scaleNodesAndHandles.param_set_increments(1, 1);
    scaleNodesAndHandles.param_set_digits(2);
    strokeWidth = 1.0;
}

bool LPEShowHandles::alertsOff = false;

/**
 * Sets default styles to element
 * this permanently remove.some styles of the element
 */

void LPEShowHandles::doOnApply(SPLPEItem const* lpeitem)
{
    if(!alertsOff) {
        char *msg = _("The \"show handles\" path effect will remove any custom style on the object you are applying it to. If this is not what you want, click Cancel.");
        Gtk::MessageDialog dialog(msg, false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL, true);
        gint response = dialog.run();
        alertsOff = true;
        if(response == GTK_RESPONSE_CANCEL) {
            SPLPEItem* item = const_cast<SPLPEItem*>(lpeitem);
            item->removeCurrentPathEffect(false);
            return;
        }
    }
    SPLPEItem* item = const_cast<SPLPEItem*>(lpeitem);
    SPCSSAttr *css = sp_repr_css_attr_new ();
    sp_repr_css_set_property (css, "stroke", "black");
    sp_repr_css_set_property (css, "stroke-width", "1");
    sp_repr_css_set_property (css, "stroke-linecap", "butt");
    sp_repr_css_set_property(css, "fill", "none");

    sp_desktop_apply_css_recursive(item, css, true);
    sp_repr_css_attr_unref (css);
}

void LPEShowHandles::doBeforeEffect (SPLPEItem const* lpeitem)
{
    SPItem const* item = SP_ITEM(lpeitem);
    strokeWidth = item->style->stroke_width.computed;
}

std::vector<Geom::Path> LPEShowHandles::doEffect_path (std::vector<Geom::Path> const & path_in)
{
    std::vector<Geom::Path> path_out;
    Geom::PathVector const original_pathv = pathv_to_linear_and_cubic_beziers(path_in);
    if(originalPath) {
        for (unsigned int i=0; i < path_in.size(); i++) {
            path_out.push_back(path_in[i]);
        }
    }
    if(!outlinepath.empty()) {
        outlinepath.clear();
    }
    generateHelperPath(original_pathv);
    for (unsigned int i=0; i < outlinepath.size(); i++) {
        path_out.push_back(outlinepath[i]);
    }
    return path_out;
}

void
LPEShowHandles::generateHelperPath(Geom::PathVector result)
{
    if(!handles && !nodes) {
        return;
    }

    Geom::CubicBezier const *cubic = NULL;
    for (Geom::PathVector::iterator path_it = result.begin(); path_it != result.end(); ++path_it) {
        //Si está vacío...
        if (path_it->empty()) {
            continue;
        }
        //Itreadores
        Geom::Path::const_iterator curve_it1 = path_it->begin(); // incoming curve
        Geom::Path::const_iterator curve_it2 = ++(path_it->begin()); // outgoing curve
        Geom::Path::const_iterator curve_endit = path_it->end_default(); // this determines when the loop has to stop

        if (path_it->closed()) {
            // if the path is closed, maybe we have to stop a bit earlier because the
            // closing line segment has zerolength.
            const Geom::Curve &closingline = path_it->back_closed(); // the closing line segment is always of type
            // Geom::LineSegment.
            if (are_near(closingline.initialPoint(), closingline.finalPoint())) {
                // closingline.isDegenerate() did not work, because it only checks for
                // *exact* zero length, which goes wrong for relative coordinates and
                // rounding errors...
                // the closing line segment has zero-length. So stop before that one!
                curve_endit = path_it->end_open();
            }
        }
        if(nodes) {
            drawNode(curve_it1->initialPoint());
        }
        while (curve_it1 != curve_endit) {
            cubic = dynamic_cast<Geom::CubicBezier const *>(&*curve_it1);
            if (cubic) {
                if(handles) {
                    if(!are_near((*cubic)[0],(*cubic)[1])){
                        drawHandle((*cubic)[1]);
                        drawHandleLine((*cubic)[0],(*cubic)[1]);
                    }
                    if(!are_near((*cubic)[3],(*cubic)[2])){
                        drawHandle((*cubic)[2]);
                        drawHandleLine((*cubic)[3],(*cubic)[2]);
                    }
                }
            }
            if(nodes) {
                drawNode(curve_it1->finalPoint());
            }
            ++curve_it1;
            if(curve_it2 != curve_endit){
                ++curve_it2;
            }
        }
    }
}

void
LPEShowHandles::drawNode(Geom::Point p)
{
    if(strokeWidth * scaleNodesAndHandles > 0.0) {
        double diameter = strokeWidth * scaleNodesAndHandles;
        char const * svgd;
        svgd = "M 0.55,0.5 A 0.05,0.05 0 0 1 0.5,0.55 0.05,0.05 0 0 1 0.45,0.5 0.05,0.05 0 0 1 0.5,0.45 0.05,0.05 0 0 1 0.55,0.5 Z M 0,0 1,0 1,1 0,1 Z";
        Geom::PathVector pathv = sp_svg_read_pathv(svgd);
        pathv *= Geom::Affine(diameter,0,0,diameter,0,0);
        pathv += p - Geom::Point(diameter/2,diameter/2);
        outlinepath.push_back(pathv[0]);
        outlinepath.push_back(pathv[1]);
    }
}

void
LPEShowHandles::drawHandle(Geom::Point p)
{
    if(strokeWidth * scaleNodesAndHandles > 0.0) {
        double diameter = strokeWidth * scaleNodesAndHandles;
        char const * svgd;
        svgd = "M 0.7,0.35 A 0.35,0.35 0 0 1 0.35,0.7 0.35,0.35 0 0 1 0,0.35 0.35,0.35 0 0 1 0.35,0 0.35,0.35 0 0 1 0.7,0.35 Z";
        Geom::PathVector pathv = sp_svg_read_pathv(svgd);
        pathv *= Geom::Affine(diameter,0,0,diameter,0,0);
        pathv += p - Geom::Point(diameter * 0.35,diameter * 0.35);
        outlinepath.push_back(pathv[0]);
    }
}


void
LPEShowHandles::drawHandleLine(Geom::Point p,Geom::Point p2)
{
    Geom::Path path;
    double diameter = strokeWidth * scaleNodesAndHandles;
    if(diameter > 0.0 && Geom::distance(p,p2) > (diameter * 0.35)){
        Geom::Ray ray2(p, p2);
        p2 =  p2 - Geom::Point::polar(ray2.angle(),(diameter * 0.35));
    }
    path.start( p );
    path.appendNew<Geom::LineSegment>( p2 );
    outlinepath.push_back(path);
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
