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
 * This class holds only a single floating point value, so passing it by value will generally
 * be faster than passing it by const reference.
 *
 * @ingroup Primitives
 */
class Angle
    : boost::additive< Angle
    , boost::additive< Angle, Coord
    , boost::equality_comparable< Angle
    , boost::equality_comparable< Angle, Coord
      > > > >
{
public:
    Angle() : _angle(0) {}
    Angle(Coord v) : _angle(v) { _normalize(); } // this can be called implicitly
    explicit Angle(Point const &p) : _angle(atan2(p)) { _normalize(); }
    Angle(Point const &a, Point const &b) : _angle(angle_between(a, b)) { _normalize(); }
    operator Coord() const { return radians(); }
    Angle &operator+=(Angle o) {
        _angle += o._angle;
        _normalize();
        return *this;
    }
    Angle &operator-=(Angle o) {
        _angle -= o._angle;
        _normalize();
        return *this;
    }
    Angle &operator+=(Coord a) {
        *this += Angle(a);
        return *this;
    }
    Angle &operator-=(Coord a) {
        *this -= Angle(a);
        return *this;
    }
    bool operator==(Angle o) const {
        return _angle == o._angle;
    }
    bool operator==(Coord c) const {
        return _angle == Angle(c)._angle;
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
        _angle = std::fmod(_angle, 2*M_PI);
        if (_angle < 0) _angle += 2*M_PI;
        //_angle -= floor(_angle * (1.0/(2*M_PI))) * 2*M_PI;
    }
    Coord _angle; // this is always in [0, 2pi)
    friend class AngleInterval;
};

inline Angle distance(Angle const &a, Angle const &b) {
    // the distance cannot be larger than M_PI.
    Coord ac = a.radians0();
    Coord bc = b.radians0();
    Coord d = fabs(ac - bc);
    return Angle(d > M_PI ? 2*M_PI - d : d);
}

/** @brief Directed angular interval.
 *
 * Wrapper for directed angles with defined start and end values. Useful e.g. for representing
 * the portion of an ellipse in an elliptical arc. Both extreme angles are contained
 * in the interval (it is a closed interval). Angular intervals can also be interptered
 * as functions \f$f: [0, 1] \to [-\pi, \pi)\f$, which return the start angle for 0,
 * the end angle for 1, and interpolate linearly for other values. Note that such functions
 * are not continuous if the interval crosses the angle \f$\pi\f$.
 *
 * This class can represent all directed angular intervals, including empty ones.
 * However, not all possible intervals can be created with the constructors.
 * For full control, use the setInitial(), setFinal() and setAngles() methods.
 *
 * @ingroup Primitives
 */
