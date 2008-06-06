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


std::vector<Geom::Path>
BPath_to_2GeomPath(NArtBpath const * bpath)
{
    std::vector<Geom::Path> pathv;
    char *svgpath = sp_svg_write_path(bpath);
    if (!svgpath) {
        g_warning("BPath_to_2GeomPath - empty path returned");
        return pathv;
    }
    pathv = sp_svg_read_pathv(svgpath);
    g_free(svgpath);
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
