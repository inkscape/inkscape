/**
 *  \file
 *  \brief Various trigoniometric helper functions
 *//*
 *  Authors:
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *   Marco Cecchetti <mrcekets at gmail.com>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2007-2010 Authors
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

#ifndef LIB2GEOM_SEEN_ANGLE_H
#define LIB2GEOM_SEEN_ANGLE_H

#include <cmath>
#include <boost/operators.hpp>
#include <2geom/exception.h>
#include <2geom/coord.h>
#include <2geom/point.h>

namespace Geom {

#ifndef M_PI
# define M_PI 3.14159265358979323846
#endif
#ifndef M_1_2PI
# define M_1_2PI 0.159154943091895335768883763373
#endif

/** @brief Wrapper for angular values.
 *
 * This class is a convenience wrapper that implements the behavior generally expected of angles,
 * like addition modulo \f$2\pi\f$. The value returned from the default conversion
 * to <tt>double</tt> is in the range \f$[-\pi, \pi)\f$ - the convention used by C's
 * math library.
 *
 * @ingroup Primitives
 */
class Angle
    : boost::additive< Angle
    , boost::equality_comparable< Angle
      > >
{
public:
    Angle() : _angle(0) {} //added default constructor because of cython
    Angle(Coord v) : _angle(v) { _normalize(); } // this can be called implicitly
    explicit Angle(Point p) : _angle(atan2(p)) { _normalize(); }
    Angle(Point a, Point b) : _angle(angle_between(a, b)) { _normalize(); }
    operator Coord() const { return radians(); }
    Angle &operator+=(Angle const &o) {
        _angle += o._angle;
        _normalize();
        return *this;
    }
    Angle &operator-=(Angle const &o) {
        _angle -= o._angle;
        _normalize();
        return *this;
    }
    bool operator==(Angle const &o) const {
        return _angle == o._angle;
    }

    /** @brief Get the angle as radians.
     * @return Number in range \f$[-\pi, \pi)\f$. */
    Coord radians() const {
        return _angle >= M_PI ? _angle - 2*M_PI : _angle;
    }
    /** @brief Get the angle as positive radians.
     * @return Number in range \f$[0, 2\pi)\f$. */
    Coord radians0() const {
        return _angle;
    }
    /** @brief Get the angle as degrees in math convention.
     * @return Number in range [-180, 180) obtained by scaling the result of radians()
     *         by \f$180/\pi\f$. */
    Coord degrees() const { return radians() * (180.0 / M_PI); }
    /** @brief Get the angle as degrees in clock convention.
     * This method converts the angle to the "clock convention": angles start from the +Y axis
     * and grow clockwise. This means that 0 corresponds to \f$\pi/2\f$ radians,
     * 90 to 0 radians, 180 to \f$-\pi/2\f$ radians, and 270 to \f$\pi\f$ radians.
     * @return A number in the range [0, 360).
     */
    Coord degreesClock() const {
        Coord ret = 90.0 - _angle * (180.0 / M_PI);
        if (ret < 0) ret += 360;
        return ret;
    }
    /** @brief Create an angle from its measure in radians. */
    static Angle from_radians(Coord d) {
        Angle a(d);
        return a;
    }
    /** @brief Create an angle from its measure in degrees. */
    static Angle from_degrees(Coord d) {
        Angle a(d * (M_PI / 180.0));
        return a;
    }
    /** @brief Create an angle from its measure in degrees in clock convention.
     * @see Angle::degreesClock() */
    static Angle from_degrees_clock(Coord d) {
        // first make sure d is in [0, 360)
        d = std::fmod(d, 360.0);
        if (d < 0) d += 360.0;
        Coord rad = M_PI/2 - d * (M_PI / 180.0);
        if (rad < 0) rad += 2*M_PI;
        Angle a;
        a._angle = rad;
        return a;
    }
private:

    void _normalize() {
        _angle -= floor(_angle * (1.0/(2*M_PI))) * 2*M_PI;
    }
    Coord _angle; // this is always in [0, 2pi)
    friend class AngleInterval;
};

/** @brief Directed angular interval.
 *
 * Wrapper for directed angles with defined start and end values. Useful e.g. for representing
 * the portion of an ellipse in an elliptical arc. Both extreme angles are contained
 * in the interval (it is a closed interval). Angular intervals can also be interptered
 * as functions \f$f: [0, 1] \to [-\pi, \pi)\f$, which return the start angle for 0,
 * the end angle for 1, and interpolate linearly for other values. Note that such functions
 * are not continuous if the interval contains the zero angle.
 *
 * This class is immutable - you cannot change the values of start and end angles
 * without creating a new instance of this class.
 *
 * @ingroup Primitives
 */
