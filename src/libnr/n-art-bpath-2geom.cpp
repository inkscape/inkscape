#define SEEN_LIBNR_N_ART_BPATH_2GEOM_CPP

/** \file
 * Contains functions to convert from NArtBpath to 2geom's Path
 *
 * Copyright (C) Johan Engelen 2007-2008 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
 
#include "libnr/n-art-bpath-2geom.h"

#include "svg/svg.h"
#include <glib.h>
#include <2geom/path.h>
#include <2geom/svg-path.h>
#include <2geom/svg-path-parser.h>
#include <typeinfo>

std::vector<Geom::Path>
BPath_to_2GeomPath(NArtBpath const * bpath)
{
    std::vector<Geom::Path> pathv;
    if (!bpath) {
        return pathv;
    }

    NArtBpath const *bp = bpath;   // points to element within bpath
    Geom::Path * current = NULL;   // points to current path
    while (bp->code != NR_END) {
        if ( current &&
             ( (bp->code == NR_MOVETO) || (bp->code == NR_MOVETO_OPEN) )
            )
        {   // about to start a new path, correct the current path: nartbpath manually adds the closing line segment so erase it for closed path.
            if (current->closed() && !current->empty()) {
                // but only remove this last segment if it is a *linesegment*:
                if ( dynamic_cast<Geom::LineSegment const *>(&current->back()) ) {
                    current->erase_last();
                }
            }
        }

        switch(bp->code) {
            case NR_MOVETO:
                pathv.push_back( Geom::Path() );  // for some reason Geom::Path(Point) does not work...
                current = &pathv.back();
                current->start( Geom::Point(bp->x3, bp->y3) );
                current->close(true);
            break;

            case NR_MOVETO_OPEN:
                pathv.push_back( Geom::Path() );  // for some reason Geom::Path(Point) does not work...
                current = &pathv.back();
                current->start( Geom::Point(bp->x3, bp->y3) );
                current->close(false);
            break;

            case NR_LINETO:
                current->appendNew<Geom::LineSegment>( Geom::Point(bp->x3, bp->y3) );
            break;

            case NR_CURVETO:
                current->appendNew<Geom::CubicBezier> ( Geom::Point(bp->x1, bp->y1), Geom::Point(bp->x2, bp->y2), Geom::Point(bp->x3, bp->y3) );
            break;

            case NR_END:
                g_error("BPath_to_2GeomPath: logical error");
            break;
        }
        ++bp;
    }
    if ( current && current->closed() && !current->empty() ) {
        // correct the current path: nartbpath manually adds the closing line segment so erase it for closed path.
        // but only remove this last segment if it is a *linesegment*:
        if ( dynamic_cast<Geom::LineSegment const *>(&current->back()) ) {
            current->erase_last();
        }
    }

    return pathv;
}

NArtBpath *
BPath_from_2GeomPath(std::vector<Geom::Path> const & path)
{
    char * svgd = sp_svg_write_path(path);
    NArtBpath *bpath = sp_svg_read_path(svgd);
    g_free(svgd);
    return bpath;
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
