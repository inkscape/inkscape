/**
 *  \file guide-snapper.cpp
 *  \brief Snapping things to guides.
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   Carl Hetherington <inkscape@carlh.net>
 *
 * Copyright (C) 1999-2002 Authors 
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "libnr/nr-values.h"
#include "libnr/nr-point-fns.h"
#include "sp-namedview.h"
#include "sp-guide.h"

Inkscape::GuideSnapper::GuideSnapper(SPNamedView const *nv, NR::Coord const d) : LineSnapper(nv, d)
{

}

Inkscape::GuideSnapper::LineList Inkscape::GuideSnapper::_getSnapLines(NR::Point const &p) const
{
    LineList s;
    
    for (GSList const *l = _named_view->guides; l != NULL; l = l->next) {
        SPGuide const *g = SP_GUIDE(l->data);

        /* We assume here that guides are horizontal or vertical */
        if (g->normal == component_vectors[NR::X]) {
            s.push_back(std::make_pair(NR::X, g->position));
        } else {
            s.push_back(std::make_pair(NR::Y, g->position));
        }
    }

    return s;
}

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
