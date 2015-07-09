/* Conic section clipping with respect to a rectangle
 *
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

#ifndef CLIP_WITH_CAIRO_SUPPORT
    #include <2geom/conic_section_clipper.h>
#endif

namespace Geom
{

/*
 *  Find rectangle-conic crossing points. They are returned in the
 *  "crossing_points" parameter.
 *  The method returns true if the conic section intersects at least one
 *  of the four lines passing through rectangle edges, else it returns false.
 */
bool CLIPPER_CLASS::intersect (std::vector<Point> & crossing_points) const
{
    crossing_points.clear();

    std::vector<double> rts;
    std::vector<Point> cpts;
    // rectangle corners
    enum {TOP_LEFT, TOP_RIGHT, BOTTOM_RIGHT, BOTTOM_LEFT};

    bool no_crossing = true;

    // rigth edge
    cs.roots (rts, R.right(), X);
    if (!rts.empty())
    {
        no_crossing = false;
        DBGPRINT ("CLIP: right: rts[0] = ", rts[0])
        DBGPRINTIF ((rts.size() == 2), "CLIP: right: rts[1] = ", rts[1])

        Point corner1 = R.corner(TOP_RIGHT);
        Point corner2 = R.corner(BOTTOM_RIGHT);

        for (size_t i = 0; i < rts.size(); ++i)
        {
            if (rts[i] < R.top() || rts[i] > R.bottom())  continue;
            Point P (R.right(), rts[i]);
            if (are_near (P, corner1))
                P = corner1;
            else if (are_near (P, corner2))
                P = corner2;

            cpts.push_back (P);
        }
        if (cpts.size() == 2 && are_near (cpts[0], cpts[1]))
        {
            cpts[0] = middle_point (cpts[0], cpts[1]);
            cpts.pop_back();
        }
    }

    // top edge
    cs.roots (rts, R.top(), Y);
    if (!rts.empty())
    {
        no_crossing = false;
        DBGPRINT ("CLIP: top: rts[0] = ", rts[0])
        DBGPRINTIF ((rts.size() == 2), "CLIP: top: rts[1] = ", rts[1])

        Point corner1 = R.corner(TOP_RIGHT);
        Point corner2 = R.corner(TOP_LEFT);

        for (size_t i = 0; i < rts.size(); ++i)
        {
            if (rts[i] < R.left() || rts[i] > R.right())  continue;
            Point P (rts[i], R.top());
            if (are_near (P, corner1))
                P = corner1;
            else if (are_near (P, corner2))
                P = corner2;

            cpts.push_back (P);
        }
        if (cpts.size() == 2 && are_near (cpts[0], cpts[1]))
        {
            cpts[0] = middle_point (cpts[0], cpts[1]);
            cpts.pop_back();
        }
    }

    // left edge
    cs.roots (rts, R.left(), X);
    if (!rts.empty())
    {
        no_crossing = false;
        DBGPRINT ("CLIP: left: rts[0] = ", rts[0])
        DBGPRINTIF ((rts.size() == 2), "CLIP: left: rts[1] = ", rts[1])

        Point corner1 = R.corner(TOP_LEFT);
        Point corner2 = R.corner(BOTTOM_LEFT);

        for (size_t i = 0; i < rts.size(); ++i)
        {
            if (rts[i] < R.top() || rts[i] > R.bottom())  continue;
            Point P (R.left(), rts[i]);
            if (are_near (P, corner1))
                P = corner1;
            else if (are_near (P, corner2))
                P = corner2;

            cpts.push_back (P);
        }
        if (cpts.size() == 2 && are_near (cpts[0], cpts[1]))
        {
            cpts[0] = middle_point (cpts[0], cpts[1]);
            cpts.pop_back();
        }
    }

    // bottom edge
    cs.roots (rts, R.bottom(), Y);
    if (!rts.empty())
    {
        no_crossing = false;
        DBGPRINT ("CLIP: bottom: rts[0] = ", rts[0])
        DBGPRINTIF ((rts.size() == 2), "CLIP: bottom: rts[1] = ", rts[1])

        Point corner1 = R.corner(BOTTOM_RIGHT);
        Point corner2 = R.corner(BOTTOM_LEFT);

        for (size_t i = 0; i < rts.size(); ++i)
        {
            if (rts[i] < R.left() || rts[i] > R.right())  continue;
            Point P (rts[i], R.bottom());
            if (are_near (P, corner1))
                P = corner1;
            else if (are_near (P, corner2))
                P = corner2;

            cpts.push_back (P);
        }
        if (cpts.size() == 2 && are_near (cpts[0], cpts[1]))
        {
            cpts[0] = middle_point (cpts[0], cpts[1]);
            cpts.pop_back();
        }
    }

    DBGPRINT ("CLIP: intersect: crossing_points.size (with duplicates) = ",
              cpts.size())

    // remove duplicates
    std::sort (cpts.begin(), cpts.end(), Point::LexLess<X>());
    cpts.erase (std::unique (cpts.begin(), cpts.end()), cpts.end());


    // Order crossing points on the rectangle edge clockwise, so two consecutive
    // crossing points would be the end points of a conic arc all inside or all
    // outside the rectangle.
    std::map<double, size_t> cp_angles;
    for (size_t i = 0; i < cpts.size(); ++i)
    {
        cp_angles.insert (std::make_pair (cs.angle_at (cpts[i]), i));
    }

    std::map<double, size_t>::const_iterator pos;
    for (pos = cp_angles.begin(); pos != cp_angles.end(); ++pos)
    {
        crossing_points.push_back (cpts[pos->second]);
    }

    DBGPRINT ("CLIP: intersect: crossing_points.size = ", crossing_points.size())
    DBGPRINTCOLL ("CLIP: intersect: crossing_points:", crossing_points)

    return no_crossing;
}  // end function intersect