class AngleInterval {
public:
    AngleInterval(Angle const &s, Angle const &e, bool cw = false)
        : _start_angle(s), _end_angle(e), _sweep(cw)
    {}
    AngleInterval(double s, double e, bool cw = false)
        : _start_angle(s), _end_angle(e), _sweep(cw)
    {}
    /** @brief Get the angular coordinate of the interval's initial point
     * @return Angle in range \f$[0,2\pi)\f$ corresponding to the start of arc */
    Angle const &initialAngle() const { return _start_angle; }
    /** @brief Get the angular coordinate of the interval's final point
     * @return Angle in range \f$[0,2\pi)\f$ corresponding to the end of arc */
    Angle const &finalAngle() const { return _end_angle; }
    bool isDegenerate() const { return initialAngle() == finalAngle(); }
    /** @brief Get an angle corresponding to the specified time value. */
    Angle angleAt(Coord t) const {
        Coord span = extent();
        Angle ret = _start_angle.radians0() + span * (_sweep ? t : -t);
        return ret;
    }
    Angle operator()(Coord t) const { return angleAt(t); }
#if 0
    /** @brief Find an angle nearest to the specified time value.
     * @param a Query angle
     * @return If the interval contains the query angle, a number from the range [0, 1]
     *         such that a = angleAt(t); otherwise, 0 or 1, depending on which extreme
     *         angle is nearer. */
    Coord nearestAngle(Angle const &a) const {
        Coord dist = distance(_start_angle, a, _sweep).radians0();
        Coord span = distance(_start_angle, _end_angle, _sweep).radians0();
        if (dist <= span) return dist / span;
        else return distance_abs(_start_angle, a).radians0() > distance_abs(_end_angle, a).radians0() ? 1.0 : 0.0;
    }
#endif
    /** @brief Check whether the interval includes the given angle. */
    bool contains(Angle const &a) const {
        Coord s = _start_angle.radians0();
        Coord e = _end_angle.radians0();
        Coord x = a.radians0();
        if (_sweep) {
            if (s < e) return x >= s && x <= e;
            return x >= s || x <= e;
        } else {
            if (s > e) return x <= s && x >= e;
            return x <= s || x >= e;
        }
    }
    /** @brief Extent of the angle interval.
     * @return Extent in range \f$[0, 2\pi)\f$ */
    Coord extent() const {
        Coord d = _end_angle - _start_angle;
        if (!_sweep) d = -d;
        if (d < 0) d += 2*M_PI;
        return d;
    }
protected:
    AngleInterval() {}
    Angle _start_angle;
    Angle _end_angle;
    bool _sweep;
};

/** @brief Given an angle in degrees, return radians
 * @relates Angle */
inline Coord deg_to_rad(Coord deg) { return deg*M_PI/180.0;}
/** @brief Given an angle in radians, return degrees
 * @relates Angle */
inline Coord rad_to_deg(Coord rad) { return rad*180.0/M_PI;}

/*
 *  start_angle and angle must belong to [0, 2PI[
 *  and angle must belong to the cirsular arc defined by
 *  start_angle, end_angle and with rotation direction cw
 */
inline
double map_circular_arc_on_unit_interval( double angle, double start_angle, double end_angle, bool cw = true )
{
    double d = end_angle - start_angle;
    double t = angle - start_angle;
    if ( !cw )
    {
    	d = -d;
    	t = -t;
    }
    d = std::fmod(d, 2*M_PI);
    t = std::fmod(t, 2*M_PI);
    if ( d < 0 ) d += 2*M_PI;
    if ( t < 0 ) t += 2*M_PI;
    return t / d;
}

inline
Coord map_unit_interval_on_circular_arc(Coord t, double start_angle, double end_angle, bool cw = true)
{
	double sweep_angle = end_angle - start_angle;
	if ( !cw ) sweep_angle = -sweep_angle;
	sweep_angle = std::fmod(sweep_angle, 2*M_PI);
	if ( sweep_angle < 0 ) sweep_angle += 2*M_PI;

	Coord angle = start_angle;
    if ( cw )
    {
        angle += sweep_angle * t;
    }
    else
    {
        angle -= sweep_angle * t;
    }
    angle = std::fmod(angle, 2*M_PI);
    if (angle < 0) angle += 2*M_PI;
    return angle;
}

/*
 *  Return true if the given angle is contained in the circular arc determined
 *  by the passed angles.
 *
 *  a:     the angle to be tested
 *  sa:    the angle the arc start from
 *  ia:    an angle strictly inner to the arc
 *  ea:    the angle the arc end to
 *
 *  prerequisite: the inner angle has to be not equal (mod 2PI) to the start
 *                or the end angle, except when they are equal each other, too.
 *  warning: when sa == ea (mod 2PI) they define a whole circle
 *           if ia != sa (mod 2PI), on the contrary if ia == sa == ea (mod 2PI)
 *           they define a single point.
 */
inline
bool arc_contains (double a, double sa, double ia, double ea)
{
    a -= sa;
    a = std::fmod(a, 2*M_PI);
    if (a < 0) a += 2*M_PI;
    ia -= sa;
    ia = std::fmod(ia, 2*M_PI);
    if (ia < 0) ia += 2*M_PI;
    ea -= sa;
    ea = std::fmod(ea, 2*M_PI);
    if (ea < 0) ea += 2*M_PI;

    if (ia == 0 && ea == 0)  return (a == 0);
    if (ia == 0 || ia == ea)
    {
        THROW_RANGEERROR ("arc_contains: passed angles do not define an arc");
    }
    return (ia < ea && a <= ea) || (ia > ea && (a >= ea || a == 0));
}

} // end namespace Geom

#endif // LIB2GEOM_SEEN_ANGLE_H

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
