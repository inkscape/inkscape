#define SEEN_LIBNR_N_ART_BPATH_2GEOM_CPP

/** \file
 * Contains functions to convert from NArtBpath to 2geom's Path
 *
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
 

#include "live_effects/n-art-bpath-2geom.h"
#include "svg/svg.h"
#include <glib.h>
#include <2geom/path.h>
#include <2geom/svg-path.h>
#include <2geom/svg-path-parser.h>
#include <2geom/sbasis-to-bezier.h>

#define LPE_USE_2GEOM_CONVERSION

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
        Geom::Path sbasis_path = path_from_sbasis(c->sbasis(), 0.1);

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

//##########################################################
#ifndef LPE_USE_2GEOM_CONVERSION

static
Geom::Point point(double *nums, int ix) {
    return Geom::Point(nums[ix], nums[ix + 1]);
}

using namespace Geom;

class OldPathBuilder {
public:
    OldPathBuilder(double const &c = Geom_EPSILON) : _current_path(NULL) {
        _continuity_tollerance = c;
    }

    void startPathRel(Point const &p0) { startPath(p0 + _current_point); }
    void startPath(Point const &p0) {
        _pathset.push_back(Geom::Path());
        _current_path = &_pathset.back();
        _initial_point = _current_point = p0;
    }

    void pushLineRel(Point const &p0) { pushLine(p0 + _current_point); }
    void pushLine(Point const &p1) {
        if (!_current_path) startPath(_current_point);
        _current_path->appendNew<LineSegment>(p1);
        _current_point = p1;
    }

    void pushLineRel(Point const &p0, Point const &p1) { pushLine(p0 + _current_point, p1 + _current_point); }
    void pushLine(Point const &p0, Point const &p1) {
        if(p0 != _current_point) startPath(p0);
        pushLine(p1);
    }

    void pushHorizontalRel(Coord y) { pushHorizontal(y + _current_point[1]); }
    void pushHorizontal(Coord y) {
        if (!_current_path) startPath(_current_point);
        pushLine(Point(_current_point[0], y));
    }

    void pushVerticalRel(Coord x) { pushVertical(x + _current_point[0]); }
    void pushVertical(Coord x) {
        if (!_current_path) startPath(_current_point);
        pushLine(Point(x, _current_point[1]));
    }

    void pushQuadraticRel(Point const &p1, Point const &p2) { pushQuadratic(p1 + _current_point, p2 + _current_point); }
    void pushQuadratic(Point const &p1, Point const &p2) {
        if (!_current_path) startPath(_current_point);
        _current_path->appendNew<QuadraticBezier>(p1, p2);
        _current_point = p2;
    }

    void pushQuadraticRel(Point const &p0, Point const &p1, Point const &p2) {
        pushQuadratic(p0 + _current_point, p1 + _current_point, p2 + _current_point);
    }
    void pushQuadratic(Point const &p0, Point const &p1, Point const &p2) {
        if(p0 != _current_point) startPath(p0);
        pushQuadratic(p1, p2);
    }

    void pushCubicRel(Point const &p1, Point const &p2, Point const &p3) {
        pushCubic(p1 + _current_point, p2 + _current_point, p3 + _current_point);
    }
    void pushCubic(Point const &p1, Point const &p2, Point const &p3) {
        if (!_current_path) startPath(_current_point);
        _current_path->appendNew<CubicBezier>(p1, p2, p3);
        _current_point = p3;
    }

    void pushCubicRel(Point const &p0, Point const &p1, Point const &p2, Point const &p3) {
        pushCubic(p0 + _current_point, p1 + _current_point, p2 + _current_point, p3 + _current_point);
    }
    void pushCubic(Point const &p0, Point const &p1, Point const &p2, Point const &p3) {
        if(p0 != _current_point) startPath(p0);
        pushCubic(p1, p2, p3);
    }
/*
    void pushEllipseRel(Point const &radii, double rotation, bool large, bool sweep, Point const &end) {
        pushEllipse(radii, rotation, large, sweep, end + _current_point);
    }
    void pushEllipse(Point const &radii, double rotation, bool large, bool sweep, Point const &end) {
        if (!_current_path) startPath(_current_point);
        _current_path->append(SVGEllipticalArc(_current_point, radii[0], radii[1], rotation, large, sweep, end));
        _current_point = end;
    }

    void pushEllipseRel(Point const &initial, Point const &radii, double rotation, bool large, bool sweep, Point const &end) {
        pushEllipse(initial + _current_point, radii, rotation, large, sweep, end + _current_point);
    }
    void pushEllipse(Point const &initial, Point const &radii, double rotation, bool large, bool sweep, Point const &end) {
        if(initial != _current_point) startPath(initial);
        pushEllipse(radii, rotation, large, sweep, end);
    }*/
    
    void pushSBasis(SBasisCurve &sb) {
        pushSBasis(sb.sbasis());
    }
    void pushSBasis(D2<SBasis> sb) {
        Point initial = Point(sb[X][0][0], sb[Y][0][0]);
        if (!_current_path) startPath(_current_point);
        if (distance(initial, _current_point) > _continuity_tollerance) {
            startPath(initial);
        } else if (_current_point != initial) {
            /* in this case there are three possible options
               1. connect the points with tiny line segments
                  this may well translate into bug reports from
                  users claiming "duplicate or extraneous nodes"
               2. fudge the initial point of the multidimsb
                  we've chosen to do this here but question the 
                  numerical stability of this decision
               3. translate the whole sbasis so that initial is coincident
                  with _current_point. this could very well lead
                  to an accumulation of error for paths that expect 
                  to meet in the end.
               perhaps someday an option could be made to allow 
               the user to choose between these alternatives
               if the need arises
            */
            sb[X][0][0] = _current_point[X];
            sb[Y][0][0] = _current_point[Y]; 
        }
        _current_path->append(sb);
    }
    
    void closePath() {
        if (_current_path) {
            _current_path->close(true);
            _current_path = NULL;
        }
        _current_point = _initial_point = Point();
    }

    std::vector<Path> const &peek() const { return _pathset; }