inline
double signed_triangle_area (Point const& p1, Point const& p2, Point const& p3)
{
    return (cross(p2, p3) - cross(p1, p3) + cross(p1, p2));
}


/*
 * Test if two crossing points are the end points of a conic arc inner to the
 * rectangle. In such a case the method returns true, else it returns false.
 * Moreover by the parameter "M" it returns a point inner to the conic arc
 * with the given end-points.
 *
 */
bool CLIPPER_CLASS::are_paired (Point& M, const Point & P1, const Point & P2) const
{
    using std::swap;

    /*
     *  we looks for the points on the conic whose tangent is parallel to the
     *  arc chord P1P2, they will be extrema of the conic arc P1P2 wrt the
     *  direction orthogonal to the chord
     */
    Point dir = P2 - P1;
    DBGPRINT ("CLIP: are_paired: first point:  ", P1)
    DBGPRINT ("CLIP: are_paired: second point: ", P2)

    double grad0 = 2 * cs.coeff(0) * dir[0] + cs.coeff(1) * dir[1];
    double grad1 = cs.coeff(1) * dir[0] + 2 * cs.coeff(2) * dir[1];
    double grad2 = cs.coeff(3) * dir[0] + cs.coeff(4) * dir[1];


    /*
     *  such points are found intersecating the conic section with the line
     *  orthogonal to "grad": the derivative wrt the "dir" direction
     */
    Line gl (grad0, grad1, grad2);
    std::vector<double> rts;
    rts = cs.roots (gl);
    DBGPRINT ("CLIP: are_paired: extrema: rts.size() = ", rts.size())



    std::vector<Point> extrema;
    for (size_t i = 0; i < rts.size(); ++i)
    {
        extrema.push_back (gl.pointAt (rts[i]));
    }

    if (extrema.size() == 2)
    {
        // in case we are dealing with an hyperbola we could have two extrema
        // on the same side wrt the line passing through P1 and P2, but
        // only the nearer extremum is on the arc P1P2
        double side0 = signed_triangle_area (P1, extrema[0], P2);
        double side1 = signed_triangle_area (P1, extrema[1], P2);

        if (sgn(side0) == sgn(side1))
        {
            if (std::fabs(side0) > std::fabs(side1)) {
                swap(extrema[0], extrema[1]);
            }
            extrema.pop_back();
        }
    }

    std::vector<Point> inner_points;
    for (size_t i = 0; i < extrema.size(); ++i)
    {
        if (!R.contains (extrema[i]))  continue;
        // in case we are dealing with an ellipse tangent to two orthogonal
        // rectangle edges we could have two extrema on opposite sides wrt the
        // line passing through P1P2 and both inner the rectangle; anyway, since
        // we order the crossing points clockwise we have only one extremum
        // that follows such an ordering wrt P1 and P2;
        // remark: the other arc will be selected when we test for the arc P2P1.
        double P1angle = cs.angle_at (P1);
        double P2angle = cs.angle_at (P2);
        double Qangle = cs.angle_at (extrema[i]);
        if (P1angle < P2angle && !(P1angle <= Qangle && Qangle <= P2angle))
            continue;
        if (P1angle > P2angle && !(P1angle <= Qangle || Qangle <= P2angle))
            continue;

        inner_points.push_back (extrema[i]);
    }

    if (inner_points.size() > 1)
    {
        THROW_LOGICALERROR ("conic section clipper: "
                            "more than one extremum found");
    }
    else if (inner_points.size() == 1)
    {
        M = inner_points.front();
        return true;
    }

    return false;
}


