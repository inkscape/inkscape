/**
 * \file
 * \brief  Elliptical arc curve
 *
 *//*
 * Authors:
 *    MenTaLguY <mental@rydia.net>
 *    Marco Cecchetti <mrcekets at gmail.com>
 *    Krzysztof Kosiński <tweenk.pl@gmail.com>
 * 
 * Copyright 2007-2009 Authors
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

#ifndef LIB2GEOM_SEEN_ELLIPTICAL_ARC_H
#define LIB2GEOM_SEEN_ELLIPTICAL_ARC_H

#include <algorithm>
#include <2geom/affine.h>
#include <2geom/angle.h>
#include <2geom/bezier-curve.h>
#include <2geom/curve.h>
#include <2geom/ellipse.h>
#include <2geom/sbasis-curve.h>  // for non-native methods
#include <2geom/utils.h>

namespace Geom 
{

class EllipticalArc : public Curve
{
public:
    /** @brief Creates an arc with all variables set to zero. */
    EllipticalArc()
        : _initial_point(0,0)
        , _final_point(0,0)
        , _large_arc(false)
    {}
    /** @brief Create a new elliptical arc.
     * @param ip Initial point of the arc
     * @param r Rays of the ellipse as a point
     * @param rot Angle of rotation of the X axis of the ellipse in radians
     * @param large If true, the large arc is chosen (always >= 180 degrees), otherwise
     *              the smaller arc is chosen
     * @param sweep If true, the clockwise arc is chosen, otherwise the counter-clockwise
     *              arc is chosen
     * @param fp Final point of the arc */
    EllipticalArc( Point const &ip, Point const &r,
                   Coord rot_angle, bool large_arc, bool sweep,
                   Point const &fp
                 )
        : _initial_point(ip)
        , _final_point(fp)
        , _ellipse(0, 0, r[X], r[Y], rot_angle)
        , _angles(0, 0, sweep)
        , _large_arc(large_arc)
    {
        _updateCenterAndAngles();
    }

    /// Create a new elliptical arc, giving the ellipse's rays as separate coordinates.
    EllipticalArc( Point const &ip, Coord rx, Coord ry,
                   Coord rot_angle, bool large_arc, bool sweep,
                   Point const &fp
                 )
        : _initial_point(ip)
        , _final_point(fp)
        , _ellipse(0, 0, rx, ry, rot_angle)
        , _angles(0, 0, sweep)
        , _large_arc(large_arc)
    {
        _updateCenterAndAngles();
    }

    /// @name Retrieve basic information
    /// @{

    /** @brief Get a coordinate of the elliptical arc's center.
     * @param d The dimension to retrieve
     * @return The selected coordinate of the center */
    Coord center(Dim2 d) const { return _ellipse.center(d); }

    /** @brief Get the arc's center
     * @return The arc's center, situated on the intersection of the ellipse's rays */
    Point center() const { return _ellipse.center(); }

    /** @brief Get one of the ellipse's rays
     * @param d Dimension to retrieve
     * @return The selected ray of the ellipse */
    Coord ray(Dim2 d) const { return _ellipse.ray(d); }

    /** @brief Get both rays as a point
     * @return Point with X equal to the X ray and Y to Y ray */
    Point rays() const { return _ellipse.rays(); }

    /** @brief Get the defining ellipse's rotation
     * @return Angle between the +X ray of the ellipse and the +X axis */
    Angle rotationAngle() const {
        return _ellipse.rotationAngle();
    }

    /** @brief Whether the arc is larger than half an ellipse.
     * @return True if the arc is larger than \f$\pi\f$, false otherwise */
    bool largeArc() const { return _large_arc; }

    /** @brief Whether the arc turns clockwise
     * @return True if the arc makes a clockwise turn when going from initial to final
     *         point, false otherwise */
    bool sweep() const { return _angles.sweep(); }

    Angle initialAngle() const { return _angles.initialAngle(); }
    Angle finalAngle() const { return _angles.finalAngle(); }
    /// @}

    /// @name Modify parameters
    /// @{

    /// Change all of the arc's parameters.
    void set( Point const &ip, double rx, double ry,
              double rot_angle, bool large_arc, bool sweep,
              Point const &fp
            )
    {
        _initial_point = ip;
        _final_point = fp;
        _ellipse.setRays(rx, ry);
        _ellipse.setRotationAngle(rot_angle);
        _angles.setSweep(sweep);
        _large_arc = large_arc;
        _updateCenterAndAngles();
    }

    /// Change all of the arc's parameters.
    void set( Point const &ip, Point const &r,
              Angle rot_angle, bool large_arc, bool sweep,
              Point const &fp
            )
    {
        _initial_point = ip;
        _final_point = fp;
        _ellipse.setRays(r);
        _ellipse.setRotationAngle(rot_angle);
        _angles.setSweep(sweep);
        _large_arc = large_arc;
        _updateCenterAndAngles();
    }

    /** @brief Change the initial and final point in one operation.
     * This method exists because modifying any of the endpoints causes rather costly
     * recalculations of the center and extreme angles.
     * @param ip New initial point
     * @param fp New final point */
    void setEndpoints(Point const &ip, Point const &fp) {
        _initial_point = ip;
        _final_point = fp;
        _updateCenterAndAngles();
    }
    /// @}

    /// @name Evaluate the arc as a function
    /// @{
    /** Check whether the arc contains the given angle
     * @param t The angle to check
     * @return True if the arc contains the angle, false otherwise */
    bool containsAngle(Angle angle) const { return _angles.contains(angle); }

    /** @brief Evaluate the arc at the specified angular coordinate
     * @param t Angle
     * @return Point corresponding to the given angle */
    Point pointAtAngle(Coord t) const;

    /** @brief Evaluate one of the arc's coordinates at the specified angle
     * @param t Angle
     * @param d The dimension to retrieve
     * @return Selected coordinate of the arc at the specified angle */
    Coord valueAtAngle(Coord t, Dim2 d) const;

    /// Compute the curve time value corresponding to the given angular value.
    Coord timeAtAngle(Angle a) const { return _angles.timeAtAngle(a); }

    /// Compute the angular domain value corresponding to the given time value.
    Angle angleAt(Coord t) const { return _angles.angleAt(t); }

    /** @brief Compute the amount by which the angle parameter changes going from start to end.
     * This has range \f$(-2\pi, 2\pi)\f$ and thus cannot be represented as instance
     * of the class Angle. Add this to the initial angle to obtain the final angle. */
    Coord sweepAngle() const { return _angles.sweepAngle(); }

    /** @brief Get the elliptical angle spanned by the arc.
     * This is basically the absolute value of sweepAngle(). */
    Coord angularExtent() const { return _angles.extent(); }

    /// Get the angular interval of the arc.
    AngleInterval angularInterval() const { return _angles; }

    /// Evaluate the arc in the curve domain, i.e. \f$[0, 1]\f$.
    virtual Point pointAt(Coord t) const;

    /// Evaluate a single coordinate on the arc in the curve domain.
    virtual Coord valueAt(Coord t, Dim2 d) const;

    /** @brief Compute a transform that maps the unit circle to the arc's ellipse.
     * Each ellipse can be interpreted as a translated, scaled and rotate unit circle.
     * This function returns the transform that maps the unit circle to the arc's ellipse.
     * @return Transform from unit circle to the arc's ellipse */
    Affine unitCircleTransform() const {
        Affine result = _ellipse.unitCircleTransform();
        return result;
    }

    /** @brief Compute a transform that maps the arc's ellipse to the unit circle. */
    Affine inverseUnitCircleTransform() const {
        Affine result = _ellipse.inverseUnitCircleTransform();
        return result;
    }
    /// @}

    /// @name Deal with degenerate ellipses.
    /// @{
    /** @brief Check whether both rays are nonzero.
     * If they are not, the arc is represented as a line segment instead. */
    bool isChord() const {
        return ray(X) == 0 || ray(Y) == 0;
    }

    /** @brief Get the line segment connecting the arc's endpoints.
     * @return A linear segment with initial and final point correspoding to those of the arc. */
    LineSegment chord() const { return LineSegment(_initial_point, _final_point); }
    /// @}

    // implementation of overloads goes here
    virtual Point initialPoint() const { return _initial_point; }
    virtual Point finalPoint() const { return _final_point; }
    virtual Curve* duplicate() const { return new EllipticalArc(*this); }
    virtual void setInitial(Point const &p) {
        _initial_point = p;
        _updateCenterAndAngles();
    }
    virtual void setFinal(Point const &p) {
        _final_point = p;
        _updateCenterAndAngles();
    }
    virtual bool isDegenerate() const {
        return _initial_point == _final_point;
    }
    virtual bool isLineSegment() const { return isChord(); }
    virtual Rect boundsFast() const {
        return boundsExact();
    }
    virtual Rect boundsExact() const;
    // TODO: native implementation of the following methods
    virtual OptRect boundsLocal(OptInterval const &i, unsigned int deg) const {
        return SBasisCurve(toSBasis()).boundsLocal(i, deg);
    }
    virtual std::vector<double> roots(double v, Dim2 d) const;
