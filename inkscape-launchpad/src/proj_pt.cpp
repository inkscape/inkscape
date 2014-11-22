/**
 * 3x4 transformation matrix to map points from projective 3-space into the projective plane
 *
 * Authors:
 *   Maximilian Albert <Anhalter42@gmx.de>
 *
 * Copyright (C) 2007  Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>

#include "proj_pt.h"
#include "svg/stringstream.h"

namespace Proj {

Pt2::Pt2(const char *coord_str) {
    if (!coord_str) {
        pt[0] = 0.0;
        pt[1] = 0.0;
        pt[2] = 1.0;
        g_warning ("Coordinate string is empty. Creating default Pt2\n");
        return;
    }
    char **coords = g_strsplit(coord_str, ":", 0);
    if (coords[0] == NULL || coords[1] == NULL || coords[2] == NULL) {
        g_strfreev (coords);
        g_warning ("Malformed coordinate string.\n");
        return;
    }

    pt[0] = g_ascii_strtod(coords[0], NULL);
    pt[1] = g_ascii_strtod(coords[1], NULL);
    pt[2] = g_ascii_strtod(coords[2], NULL);
    g_strfreev (coords);
}

void
Pt2::normalize() {
    if (fabs(pt[2]) < 1E-6 || pt[2] == 1.0)
        return;
    pt[0] /= pt[2];
    pt[1] /= pt[2];
    pt[2] = 1.0;
}

Geom::Point
Pt2::affine() {
  if (fabs(pt[2]) < epsilon) {
    return Geom::Point (Geom::infinity(), Geom::infinity());
  }
  return Geom::Point (pt[0]/pt[2], pt[1]/pt[2]);
}

char *
Pt2::coord_string() {
    Inkscape::SVGOStringStream os;
    os << pt[0] << " : "
       << pt[1] << " : "
       << pt[2];
    return g_strdup(os.str().c_str());
}

Pt3::Pt3(const char *coord_str) {
    if (!coord_str) {
        pt[0] = 0.0;
        pt[1] = 0.0;
        pt[2] = 0.0;
        pt[3] = 1.0;
        g_warning ("Coordinate string is empty. Creating default Pt2\n");
        return;
    }
    char **coords = g_strsplit(coord_str, ":", 0);
    if (coords[0] == NULL || coords[1] == NULL ||
        coords[2] == NULL || coords[3] == NULL) {
        g_strfreev (coords);
        g_warning ("Malformed coordinate string.\n");
        return;
    }

    pt[0] = g_ascii_strtod(coords[0], NULL);
    pt[1] = g_ascii_strtod(coords[1], NULL);
    pt[2] = g_ascii_strtod(coords[2], NULL);
    pt[3] = g_ascii_strtod(coords[3], NULL);
}

void
Pt3::normalize() {
    if (fabs(pt[3]) < 1E-6 || pt[3] == 1.0)
        return;
    pt[0] /= pt[3];
    pt[1] /= pt[3];
    pt[2] /= pt[3];
    pt[3] = 1.0;
}

char *
Pt3::coord_string() {
    Inkscape::SVGOStringStream os;
    os << pt[0] << " : "
       << pt[1] << " : "
       << pt[2] << " : "
       << pt[3];
    return g_strdup(os.str().c_str());
}

} // namespace Proj

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8 :