/*
 *  Pair the points contained in the "crossing_points" vector; the paired points
 *  are put in the paired_points vector so that given a point with an even index
 *  and the next one they are the end points of a conic arc that is inner to the
 *  rectangle. In the "inner_points" are returned points that are inner to the
 *  arc, where the inner point with index k is related to the arc with end
 *  points with indexes 2k, 2k+1. In case there are unpaired points the are put
 *  in to the "single_points" vector.
 */
void CLIPPER_CLASS::pairing (std::vector<Point> & paired_points,
                       std::vector<Point> & inner_points,
                       const std::vector<Point> & crossing_points)
{
    paired_points.clear();
    paired_points.reserve (crossing_points.size());

    inner_points.clear();
    inner_points.reserve (crossing_points.size() / 2);

    single_points.clear();

    // to keep trace of which crossing points have been paired
    std::vector<bool> paired (crossing_points.size(), false);

    Point M;

    // by the way we have ordered crossing points we need to test one point wrt
    // the next point only, for pairing; moreover the last point need to be
    // tested wrt the first point; pay attention: one point can be paired both
    // with the previous and the next one: this is not an error, think of
    // crossing points that are tangent to the rectangle edge (and inner);
    for (size_t i = 0; i < crossing_points.size(); ++i)
    {
        // we need to test the last point wrt the first one
        size_t j = (i == 0) ? (crossing_points.size() - 1) : (i-1);
        if (are_paired (M, crossing_points[j], crossing_points[i]))
        {
#ifdef CLIP_WITH_CAIRO_SUPPORT
            cairo_set_source_rgba(cr, 0.1, 0.1, 0.8, 1.0);
            draw_line_seg (cr, crossing_points[j], crossing_points[i]);
            draw_handle (cr, crossing_points[j]);
            draw_handle (cr, crossing_points[i]);
            draw_handle (cr, M);
            cairo_stroke (cr);
#endif
            paired[j] = paired[i] = true;
            paired_points.push_back (crossing_points[j]);
            paired_points.push_back (crossing_points[i]);
            inner_points.push_back (M);
        }
    }

    // some point are not paired with any point, e.g. a crossing point tangent
    // to a rectangle edge but with the conic arc outside the rectangle
    for (size_t i = 0; i < paired.size(); ++i)
    {
        if (!paired[i])
            single_points.push_back (crossing_points[i]);
    }
    DBGPRINTCOLL ("single_points", single_points)

}


/*
 *  This method clip the section conic wrt the rectangle and returns the inner
 *  conic arcs as a vector of RatQuad objects by the "arcs" parameter.
 */
