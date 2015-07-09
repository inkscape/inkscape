/**
 * @file
 * @brief  Path sink for Cairo contexts
 *//*
 * Copyright 2014 Krzysztof Kosiński
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 *
 */

#ifndef LIB2GEOM_SEEN_CAIRO_PATH_SINK_H
#define LIB2GEOM_SEEN_CAIRO_PATH_SINK_H

#include <2geom/path-sink.h>
#include <cairo.h>

namespace Geom {


/** @brief Output paths to a Cairo drawing context
 *
 * This class converts from 2Geom path representation to the Cairo representation.
 * Use it to simplify visualizing the results of 2Geom operations with the Cairo library,
 * for example:
 * @code
 *   CairoPathSink sink(cr);
 *   sink.feed(pv);
 *   cairo_stroke(cr);
 * @endcode
 *
 * Currently the flush method is a no-op, but this is not guaranteed
 * to hold forever.
 */
class CairoPathSink
    : public PathSink
{
public:
    CairoPathSink(cairo_t *cr);

    void moveTo(Point const &p);
    void lineTo(Point const &p);
    void curveTo(Point const &c0, Point const &c1, Point const &p);
    void quadTo(Point const &c, Point const &p);
    void arcTo(Coord rx, Coord ry, Coord angle,
               bool large_arc, bool sweep, Point const &p);
    void closePath();
    void flush();

private:
    cairo_t *_cr;
    Point _current_point;
};

}

#endif // !LIB2GEOM_SEEN_CAIRO_PATH_SINK_H
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
