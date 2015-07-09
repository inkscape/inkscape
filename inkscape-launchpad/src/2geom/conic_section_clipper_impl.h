/** @file
 * @brief Conic section clipping with respect to a rectangle
 *//*
 * Authors:
 *      Marco Cecchetti <mrcekets at gmail>
 *
 * Copyright 2009  authors
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

#ifndef LIB2GEOM_SEEN_CONIC_SECTION_CLIPPER_IMPL_H
#define LIB2GEOM_SEEN_CONIC_SECTION_CLIPPER_IMPL_H


#include <2geom/conicsec.h>
#include <2geom/line.h>

#include <list>
#include <map>



#ifdef CLIP_WITH_CAIRO_SUPPORT
    #include <2geom/toys/path-cairo.h>
    #define CLIPPER_CLASS clipper_cr
#else
    #define CLIPPER_CLASS clipper
#endif

//#define CLIPDBG

#ifdef CLIPDBG
#include <2geom/toys/path-cairo.h>
#define DBGINFO(msg) \
    std::cerr << msg << std::endl;
#define DBGPRINT(msg, var) \
    std::cerr << msg << var << std::endl;
#define DBGPRINTIF(cond, msg, var) \
    if (cond)                                                                  \
        std::cerr << msg << var << std::endl;

#define DBGPRINT2(msg1, var1, msg2, var2) \
    std::cerr << msg1 << var1 << msg2 << var2 << std::endl;

#define DBGPRINTCOLL(msg, coll) \
    if (coll.size() != 0)                                                      \
        std::cerr << msg << ":\n";                                             \
    for (size_t i = 0; i < coll.size(); ++i)                                   \
    {                                                                          \
        std::cerr << i << ": " << coll[i] << "\n";                             \
    }

#else
#define DBGINFO(msg)
#define DBGPRINT(msg, var)
#define DBGPRINTIF(cond, msg, var)
#define DBGPRINT2(msg1, var1, msg2, var2)
#define DBGPRINTCOLL(msg, coll)
#endif




namespace Geom
{

class CLIPPER_CLASS
{

  public:

#ifdef CLIP_WITH_CAIRO_SUPPORT
    clipper_cr (cairo_t* _cr, const xAx & _cs, const Rect & _R)
        : cr(_cr), cs(_cs), R(_R)
    {
        DBGPRINT ("CLIP: right side: ", R.right())
        DBGPRINT ("CLIP: top side: ", R.top())
        DBGPRINT ("CLIP: left side: ", R.left())
        DBGPRINT ("CLIP: bottom side: ", R.bottom())
    }
#else
    clipper (const xAx & _cs, const Rect & _R)
        : cs(_cs), R(_R)
    {
    }
#endif

    bool clip (std::vector<RatQuad> & arcs);

    bool found_any_isolated_point() const
    {
        return ( !single_points.empty() );
    }

    const std::vector<Point> & isolated_points() const
    {
        return single_points;
    }


  private:
    bool intersect (std::vector<Point> & crossing_points) const;

    bool are_paired (Point & M, const Point & P1, const Point & P2) const;
    void pairing (std::vector<Point> & paired_points,
                  std::vector<Point> & inner_points,
                  const std::vector<Point> & crossing_points);

    Point find_inner_point_by_bisector_line (const Point & P,
                                             const Point & Q) const;
    Point find_inner_point (const Point & P, const Point & Q) const;

    std::list<Point>::iterator split (std::list<Point> & points,
                                      std::list<Point>::iterator sp,
                                      std::list<Point>::iterator fp) const;
    void rsplit (std::list<Point> & points,
                 std::list<Point>::iterator sp,
                 std::list<Point>::iterator fp,
                 size_t k) const;

    void rsplit (std::list<Point> & points,
                 std::list<Point>::iterator sp,
                 std::list<Point>::iterator fp,
                 double length) const;

  private:
#ifdef CLIP_WITH_CAIRO_SUPPORT
    cairo_t* cr;
#endif
    const xAx & cs;
    const Rect & R;
    std::vector<Point> single_points;
};




/*
 *  Given two point "P", "Q" on the conic section the method computes
 *  a third point inner to the arc with end-point "P", "Q".
 *  The new point is found by intersecting the conic with the bisector line
 *  of the PQ line segment.
 */
inline
Point CLIPPER_CLASS::find_inner_point_by_bisector_line (const Point & P,
                                                  const Point & Q) const
{
    DBGPRINT ("CLIP: find_inner_point_by_bisector_line: P = ", P)
    DBGPRINT ("CLIP: find_inner_point_by_bisector_line: Q = ", Q)
    Line bl = make_bisector_line (LineSegment (P, Q));
    std::vector<double> rts = cs.roots (bl);
    //DBGPRINT ("CLIP: find_inner_point: rts.size = ", rts.size())
    double t;
    if (rts.size() == 0)
    {
        THROW_LOGICALERROR ("clipper::find_inner_point_by_bisector_line: "
                            "no conic-bisector line intersection point");
    }
    if (rts.size() == 2)
    {
        // we suppose that the searched point is the nearest
        // to the line segment PQ
        t = (std::fabs(rts[0]) < std::fabs(rts[1])) ? rts[0] : rts[1];
    }
    else
    {
        t = rts[0];
    }
    return bl.pointAt (t);
}