bool CLIPPER_CLASS::clip (std::vector<RatQuad> & arcs)
{
    using std::swap;

    arcs.clear();
    std::vector<Point> crossing_points;
    std::vector<Point> paired_points;
    std::vector<Point> inner_points;

    Line l1, l2;
    if (cs.decompose (l1, l2))
    {
        bool inner_empty = true;

        DBGINFO ("CLIP: degenerate section conic")

        boost::optional<LineSegment> ls1 = Geom::clip (l1, R);
        if (ls1)
        {
            if (ls1->isDegenerate())
            {
                single_points.push_back (ls1->initialPoint());
            }
            else
            {
                Point M = middle_point (*ls1);
                arcs.push_back
                    (RatQuad (ls1->initialPoint(), M, ls1->finalPoint(), 1));
                inner_empty = false;
            }
        }

        boost::optional<LineSegment> ls2 = Geom::clip (l2, R);
        if (ls2)
        {
            if (ls2->isDegenerate())
            {
                single_points.push_back (ls2->initialPoint());
            }
            else
            {
                Point M = middle_point (*ls2);
                arcs.push_back
                    (RatQuad (ls2->initialPoint(), M, ls2->finalPoint(), 1));
                inner_empty = false;
            }
        }

        return !inner_empty;
    }


    bool no_crossing = intersect (crossing_points);

    // if the only crossing point is a rectangle corner than the section conic
    // is all outside the rectangle
    if (crossing_points.size() == 1)
    {
        for (size_t i = 0; i < 4; ++i)
        {
            if (crossing_points[0] == R.corner(i))
            {
                single_points.push_back (R.corner(i));
                return false;
            }
        }
    }

    // if the conic does not cross any line passing through a rectangle edge or
    // it is tangent to only one edge then it is an ellipse
    if (no_crossing
            || (crossing_points.size() == 1 && single_points.empty()))
    {
        // if the ellipse centre is inside the rectangle
        // then so it is the ellipse
        boost::optional<Point> c = cs.centre();
        if (c && R.contains (*c))
        {
            DBGPRINT ("CLIP: ellipse with centre", *c)
            // we set paired and inner points by finding the ellipse
            // intersection with its axes; this choice let us having a more
            // accurate RatQuad parametric arc
            paired_points.resize(4);
            std::vector<double> rts;
            double angle = cs.axis_angle();
            Line axis1 (*c, angle);
            rts = cs.roots (axis1);
            if (rts[0] > rts[1])  swap (rts[0], rts[1]);
            paired_points[0] = axis1.pointAt (rts[0]);
            paired_points[1] = axis1.pointAt (rts[1]);
            paired_points[2] = paired_points[1];
            paired_points[3] = paired_points[0];
            Line axis2 (*c, angle + M_PI/2);
            rts = cs.roots (axis2);
            if (rts[0] > rts[1])  swap (rts[0], rts[1]);
            inner_points.push_back (axis2.pointAt (rts[0]));
            inner_points.push_back (axis2.pointAt (rts[1]));
        }
        else if (crossing_points.size() == 1)
        {
            // so we have a tangent crossing point but the ellipse is outside
            // the rectangle
            single_points.push_back (crossing_points[0]);
        }
    }
    else
    {
        // in case the conic section intersects any of the four lines passing
        // through the rectangle edges but it does not cross any rectangle edge
        // then the conic is all outer of the rectangle
        if (crossing_points.empty()) return false;
        // else we need to pair crossing points, and to find an arc inner point
        // in order to generate a RatQuad object
        pairing (paired_points, inner_points, crossing_points);
    }


    // we split arcs until the end-point distance is less than a given value,
    // in this way the RatQuad parametrization is enough accurate
    std::list<Point> points;
    std::list<Point>::iterator sp, ip, fp;
    for (size_t i = 0, j = 0; i < paired_points.size(); i += 2, ++j)
    {
        //DBGPRINT ("CLIP: clip: P = ", paired_points[i])
        //DBGPRINT ("CLIP: clip: M = ", inner_points[j])
        //DBGPRINT ("CLIP: clip: Q = ", paired_points[i+1])

        // in case inner point and end points are near is better not split
        // the conic arc further or we could get a degenerate RatQuad object
        if (are_near (paired_points[i], inner_points[j], 1e-4)
                && are_near (paired_points[i+1], inner_points[j], 1e-4))
        {
            arcs.push_back (cs.toRatQuad (paired_points[i],
                                          inner_points[j],
                                          paired_points[i+1]));
            continue;
        }

        // populate the list
        points.push_back(paired_points[i]);
        points.push_back(inner_points[j]);
        points.push_back(paired_points[i+1]);

        // an initial unconditioned splitting
        sp = points.begin();
        ip = sp; ++ip;
        fp = ip; ++fp;
        rsplit (points, sp, ip, size_t(1u));
        rsplit (points, ip, fp, size_t(1u));

        // length conditioned split
        sp = points.begin();
        fp = sp; ++fp;
        while (fp != points.end())
        {
            rsplit (points, sp, fp, 100.0);
            sp = fp;
            ++fp;
        }

        sp = points.begin();
        ip = sp; ++ip;
        fp = ip; ++fp;
        //DBGPRINT ("CLIP: points ", j)
        //DBGPRINT ("CLIP: points.size = ", points.size())
        while (ip != points.end())
        {
#ifdef CLIP_WITH_CAIRO_SUPPORT
            cairo_set_source_rgba(cr, 0.1, 0.1, 0.8, 1.0);
            draw_handle (cr, *sp);
            draw_handle (cr, *ip);
            cairo_stroke (cr);
#endif
            //std::cerr << "CLIP: arc: [" << *sp << ", " << *ip << ", "
            //          << *fp << "]" << std::endl;
            arcs.push_back (cs.toRatQuad (*sp, *ip, *fp));
            sp = fp;
            ip = sp; ++ip;
            fp = ip; ++fp;
        }
        points.clear();
    }
    DBGPRINT ("CLIP: arcs.size() = ", arcs.size())
    return (arcs.size() != 0);
} // end method clip


} // end namespace geom




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
