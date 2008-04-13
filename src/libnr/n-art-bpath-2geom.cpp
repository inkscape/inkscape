#define SEEN_LIBNR_N_ART_BPATH_2GEOM_CPP

/** \file
 * Contains functions to convert from NArtBpath to 2geom's Path
 *
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
 

#include "libnr/n-art-bpath-2geom.h"
#include "svg/svg.h"
#include <glib.h>
#include <2geom/path.h>
#include <2geom/svg-path.h>
#include <2geom/svg-path-parser.h>
#include <2geom/sbasis-to-bezier.h>

//##########################################################

#include <iostream>
#include <sstream>
#include <string>
#include <boost/format.hpp>

static void curve_to_svgd(std::ostream & f, Geom::Curve const* c) {
    if(Geom::LineSegment const *line_segment = dynamic_cast<Geom::LineSegment const  *>(c)) {
        f << boost::format("L %g,%g ") % (*line_segment)[1][0] % (*line_segment)[1][1];
    }
    else if(Geom::QuadraticBezier const *quadratic_bezier = dynamic_cast<Geom::QuadraticBezier const  *>(c)) {
        f << boost::format("Q %g,%g %g,%g ") % (*quadratic_bezier)[1][0] % (*quadratic_bezier)[1][0]  
                % (*quadratic_bezier)[2][0] % (*quadratic_bezier)[2][1];
    }
    else if(Geom::CubicBezier const *cubic_bezier = dynamic_cast<Geom::CubicBezier const  *>(c)) {
        f << boost::format("C %g,%g %g,%g %g,%g ") 
                % (*cubic_bezier)[1][0] % (*cubic_bezier)[1][1] 
                % (*cubic_bezier)[2][0] % (*cubic_bezier)[2][1] 
                % (*cubic_bezier)[3][0] % (*cubic_bezier)[3][1];
    }
//    else if(Geom::SVGEllipticalArc const *svg_elliptical_arc = dynamic_cast<Geom::SVGEllipticalArc *>(c)) {
//        //get at the innards and spit them out as svgd
//    }
    else { 
        //this case handles sbasis as well as all other curve types
        Geom::Path sbasis_path = Geom::path_from_sbasis(c->toSBasis(), 0.1);

        //recurse to convert the new path resulting from the sbasis to svgd
        for(Geom::Path::iterator iter = sbasis_path.begin(); iter != sbasis_path.end(); ++iter) {
            curve_to_svgd(f, &(*iter));
        }
    }
}

static void write_svgd(std::ostream & f, Geom::Path const &p) {
    if(f == NULL) {
        f << "ERRRRRRORRRRR";
        return;
    }

    f << boost::format("M %g,%g ") % p.initialPoint()[0] % p.initialPoint()[1];
    
    for(Geom::Path::const_iterator iter(p.begin()), end(p.end()); iter != end; ++iter) {
        curve_to_svgd(f, &(*iter));
    }
    if(p.closed())
        f << "Z ";
}

static void write_svgd(std::ostream & f, std::vector<Geom::Path> const &p) {
    std::vector<Geom::Path>::const_iterator it(p.begin());
    for(; it != p.end(); it++) {
        write_svgd(f, *it);
    }
}

std::vector<Geom::Path>  
SVGD_to_2GeomPath (char const *svgd)
{
    std::vector<Geom::Path> pathv;

    try {
        pathv = Geom::parse_svg_path(svgd);
    }
    catch (std::runtime_error e) {
        g_warning("SVGPathParseError: %s", e.what());
    }

    return pathv;
}


std::vector<Geom::Path>
BPath_to_2GeomPath(NArtBpath const * bpath)
{
    std::vector<Geom::Path> pathv;
    char *svgpath = sp_svg_write_path(bpath);
    if (!svgpath) {
        g_warning("BPath_to_2GeomPath - empty path returned");
        return pathv;
    }
    pathv = SVGD_to_2GeomPath(svgpath);
    g_free(svgpath);
    return pathv;
}

char *
SVGD_from_2GeomPath(std::vector<Geom::Path> const & path)
{
    std::ostringstream ss;
    write_svgd(ss, path);
    ss.flush();
    std::string str = ss.str();
    char * svgd = g_strdup(str.c_str());
    return svgd;
}

NArtBpath *
BPath_from_2GeomPath(std::vector<Geom::Path> const & path)
{
    char * svgd = SVGD_from_2GeomPath(path);
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
