#ifdef AXONOM

/**
 *  \file axonomgrid-snapper.cpp
 *  \brief Snapping things to axonometric grids.
 *
 * Author:
 *   Johan Engelen <johan@shouraizou.nl>
 *
 * Copyright (C) 2006 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-namedview.h"
#include "inkscape.h"
#include "desktop.h"
#include "display/canvas-axonomgrid.h"

/**
 * \return x rounded to the nearest multiple of c1 plus c0.
 *
 * \note
 * If c1==0 (and c0 is finite), then returns +/-inf.  This makes grid spacing of zero
 * mean "ignore the grid in this dimention".  We're currently discussing "good" semantics
 * for guide/grid snapping.
 */

/* FIXME: move this somewhere else, perhaps */
static 

Inkscape::AxonomGridSnapper::AxonomGridSnapper(SPNamedView const *nv, NR::Coord const d) : LineSnapper(nv, d)
{

}

Inkscape::LineSnapper::LineList 
Inkscape::AxonomGridSnapper::_getSnapLines(NR::Point const &p) const
{
    double round_to_nearest_multiple_plus(double x, double const c1, double const c0)
    {
        return floor((x - c0) / c1 + .5) * c1 + c0;
    }    
    
    LineList s;

    if ( NULL == _named_view ) {
        return s;
    }

    SPCAxonomGrid *griditem = NULL;
    for (GSList *l = _named_view->gridviews; l != NULL; l = l->next) {
        // FIXME : this is a hack since there is only one view for now
        //                 but when we'll handle multiple views, snapping should
        //                 must be rethought and maybe only the current view
        //                 should give back it's SHOWN lines to snap to
        //                 For now, the last SPCGrid in _named_view->gridviews will be used.
        griditem = SP_CAXONOMGRID(l->data);
    }

    g_assert(griditem != NULL);

    for (unsigned int i = 0; i < 2; ++i) {

        /* This is to make sure we snap to only visible grid lines */
        double scaled_spacing = griditem->sw[i]; // this is spacing of visible lines if screen pixels

        // convert screen pixels to px
        // FIXME: after we switch to snapping dist in screen pixels, this will be unnecessary
        if (SP_ACTIVE_DESKTOP) {
            scaled_spacing /= SP_ACTIVE_DESKTOP->current_zoom();
        }

        NR::Coord const rounded = round_to_nearest_multiple_plus(p[i],
                                                                 scaled_spacing,
                                                                 _named_view->gridorigin[i]);

        s.push_back(std::make_pair(NR::Dim2(i), rounded));
    }

    return s;
}



#endif           
