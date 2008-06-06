#ifndef SEEN_LIBNR_N_ART_BPATH_2GEOM_H
#define SEEN_LIBNR_N_ART_BPATH_2GEOM_H

/** \file
 * Contains functions to convert from NArtBpath to 2geom's Path
 *
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <vector>
#include <2geom/path.h>
#include <libnr/n-art-bpath.h>

std::vector<Geom::Path>  BPath_to_2GeomPath (NArtBpath const *bpath);
NArtBpath *              BPath_from_2GeomPath (std::vector<Geom::Path> const & path);


#endif /* !SEEN_LIBNR_N_ART_BPATH_2GEOM_H */

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
