/*
 * Routines for dealing with lines (intersections, etc.)
 *
 * Authors:
 *   Maximilian Albert <Anhalter42@gmx.de>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_LINE_GEOMETRY_H
#define SEEN_LINE_GEOMETRY_H

#include <2geom/point.h>
#include <boost/optional.hpp>

#include "axis-manip.h" // FIXME: This is only for Box3D::epsilon; move that to a better location
#include "persp3d.h"

class SPDesktop;
typedef unsigned int guint32;

namespace Box3D {

class Line {
public:
    Line(Geom::Point const &start, Geom::Point const &vec, bool is_endpoint = true);
    Line(Line const &line);
    virtual ~Line() {}
    Line &operator=(Line const &line);
    virtual boost::optional<Geom::Point> intersect(Line const &line);
    inline Geom::Point direction () { return v_dir; }
    
    Geom::Point closest_to(Geom::Point const &pt); // returns the point on the line closest to pt 

    friend inline std::ostream &operator<< (std::ostream &out_file, const Line &in_line);
    boost::optional<Geom::Point> intersection_with_viewbox (SPDesktop *desktop);
    inline bool lie_on_same_side (Geom::Point const &A, Geom::Point const &B) {
        /* If A is a point in the plane and n is the normal vector of the line then
           the sign of dot(A, n) specifies the half-plane in which A lies.
           Thus A and B lie on the same side if the dot products have equal sign. */
        return ((Geom::dot(A, normal) - d0) * (Geom::dot(B, normal) - d0)) > 0;
    }

    double lambda (Geom::Point const pt);
    inline Geom::Point point_from_lambda (double const lambda) { 
        return (pt + lambda * Geom::unit_vector (v_dir)); }

protected:
    void set_direction(Geom::Point const &dir);
    inline static bool pts_coincide (Geom::Point const pt1, Geom::Point const pt2)
    {
        return (Geom::L2 (pt2 - pt1) < epsilon);
    }

    Geom::Point pt;
    Geom::Point v_dir;
    Geom::Point normal;
    Geom::Coord d0;
};

inline double determinant (Geom::Point const &a, Geom::Point const &b)
{
    return (a[Geom::X] * b[Geom::Y] - a[Geom::Y] * b[Geom::X]);
}
std::pair<double, double> coordinates (Geom::Point const &v1, Geom::Point const &v2, Geom::Point const &w);
bool lies_in_sector (Geom::Point const &v1, Geom::Point const &v2, Geom::Point const &w);
bool lies_in_quadrangle (Geom::Point const &A, Geom::Point const &B, Geom::Point const &C, Geom::Point const &D, Geom::Point const &pt);
std::pair<Geom::Point, Geom::Point> side_of_intersection (Geom::Point const &A, Geom::Point const &B,
                                                          Geom::Point const &C, Geom::Point const &D,
                                                          Geom::Point const &pt, Geom::Point const &dir);

/*** For debugging purposes: Draw a knot/node of specified size and color at the given position ***/
void create_canvas_point(Geom::Point const &pos, double size = 4.0, guint32 rgba = 0xff00007f);

/*** For debugging purposes: Draw a line between the specified points ***/
void create_canvas_line(Geom::Point const &p1, Geom::Point const &p2, guint32 rgba = 0xff00007f);


/** A function to print out the Line.  It just prints out the coordinates of start point and
    direction on the given output stream */
inline std::ostream &operator<< (std::ostream &out_file, const Line &in_line) {
    out_file << "Start: " << in_line.pt << "  Direction: " << in_line.v_dir;
    return out_file;
}

} // namespace Box3D


#endif /* !SEEN_LINE_GEOMETRY_H */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
