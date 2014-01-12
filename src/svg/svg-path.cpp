#define __SP_SVG_PARSE_C__
/*
   svg-path.c: Parse SVG path element data into bezier path.

   Copyright (C) 2000 Eazel, Inc.
   Copyright (C) 2000 Lauris Kaplinski
   Copyright (C) 2001 Ximian, Inc.
   Copyright (C) 2008 Johan Engelen

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public
   License along with this program; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Authors:
     Johan Engelen
     (old nartbpath code that has been deleted: Raph Levien <raph@artofcode.com>)
     (old nartbpath code that has been deleted: Lauris Kaplinski <lauris@ximian.com>)
*/

#include <cstring>
#include <string>
#include <cassert>
#include <glib.h> // g_assert()

#include "svg/svg.h"
#include "svg/path-string.h"

#include <2geom/pathvector.h>
#include <2geom/path.h>
#include <2geom/curves.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/path-sink.h>
#include <2geom/svg-path-parser.h>
#include <2geom/exception.h>
#include <2geom/angle.h>

/*
 * Parses the path in str. When an error is found in the pathstring, this method
 * returns a truncated path up to where the error was found in the pathstring.
 * Returns an empty PathVector when str==NULL
 */
Geom::PathVector sp_svg_read_pathv(char const * str)
{
    Geom::PathVector pathv;
    if (!str)
        return pathv;  // return empty pathvector when str == NULL


    typedef std::back_insert_iterator<Geom::PathVector> Inserter;
    Inserter iter(pathv);
    Geom::PathIteratorSink<Inserter> generator(iter);

    try {
        Geom::parse_svg_path(str, generator);
    }
    catch (Geom::SVGPathParseError &e) {
        generator.flush();
        // This warning is extremely annoying when testing
        //g_warning("Malformed SVG path, truncated path up to where error was found.\n Input path=\"%s\"\n Parsed path=\"%s\"", str, sp_svg_write_path(pathv));
    }

    return pathv;
}

static void sp_svg_write_curve(Inkscape::SVG::PathString & str, Geom::Curve const * c) {
    if(Geom::LineSegment const *line_segment = dynamic_cast<Geom::LineSegment const  *>(c)) {
        // don't serialize stitch segments
        if (!dynamic_cast<Geom::Path::StitchSegment const *>(c)) {
            str.lineTo( (*line_segment)[1][0], (*line_segment)[1][1] );
        }
    }
    else if(Geom::QuadraticBezier const *quadratic_bezier = dynamic_cast<Geom::QuadraticBezier const  *>(c)) {
        str.quadTo( (*quadratic_bezier)[1][0], (*quadratic_bezier)[1][1],
                    (*quadratic_bezier)[2][0], (*quadratic_bezier)[2][1] );
    }
    else if(Geom::CubicBezier const *cubic_bezier = dynamic_cast<Geom::CubicBezier const  *>(c)) {
        str.curveTo( (*cubic_bezier)[1][0], (*cubic_bezier)[1][1],
                     (*cubic_bezier)[2][0], (*cubic_bezier)[2][1],
                     (*cubic_bezier)[3][0], (*cubic_bezier)[3][1] );
    }
    else if(Geom::SVGEllipticalArc const *svg_elliptical_arc = dynamic_cast<Geom::SVGEllipticalArc const *>(c)) {
        str.arcTo( svg_elliptical_arc->ray(Geom::X), svg_elliptical_arc->ray(Geom::Y),
                   Geom::rad_to_deg(svg_elliptical_arc->rotationAngle()),
                   svg_elliptical_arc->largeArc(), svg_elliptical_arc->sweep(),
                   svg_elliptical_arc->finalPoint() );
    }
    else if(Geom::HLineSegment const *hline_segment = dynamic_cast<Geom::HLineSegment const  *>(c)) {
        str.horizontalLineTo( hline_segment->finalPoint()[0] );
    }
    else if(Geom::VLineSegment const *vline_segment = dynamic_cast<Geom::VLineSegment const  *>(c)) {
        str.verticalLineTo( vline_segment->finalPoint()[1] );
    } else { 
        //this case handles sbasis as well as all other curve types
        Geom::Path sbasis_path = Geom::cubicbezierpath_from_sbasis(c->toSBasis(), 0.1);

        //recurse to convert the new path resulting from the sbasis to svgd
        for(Geom::Path::iterator iter = sbasis_path.begin(); iter != sbasis_path.end(); ++iter) {
            sp_svg_write_curve(str, &(*iter));
        }
    }
}

static void sp_svg_write_path(Inkscape::SVG::PathString & str, Geom::Path const & p) {
    str.moveTo( p.initialPoint()[0], p.initialPoint()[1] );

    for(Geom::Path::const_iterator cit = p.begin(); cit != p.end_open(); ++cit) {
        sp_svg_write_curve(str, &(*cit));
    }

    if (p.closed()) {
        str.closePath();
    }
}

gchar * sp_svg_write_path(Geom::PathVector const &p) {
    Inkscape::SVG::PathString str;

    for(Geom::PathVector::const_iterator pit = p.begin(); pit != p.end(); ++pit) {
        sp_svg_write_path(str, *pit);
    }

    return g_strdup(str.c_str());
}

gchar * sp_svg_write_path(Geom::Path const &p) {
    Inkscape::SVG::PathString str;

    sp_svg_write_path(str, p);

    return g_strdup(str.c_str());
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