private:
    std::vector<Path> _pathset;
    Path *_current_path;
    Point _current_point;
    Point _initial_point;
    double _continuity_tollerance;
};

static
std::vector<Geom::Path>
read_svgd(std::istringstream & s) {
    assert(s);

    OldPathBuilder builder;

    char mode = 0;

    double nums[7];
    int cur = 0;
    while(!s.eof()) {
        char ch;
        s >> ch;
        if((ch >= 'A' and ch <= 'Z') or (ch >= 'a' and ch <= 'z')) {
            mode = ch;
            cur = 0;
        } else if (ch == ' ' or ch == '\t' or ch == '\n' or ch == '\r' or ch == ',')
            continue;
        else if ((ch >= '0' and ch <= '9') or ch == '-' or ch == '.' or ch == '+') {
            s.unget();
            //TODO: use something else, perhaps.  Unless the svg path number spec matches scan.
            s >> nums[cur];
            cur++;
        }
        
        switch(mode) {
        //FIXME: "If a moveto is followed by multiple pairs of coordinates, the subsequent pairs are treated as implicit lineto commands."
        case 'm':
            if(cur >= 2) {
                builder.startPathRel(point(nums, 0));
                cur = 0;
            }
            break;
        case 'M':
            if(cur >= 2) {
                builder.startPath(point(nums, 0));
                cur = 0;
            }
            break;
        case 'l':
            if(cur >= 2) {
                builder.pushLineRel(point(nums, 0));
                cur = 0;
            }
            break;
        case 'L':
            if(cur >= 2) {
                builder.pushLine(point(nums, 0));
                cur = 0;
            }
            break;
        case 'h':
            if(cur >= 1) {
                builder.pushHorizontalRel(nums[0]);
                cur = 0;
            }
            break;
        case 'H':
            if(cur >= 1) {
                builder.pushHorizontal(nums[0]);
                cur = 0;
            }
            break;
        case 'v':
            if(cur >= 1) {
                builder.pushVerticalRel(nums[0]);
                cur = 0;
            }
            break;
        case 'V':
            if(cur >= 1) {
                builder.pushVertical(nums[0]);
                cur = 0;
            }
            break;
        case 'c':
            if(cur >= 6) {
                builder.pushCubicRel(point(nums, 0), point(nums, 2), point(nums, 4));
                cur = 0;
            }
            break;
        case 'C':
            if(cur >= 6) {
                builder.pushCubic(point(nums, 0), point(nums, 2), point(nums, 4));
                cur = 0;
            }
            break;
        case 'q':
            if(cur >= 4) {
                builder.pushQuadraticRel(point(nums, 0), point(nums, 2));
                cur = 0;
            }
            break;
        case 'Q':
            if(cur >= 4) {
                builder.pushQuadratic(point(nums, 0), point(nums, 2));
                cur = 0;
            }
            break;
        case 'a':
            if(cur >= 7) {
                //builder.pushEllipseRel(point(nums, 0), nums[2], nums[3] > 0, nums[4] > 0, point(nums, 5));
                cur = 0;
            }
            break;
        case 'A':
            if(cur >= 7) {
                //builder.pushEllipse(point(nums, 0), nums[2], nums[3] > 0, nums[4] > 0, point(nums, 5));
                cur = 0;
            }
            break;
        case 'z':
        case 'Z':
            builder.closePath();
            break;
        }
    }
    return builder.peek();
}


#endif 
//##########################################################

std::vector<Geom::Path>  
SVGD_to_2GeomPath (char const *svgd)
{
    std::vector<Geom::Path> pathv;
#ifdef LPE_USE_2GEOM_CONVERSION
    try {
        pathv = Geom::parse_svg_path(svgd);
    }
    catch (std::runtime_error e) {
        g_warning("SVGPathParseError: %s", e.what());
    }
#else
    std::istringstream ss;
    std::string svgd_string = svgd;
    ss.str(svgd_string);
    pathv = read_svgd(ss);
#endif
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
