#define INKSCAPE_LPE_PATH_LENGTH_CPP
/** \file
 * LPE <path_length> implementation.
 */
/*
 * Authors:
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *   Johan Engelen
 *
 * Copyright (C) 2007-2008 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-path_length.h"
#include "sp-metrics.h"

#include "2geom/sbasis-geometric.h"

namespace Inkscape {
namespace LivePathEffect {

LPEPathLength::LPEPathLength(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    scale(_("Scale"), _("Scaling factor"), "scale", &wr, this, 1.0),
    info_text(this),
    unit(_("Unit"), _("Unit"), "unit", &wr, this)
{
    registerParameter(dynamic_cast<Parameter *>(&scale));
    registerParameter(dynamic_cast<Parameter *>(&info_text));
    registerParameter(dynamic_cast<Parameter *>(&unit));
}

LPEPathLength::~LPEPathLength()
{

}

void
LPEPathLength::hideCanvasText() {
    // this is only used in sp-lpe-item.cpp to hide the canvas text when the effect is invisible
    info_text.param_setValue("");
}

Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPEPathLength::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    using namespace Geom;

    /* convert the measured length to the correct unit ... */
    double lengthval = Geom::length(pwd2_in) * scale;
    gboolean success = sp_convert_distance(&lengthval, &sp_unit_get_by_id(SP_UNIT_PX), unit);

    /* ... set it as the canvas text ... */
    gchar *arc_length = g_strdup_printf("%.2f %s", lengthval, success ? unit.get_abbreviation() : "px");
    info_text.param_setValue(arc_length);
    g_free(arc_length);

    info_text.setPosAndAnchor(pwd2_in, 0.5, 10);

    // TODO: how can we compute the area (such that cw turns don't count negative)?
    //       should we display the area here, too, or write a new LPE for this?
    Piecewise<D2<SBasis> > A = integral(pwd2_in);
    Point c;
    double area;
    if (centroid(pwd2_in, c, area)) {
        //g_print ("Area is zero\n");
    }
    //g_print ("Area: %f\n", area);

    return pwd2_in;
}

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
