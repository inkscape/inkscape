#ifndef INKSCAPE_HELPER_GEOM_CURVES_H
#define INKSCAPE_HELPER_GEOM_CURVES_H

/**
 * Specific curve type functions for Inkscape, not provided my lib2geom.
 *
 * Author:
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *
 * Copyright (C) 2008 Johan Engelen
 *
 * Released under GNU GPL
 */

#include <2geom/hvlinesegment.h>

inline bool is_straight_curve(Geom::Curve const & c) {
    if( dynamic_cast<Geom::LineSegment const*>(&c) ||
        dynamic_cast<Geom::HLineSegment const*>(&c) ||
        dynamic_cast<Geom::VLineSegment const*>(&c) )
    {
        return true;
    } else {
        return false;
    }
}

#endif  // INKSCAPE_HELPER_GEOM_CURVES_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
