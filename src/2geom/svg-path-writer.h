/** @file
 * @brief Path sink which writes an SVG-compatible command string
 *//*
 * Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 * 
 * Copyright 2014 Authors
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
 */

#ifndef LIB2GEOM_SEEN_SVG_PATH_WRITER_H
#define LIB2GEOM_SEEN_SVG_PATH_WRITER_H

#include <2geom/path-sink.h>
#include <sstream>

namespace Geom {

/** @brief Serialize paths to SVG path data strings.
 * You can access the generated string by calling the str() method.
 * @ingroup Paths
 */
class SVGPathWriter
    : public PathSink
{
public:
    SVGPathWriter();
    ~SVGPathWriter() {}

    void moveTo(Point const &p);
    void lineTo(Point const &p);
    void quadTo(Point const &c, Point const &p);
    void curveTo(Point const &c0, Point const &c1, Point const &p);
    void arcTo(double rx, double ry, double angle,
               bool large_arc, bool sweep, Point const &p);
    void closePath();
    void flush();

    /// Clear any path data written so far.
    void clear();

    /** @brief Set output precision.
     * When the parameter is negative, the path writer enters a verbatim mode
     * which preserves all values exactly. */
    void setPrecision(int prec);

    /** @brief Enable or disable length optimization.
     * 
     * When set to true, the path writer will optimize the generated path data
     * for minimum length. However, this will make the data less readable,
     * because spaces between commands and coordinates will be omitted where
     * unnecessary for correct parsing.
     *
     * When set to false, the string will be a straightforward, partially redundant
     * representation of the passed commands, optimized for readability.
     * Commands and coordinates will always be separated by spaces and the command
     * symbol will not be omitted for multiple consecutive commands of the same type.
     *
     * Length optimization is turned off by default. */
    void setOptimize(bool opt) { _optimize = opt; }

    /** @brief Enable or disable the use of V, H, T and S commands where possible.
     * Shorthands are turned on by default. */
    void setUseShorthands(bool use) { _use_shorthands = use; }

    /// Retrieve the generated path data string.
    std::string str() const { return _s.str(); }

private:
    void _setCommand(char cmd);
    std::string _formatCoord(Coord par);

    std::ostringstream _s, _ns;
    std::vector<Coord> _current_pars;
    Point _subpath_start;
    Point _current;
    Point _quad_tangent;
    Point _cubic_tangent;
    Coord _epsilon;
    int _precision;
    bool _optimize;
    bool _use_shorthands;
    char _command;
};

std::string write_svg_path(PathVector const &pv, int prec = -1, bool optimize = false, bool shorthands = true);

} // namespace Geom

#endif // LIB2GEOM_SEEN_SVG_PATH_WRITER_H
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
