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

#include "libnr/nr-point.h"
#include "libnr/nr-point-fns.h"
#include <boost/optional.hpp>
#include "glib.h"
#include "display/sp-ctrlline.h"
#include "axis-manip.h" // FIXME: This is only for Box3D::epsilon; move that to a better location

#include "document.h"
#include "ui/view/view.h"

namespace Box3D {

class Line {
public:
    Line(NR::Point const &start, NR::Point const &vec, bool is_endpoint = true);
    Line(Line const &line);
    virtual ~Line() {}
    Line &operator=(Line const &line);
    virtual boost::optional<NR::Point> intersect(Line const &line);
    inline NR::Point direction () { return v_dir; }
    
    NR::Point closest_to(NR::Point const &pt); // returns the point on the line closest to pt 

    friend inline std::ostream &operator<< (std::ostream &out_file, const Line &in_line);
    boost::optional<NR::Point> intersection_with_viewbox (SPDesktop *desktop);
    inline bool lie_on_same_side (NR::Point const &A, NR::Point const &B) {
        /* If A is a point in the plane and n is the normal vector of the line then
           the sign of dot(A, n) specifies the half-plane in which A lies.
           Thus A and B lie on the same side if the dot products have equal sign. */
        return ((NR::dot(A, normal) - d0) * (NR::dot(B, normal) - d0)) > 0;
    }

    double lambda (NR::Point const pt);
    inline NR::Point point_from_lambda (double const lambda) { 
        return (pt + lambda * NR::unit_vector (v_dir)); }

protected:
    void set_direction(NR::Point const &dir);
    inline static bool pts_coincide (NR::Point const pt1, NR::Point const pt2)
    {
        return (NR::L2 (pt2 - pt1) < epsilon);
    }

    NR::Point pt;
    NR::Point v_dir;
    NR::Point normal;
    NR::Coord d0;
};

inline double determinant (NR::Point const &a, NR::Point const &b)
{
    return (a[NR::X] * b[NR::Y] - a[NR::Y] * b[NR::X]);
}
std::pair<double, double> coordinates (NR::Point const &v1, NR::Point const &v2, NR::Point const &w);
bool lies_in_sector (NR::Point const &v1, NR::Point const &v2, NR::Point const &w);
bool lies_in_quadrangle (NR::Point const &A, NR::Point const &B, NR::Point const &C, NR::Point const &D, NR::Point const &pt);
std::pair<NR::Point, NR::Point> side_of_intersection (NR::Point const &A, NR::Point const &B,
                                                      NR::Point const &C, NR::Point const &D,
                                                      NR::Point const &pt, NR::Point const &dir);

/*** For debugging purposes: Draw a knot/node of specified size and color at the given position ***/
void create_canvas_point(NR::Point const &pos, double size = 4.0, guint32 rgba = 0xff00007f);

/*** For debugging purposes: Draw a line between the specified points ***/
void create_canvas_line(NR::Point const &p1, NR::Point const &p2, guint32 rgba = 0xff00007f);


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
