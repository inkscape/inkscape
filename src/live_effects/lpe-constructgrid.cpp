/** \file
 * LPE Construct Grid implementation
 */
/*
 * Authors:
 *   Johan Engelen
*
* Copyright (C) Johan Engelen 2008 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/i18n.h>

#include "live_effects/lpe-constructgrid.h"

#include <2geom/path.h>
#include <2geom/transforms.h>

namespace Inkscape {
namespace LivePathEffect {

using namespace Geom;

LPEConstructGrid::LPEConstructGrid(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    nr_x(_("Size _X:"), _("The size of the grid in X direction."), "nr_x", &wr, this, 5),
    nr_y(_("Size _Y:"), _("The size of the grid in Y direction."), "nr_y", &wr, this, 5)
{
    registerParameter( dynamic_cast<Parameter *>(&nr_x) );
    registerParameter( dynamic_cast<Parameter *>(&nr_y) );

    nr_x.param_make_integer();
    nr_y.param_make_integer();
    nr_x.param_set_range(1, 1e10);
    nr_y.param_set_range(1, 1e10);
}

LPEConstructGrid::~LPEConstructGrid()
{

}

Geom::PathVector
LPEConstructGrid::doEffect_path (Geom::PathVector const & path_in)
{
  // Check that the path has at least 3 nodes (i.e. 2 segments), more nodes are ignored
    if (path_in[0].size() >= 2) {
        // read the first 3 nodes:
        Geom::Path::const_iterator it ( path_in[0].begin() );
        Geom::Point first_p  = (*it++).initialPoint();
        Geom::Point origin   = (*it++).initialPoint();
        Geom::Point second_p = (*it++).initialPoint();
        // make first_p and second_p be the construction *vectors* of the grid:
        first_p  -= origin;
        second_p -= origin;
        Geom::Translate first_translation( first_p );
        Geom::Translate second_translation( second_p );

        // create the gridpaths of the two directions
        Geom::Path first_path( origin );
        first_path.appendNew<LineSegment>( origin + first_p*nr_y );
        Geom::Path second_path( origin );
        second_path.appendNew<LineSegment>( origin + second_p*nr_x );

        // use the gridpaths and set them in the correct grid
        Geom::PathVector path_out;
        path_out.push_back(first_path);
        for (int ix = 0; ix < nr_x; ix++) {
            path_out.push_back(path_out.back() * second_translation );
        }
        path_out.push_back(second_path);
        for (int iy = 0; iy < nr_y; iy++) {
            path_out.push_back(path_out.back() * first_translation );
        }

        return path_out;
    } else {
        return path_in;
    }
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