class AngleInterval
    : boost::equality_comparable< AngleInterval >
{
public:
    AngleInterval() {}
    /** @brief Create an angular interval from two angles and direction.
     * If the initial and final angle are the same, a degenerate interval
     * (containing only one angle) will be created.
     * @param s Starting angle
     * @param e Ending angle
     * @param cw Which direction the interval goes. True means that it goes
     *   in the direction of increasing angles, while false means in the direction
     *   of decreasing angles. */
    AngleInterval(Angle s, Angle e, bool cw = false)
        : _start_angle(s), _end_angle(e), _sweep(cw), _full(false)
    {}
    AngleInterval(double s, double e, bool cw = false)
        : _start_angle(s), _end_angle(e), _sweep(cw), _full(false)
    {}
    /** @brief Create an angular interval from three angles.
     * If the inner angle is exactly equal to initial or final angle,
     * the sweep flag will be set to true, i.e. the interval will go
     * in the direction of increasing angles.
     *
     * If the initial and final angle are the same, but the inner angle
     * is different, a full angle in the direction of increasing angles
     * will be created.
     *
     * @param s Initial angle
     * @param inner Angle contained in the interval
     * @param e Final angle */
    AngleInterval(Angle s, Angle inner, Angle e)
        : _start_angle(s)
        , _end_angle(e)
        , _sweep((inner-s).radians0() <= (e-s).radians0())
        , _full(s == e && s != inner)
    {
        if (_full) {
            _sweep = true;
        }
    }

    /// Get the start angle.
    Angle initialAngle() const { return _start_angle; }
    /// Get the end angle.
    Angle finalAngle() const { return _end_angle; }
    /// Check whether the interval goes in the direction of increasing angles.
    bool sweep() const { return _sweep; }
    /// Check whether the interval contains only a single angle.
    bool isDegenerate() const {
        return _start_angle == _end_angle && !_full;
    }
    /// Check whether the interval contains all angles.
    bool isFull() const {
        return _start_angle == _end_angle && _full;
    }

    /** @brief Set the initial angle.
     * @param a Angle to set
     * @param prefer_full Whether to set a full angular interval when
     * the initial angle is set to the final angle */
    void setInitial(Angle a, bool prefer_full = false) {
        _start_angle = a;
        _full = prefer_full && a == _end_angle;
    }

    /** @brief Set the final angle.
     * @param a Angle to set
     * @param prefer_full Whether to set a full angular interval when
     * the initial angle is set to the final angle */
    void setFinal(Angle a, bool prefer_full = false) {
        _end_angle = a;
        _full = prefer_full && a == _start_angle;
    }
    /** @brief Set both angles at once.
     * The direction (sweep flag) is left unchanged.
     * @param s Initial angle
     * @param e Final angle
     * @param prefer_full Whether to set a full interval when the passed
     * initial and final angle are the same */
    void setAngles(Angle s, Angle e, bool prefer_full = false) {
        _start_angle = s;
        _end_angle = e;
        _full = prefer_full && s == e;
    }
    /// Set whether the interval goes in the direction of increasing angles.
    void setSweep(bool s) { _sweep = s; }

    /// Reverse the direction of the interval while keeping contained values the same.
    void reverse() {
        using std::swap;
        swap(_start_angle, _end_angle);
        _sweep = !_sweep;
    }
    /// Get a new interval with reversed direction.
    AngleInterval reversed() const {
        AngleInterval result(*this);
        result.reverse();
        return result;
    }

    /// Get an angle corresponding to the specified time value.
    Angle angleAt(Coord t) const {
        Coord span = extent();
        Angle ret = _start_angle.radians0() + span * (_sweep ? t : -t);
        return ret;
    }
    Angle operator()(Coord t) const { return angleAt(t); }

    /** @brief Compute a time value that would evaluate to the given angle.
     * If the start and end angle are exactly the same, NaN will be returned.
     * Negative values will be returned for angles between the initial angle
     * and the angle exactly opposite the midpoint of the interval. */
    Coord timeAtAngle(Angle a) const {
        if (_full) {
            Angle ta = _sweep ? a - _start_angle : _start_angle - a;
            return ta.radians0() / (2*M_PI);
        }
        Coord ex = extent();
        Coord outex = 2*M_PI - ex;
        if (_sweep) {
            Angle midout = _start_angle - outex / 2;
            Angle acmp = a - midout, scmp = _start_angle - midout;
            if (acmp.radians0() >= scmp.radians0()) {
                return (a - _start_angle).radians0() / ex;
            } else {
                return -(_start_angle - a).radians0() / ex;
            }
        } else {
            Angle midout = _start_angle + outex / 2;
            Angle acmp = a - midout, scmp = _start_angle - midout;
            if (acmp.radians0() <= scmp.radians0()) {
                return (_start_angle - a).radians0() / ex;
            } else {
                return -(a - _start_angle).radians0() / ex;
            }
        }
    }

    /// Check whether the interval includes the given angle.
    bool contains(Angle a) const {
        if (_full) return true;
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
     * Equivalent to the absolute value of the sweep angle.
     * @return Extent in range \f$[0, 2\pi)\f$. */
    Coord extent() const {
        if (_full) return 2*M_PI;
        return _sweep
            ? (_end_angle - _start_angle).radians0()
            : (_start_angle - _end_angle).radians0();
    }
    /** @brief Get the sweep angle of the interval.
     * This is the value you need to add to the initial angle to get the final angle.
     * It is positive when sweep is true. Denoted as \f$\Delta\theta\f$ in the SVG
     * elliptical arc implementation notes. */
    Coord sweepAngle() const {
        if (_full) return _sweep ? 2*M_PI : -2*M_PI;
        Coord sa = _end_angle.radians0() - _start_angle.radians0();
        if (_sweep && sa < 0) sa += 2*M_PI;
        if (!_sweep && sa > 0) sa -= 2*M_PI;
        return sa;
    }

    /// Check another interval for equality.
    bool operator==(AngleInterval const &other) const {
        if (_start_angle != other._start_angle) return false;
        if (_end_angle != other._end_angle) return false;
        if (_sweep != other._sweep) return false;
        if (_full != other._full) return false;
        return true;
    }

    static AngleInterval create_full(Angle start, bool sweep = true) {
        AngleInterval result;
        result._start_angle = result._end_angle = start;
        result._sweep = sweep;
        result._full = true;
        return result;
    }

private:
    Angle _start_angle;
    Angle _end_angle;
    bool _sweep;
    bool _full;
};

/** @brief Given an angle in degrees, return radians
 * @relates Angle */
inline Coord rad_from_deg(Coord deg) { return deg*M_PI/180.0;}
/** @brief Given an angle in radians, return degrees
 * @relates Angle */
inline Coord deg_from_rad(Coord rad) { return rad*180.0/M_PI;}

} // end namespace Geom

namespace std {
template <> class iterator_traits<Geom::Angle> {};
}

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