/*
 *  Given two point "P", "Q" on the conic section the method computes
 *  a third point inner to the arc with end-point "P", "Q".
 *  The new point is found by intersecting the conic with the line
 *  passing through the middle point of the PQ line segment and
 *  the intersection point of the tangent lines at points P and Q.
 */
inline
Point CLIPPER_CLASS::find_inner_point (const Point & P, const Point & Q) const
{

    Line l1 = cs.tangent (P);
    Line l2 = cs.tangent (Q);
    Line l;
    // in case we fail to find a crossing point we fall back to the bisector
    // method
    try
    {
        OptCrossing oc = intersection(l1, l2);
        if (!oc)
        {
            return find_inner_point_by_bisector_line (P, Q);
        }
        l.setPoints (l1.pointAt (oc->ta), middle_point (P, Q));
    }
    catch (Geom::InfiniteSolutions e)
    {
        return find_inner_point_by_bisector_line (P, Q);
    }

    std::vector<double> rts = cs.roots (l);
    double t;
    if (rts.size() == 0)
    {
        return find_inner_point_by_bisector_line (P, Q);
    }
    // the line "l" origin is set to the tangent crossing point so in case
    // we find two intersection points only the nearest belongs to the given arc
    // pay attention: in case we are dealing with an hyperbola (remember that
    // end points are on the same branch, because they are paired) the tangent
    // crossing point belongs to the angle delimited by hyperbola asymptotes
    // and containing the given hyperbola branch, so the previous statement is
    // still true
    if (rts.size() == 2)
    {
        t = (std::fabs(rts[0]) < std::fabs(rts[1])) ? rts[0] : rts[1];
    }
    else
    {
        t = rts[0];
    }
    return l.pointAt (t);
}


/*
 *  Given a list of points on the conic section, and given two consecutive
 *  points belonging to the list and passed by two list iterators, the method
 *  finds a new point that is inner to the conic arc which has the two passed
 *  points as initial and final point. This new point is inserted into the list
 *  between the two passed points and an iterator pointing to the new point
 *  is returned.
 */
inline
std::list<Point>::iterator CLIPPER_CLASS::split (std::list<Point> & points,
                                           std::list<Point>::iterator sp,
                                           std::list<Point>::iterator fp) const
{
    Point new_point = find_inner_point (*sp, *fp);
    std::list<Point>::iterator ip = points.insert (fp, new_point);
    //std::cerr << "CLIP: split: [" << *sp << ", " << *ip << ", "
    //          << *fp << "]" << std::endl;
    return ip;
}


/*
 *  Given a list of points on the conic section, and given two consecutive
 *  points belonging to the list and passed by two list iterators, the method
 *  recursively finds new points that are inner to the conic arc which has
 *  the two passed points as initial and final point. The recursion stop after
 *  "k" recursive calls. These new points are inserted into the list between
 *  the two passed points, and in the order we cross them going from
 *  the initial to the final arc point.
 */
inline
void CLIPPER_CLASS::rsplit (std::list<Point> & points,
                      std::list<Point>::iterator sp,
                      std::list<Point>::iterator fp,
                      size_t k) const
{
    if (k == 0)
    {
        //DBGINFO("CLIP: split: no further split")
        return;
    }

    std::list<Point>::iterator ip = split (points, sp, fp);
    --k;
    rsplit (points, sp, ip, k);
    rsplit (points, ip, fp, k);
}


/*
 *  Given a list of points on the conic section, and given two consecutive
 *  points belonging to the list and passed by two list iterators, the method
 *  recursively finds new points that are inner to the conic arc which has
 *  the two passed points as initial and final point. The recursion stop when
 *  the max distance between the new computed inner point and the two passed
 *  arc end-points is less then the value specified by the "length" parameter.
 *  These new points are inserted into the list between the two passed points,
 *  and in the order we cross them going from the initial to the final arc point.
 */
inline
void CLIPPER_CLASS::rsplit (std::list<Point> & points,
                      std::list<Point>::iterator sp,
                      std::list<Point>::iterator fp,
                      double length) const
{
    std::list<Point>::iterator ip = split (points, sp, fp);
    double d1 = distance (*sp, *ip);
    double d2 = distance (*ip, *fp);
    double mdist = std::max (d1, d2);

    if (mdist < length)
    {
        //DBGINFO("CLIP: split: no further split")
        return;
    }

    // they have to be called both to keep the number of points in the list
    // in the form 2k+1 where k are the sub-arcs the initial arc is splitted in.
    rsplit (points, sp, ip, length);
    rsplit (points, ip, fp, length);
}


} // end namespace Geom

#endif // LIB2GEOM_SEEN_CONIC_SECTION_CLIPPER_IMPL_H

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
