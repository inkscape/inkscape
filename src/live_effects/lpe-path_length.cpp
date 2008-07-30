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

#include "2geom/sbasis-geometric.h"

namespace Inkscape {
namespace LivePathEffect {

LPEPathLength::LPEPathLength(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    info_text(_("Info text"), _("Parameter for text creation"), "info_text", &wr, this, "")
{
    /* uncomment the next line if you want the original path to be
       permanently displayed as a helperpath while the item is selected */
    //show_orig_path = true;

    registerParameter(dynamic_cast<Parameter *>(&info_text));
}

LPEPathLength::~LPEPathLength()
{

}

Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPEPathLength::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    using namespace Geom;

    gchar *arc_length = g_strdup_printf("%.2f", Geom::length(pwd2_in));
    info_text.param_setValue(arc_length);
    g_free(arc_length);

    info_text.setPosAndAnchor(pwd2_in, 0.5, 20);

    Piecewise<D2<SBasis> > A = integral(pwd2_in);
    Point c;
    double area;
    if (centroid(pwd2_in, c, area)) {
        g_print ("Area is zero\n");
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