#ifdef HAVE_GSL
    virtual std::vector<double> allNearestTimes( Point const& p, double from = 0, double to = 1 ) const;
    virtual double nearestTime( Point const& p, double from = 0, double to = 1 ) const {
        if ( are_near(ray(X), ray(Y)) && are_near(center(), p) ) {
            return from;
        }
        return allNearestTimes(p, from, to).front();
    }
#endif
    virtual std::vector<CurveIntersection> intersect(Curve const &other, Coord eps=EPSILON) const;
    virtual int degreesOfFreedom() const { return 7; }
    virtual Curve *derivative() const;

    using Curve::operator*=;
    virtual void operator*=(Translate const &tr);
    virtual void operator*=(Scale const &s);
    virtual void operator*=(Rotate const &r);
    virtual void operator*=(Zoom const &z);
    virtual void operator*=(Affine const &m);

    virtual std::vector<Point> pointAndDerivatives(Coord t, unsigned int n) const;
    virtual D2<SBasis> toSBasis() const;
    virtual Curve *portion(double f, double t) const;
    virtual Curve *reverse() const;
    virtual bool operator==(Curve const &c) const;
    virtual bool isNear(Curve const &other, Coord precision) const;
    virtual void feed(PathSink &sink, bool moveto_initial) const;
    virtual int winding(Point const &p) const;

private:
    void _updateCenterAndAngles();
    void _filterIntersections(std::vector<ShapeIntersection> &xs, bool is_first) const;

    Point _initial_point, _final_point;
    Ellipse _ellipse;
    AngleInterval _angles;
    bool _large_arc;
}; // end class EllipticalArc


// implemented in elliptical-arc-from-sbasis.cpp
/** @brief Fit an elliptical arc to an SBasis fragment.
 * @relates EllipticalArc */
bool arc_from_sbasis(EllipticalArc &ea, D2<SBasis> const &in,
                     double tolerance = EPSILON, unsigned num_samples = 20);

/** @brief Debug output for elliptical arcs.
 * @relates EllipticalArc */
std::ostream &operator<<(std::ostream &out, EllipticalArc const &ea);

} // end namespace Geom

#endif // LIB2GEOM_SEEN_ELLIPTICAL_ARC_H

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
