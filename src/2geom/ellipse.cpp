/** @file
 * @brief Ellipse shape
 *//*
 * Authors:
 *   Marco Cecchetti <mrcekets at gmail.com>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright 2008-2014 Authors
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

#include <2geom/ellipse.h>
#include <2geom/elliptical-arc.h>
#include <2geom/numeric/fitting-tool.h>
#include <2geom/numeric/fitting-model.h>

namespace Geom {

Ellipse::Ellipse(Geom::Circle const &c)
    : _center(c.center())
    , _rays(c.radius(), c.radius())
    , _angle(0)
{}

void Ellipse::setCoefficients(double A, double B, double C, double D, double E, double F)
{
    double den = 4*A*C - B*B;
    if (den == 0) {
        THROW_RANGEERROR("den == 0, while computing ellipse centre");
    }
    _center[X] = (B*E - 2*C*D) / den;
    _center[Y] = (B*D - 2*A*E) / den;

    // evaluate the a coefficient of the ellipse equation in normal form
    // E(x,y) = a*(x-cx)^2 + b*(x-cx)*(y-cy) + c*(y-cy)^2 = 1
    // where b = a*B , c = a*C, (cx,cy) == centre
    double num =   A * sqr(_center[X])
                 + B * _center[X] * _center[Y]
                 + C * sqr(_center[Y])
                 - F;


    //evaluate ellipse rotation angle
    _angle = std::atan2( -B, -(A - C) )/2;

    // evaluate the length of the ellipse rays
    double sinrot, cosrot;
    sincos(_angle, sinrot, cosrot);
    double cos2 = cosrot * cosrot;
    double sin2 = sinrot * sinrot;
    double cossin = cosrot * sinrot;

    den = A * cos2 + B * cossin + C * sin2;
    if (den == 0) {
        THROW_RANGEERROR("den == 0, while computing 'rx' coefficient");
    }
    double rx2 = num / den;
    if (rx2 < 0) {
        THROW_RANGEERROR("rx2 < 0, while computing 'rx' coefficient");
    }
    _rays[X] = std::sqrt(rx2);

    den = C * cos2 - B * cossin + A * sin2;
    if (den == 0) {
        THROW_RANGEERROR("den == 0, while computing 'ry' coefficient");
    }
    double ry2 = num / den;
    if (ry2 < 0) {
        THROW_RANGEERROR("ry2 < 0, while computing 'rx' coefficient");
    }
    _rays[Y] = std::sqrt(ry2);

    // the solution is not unique so we choose always the ellipse
    // with a rotation angle between 0 and PI/2
    makeCanonical();
}

Point Ellipse::initialPoint() const
{
    Coord sinrot, cosrot;
    sincos(_angle, sinrot, cosrot);
    Point p(ray(X) * cosrot + center(X), ray(X) * sinrot + center(Y));
    return p;
}


Affine Ellipse::unitCircleTransform() const
{
    Affine ret = Scale(ray(X), ray(Y)) * Rotate(_angle);
    ret.setTranslation(center());
    return ret;
}

Affine Ellipse::inverseUnitCircleTransform() const
{
    if (ray(X) == 0 || ray(Y) == 0) {
        THROW_RANGEERROR("a degenerate ellipse doesn't have an inverse unit circle transform");
    }
    Affine ret = Translate(-center()) * Rotate(-_angle) * Scale(1/ray(X), 1/ray(Y));
    return ret;
}


LineSegment Ellipse::axis(Dim2 d) const
{
    Point a(0, 0), b(0, 0);
    a[d] = -1;
    b[d] = 1;
    LineSegment ls(a, b);
    ls.transform(unitCircleTransform());
    return ls;
}

LineSegment Ellipse::semiaxis(Dim2 d, int sign) const
{
    Point a(0, 0), b(0, 0);
    b[d] = sgn(sign);
    LineSegment ls(a, b);
    ls.transform(unitCircleTransform());
    return ls;
}

Rect Ellipse::boundsExact() const
{
    Angle extremes[2][2];
    double sinrot, cosrot;
    sincos(_angle, sinrot, cosrot);

    extremes[X][0] = std::atan2( -ray(Y) * sinrot, ray(X) * cosrot );
    extremes[X][1] = extremes[X][0] + M_PI;
    extremes[Y][0] = std::atan2( ray(Y) * cosrot, ray(X) * sinrot );
    extremes[Y][1] = extremes[Y][0] + M_PI;

    Rect result;
    for (unsigned d = 0; d < 2; ++d) {
        result[d] = Interval(valueAt(extremes[d][0], d ? Y : X),
                             valueAt(extremes[d][1], d ? Y : X));
    }
    return result;
}

std::vector<double> Ellipse::coefficients() const
{
    std::vector<double> c(6);
    coefficients(c[0], c[1], c[2], c[3], c[4], c[5]);
    return c;
}

void Ellipse::coefficients(Coord &A, Coord &B, Coord &C, Coord &D, Coord &E, Coord &F) const
{
    if (ray(X) == 0 || ray(Y) == 0) {
        THROW_RANGEERROR("a degenerate ellipse doesn't have an implicit form");
    }

    double cosrot, sinrot;
    sincos(_angle, sinrot, cosrot);
    double cos2 = cosrot * cosrot;
    double sin2 = sinrot * sinrot;
    double cossin = cosrot * sinrot;
    double invrx2 = 1 / (ray(X) * ray(X));
    double invry2 = 1 / (ray(Y) * ray(Y));

    A = invrx2 * cos2 + invry2 * sin2;
    B = 2 * (invrx2 - invry2) * cossin;
    C = invrx2 * sin2 + invry2 * cos2;
    D = -2 * A * center(X) - B * center(Y);
    E = -2 * C * center(Y) - B * center(X);
    F =   A * center(X) * center(X)
        + B * center(X) * center(Y)
        + C * center(Y) * center(Y)
        - 1;
}


void Ellipse::fit(std::vector<Point> const &points)
{
    size_t sz = points.size();
    if (sz < 5) {
        THROW_RANGEERROR("fitting error: too few points passed");
    }
    NL::LFMEllipse model;
    NL::least_squeares_fitter<NL::LFMEllipse> fitter(model, sz);

    for (size_t i = 0; i < sz; ++i) {
        fitter.append(points[i]);
    }
    fitter.update();

    NL::Vector z(sz, 0.0);
    model.instance(*this, fitter.result(z));
}


EllipticalArc *
Ellipse::arc(Point const &ip, Point const &inner, Point const &fp)
{
    // This is resistant to degenerate ellipses:
    // both flags evaluate to false in that case.

    bool large_arc_flag = false;
    bool sweep_flag = false;

    // Determination of large arc flag:
    // large_arc is false when the inner point is on the same side
    // of the center---initial point line as the final point, AND
    // is on the same side of the center---final point line as the
    // initial point.
    // Additionally, large_arc is always false when we have exactly
    // 1/2 of an arc, i.e. the cross product of the center -> initial point
    // and center -> final point vectors is zero.
    // Negating the above leads to the condition for large_arc being true.
    Point fv = fp - _center;
    Point iv = ip - _center;
    Point innerv = inner - _center;
    double ifcp = cross(fv, iv);

    if (ifcp != 0 && (sgn(cross(fv, innerv)) != sgn(ifcp) ||
                      sgn(cross(iv, innerv)) != sgn(-ifcp)))
    {
        large_arc_flag = true;
    }

    //cross(-iv, fv) && large_arc_flag
    

    // Determination of sweep flag:
    // For clarity, let's assume that Y grows up. Then the cross product
    // is positive for points on the left side of a vector and negative
    // on the right side of a vector.
    //
    //     cross(?, v) > 0
    //  o------------------->
    //     cross(?, v) < 0
    //
    // If the arc is small (large_arc_flag is false) and the final point
    // is on the right side of the vector initial point -> center,
    // we have to go in the direction of increasing angles
    // (counter-clockwise) and the sweep flag is true.
    // If the arc is large, the opposite is true, since we have to reach
    // the final point going the long way - in the other direction.
    // We can express this observation as:
    // cross(_center - ip, fp - _center) < 0 xor large_arc flag
    // This is equal to:
    // cross(-iv, fv) < 0 xor large_arc flag
    // But cross(-iv, fv) is equal to cross(fv, iv) due to antisymmetry
    // of the cross product, so we end up with the condition below.
    if ((ifcp < 0) ^ large_arc_flag) {
        sweep_flag = true;
    }

    EllipticalArc *ret_arc = new EllipticalArc(ip, ray(X), ray(Y), rotationAngle(),
                                               large_arc_flag, sweep_flag, fp);
    return ret_arc;
}

Ellipse &Ellipse::operator*=(Rotate const &r)
{
    _angle += r.angle();
    _center *= r;
    return *this;
}

Ellipse &Ellipse::operator*=(Affine const& m)
{
    Affine a = Scale(ray(X), ray(Y)) * Rotate(_angle);
    Affine mwot = m.withoutTranslation();
    Affine am = a * mwot;
    Point new_center = _center * m;

    if (are_near(am.descrim(), 0)) {
        double angle;
        if (am[0] != 0) {
            angle = std::atan2(am[2], am[0]);
        } else if (am[1] != 0) {
            angle = std::atan2(am[3], am[1]);
        } else {
            angle = M_PI/2;
        }
        Point v = Point::polar(angle) * am;
        _center = new_center;
        _rays[X] = L2(v);
        _rays[Y] = 0;
        _angle = atan2(v);
        return *this;
    }

    std::vector<double> coeff = coefficients();
    Affine q( coeff[0],   coeff[1]/2,
              coeff[1]/2, coeff[2],
              0,          0   );

    Affine invm = mwot.inverse();
    q = invm * q ;
    std::swap(invm[1], invm[2]);
    q *= invm;
    setCoefficients(q[0], 2*q[1], q[3], 0, 0, -1);
    _center = new_center;

    return *this;
}

Ellipse Ellipse::canonicalForm() const
{
    Ellipse result(*this);
    result.makeCanonical();
    return result;
}

void Ellipse::makeCanonical()
{
    if (_rays[X] == _rays[Y]) {
        _angle = 0;
        return;
    }

    if (_angle < 0) {
        _angle += M_PI;
    }
    if (_angle >= M_PI/2) {
        std::swap(_rays[X], _rays[Y]);
        _angle -= M_PI/2;
    }
}

Point Ellipse::pointAt(Coord t) const
{
    Point p = Point::polar(t);
    p *= unitCircleTransform();
    return p;
}

Coord Ellipse::valueAt(Coord t, Dim2 d) const
{
    Coord sinrot, cosrot, cost, sint;
    sincos(rotationAngle(), sinrot, cosrot);
    sincos(t, sint, cost);

    if ( d == X ) {
        return    ray(X) * cosrot * cost
                - ray(Y) * sinrot * sint
                + center(X);
    } else {
        return    ray(X) * sinrot * cost
                + ray(Y) * cosrot * sint
                + center(Y);
    }
}

Coord Ellipse::timeAt(Point const &p) const
{
    // degenerate ellipse is basically a reparametrized line segment
    if (ray(X) == 0 || ray(Y) == 0) {
        if (ray(X) != 0) {
            return asin(Line(axis(X)).timeAt(p));
        } else if (ray(Y) != 0) {
            return acos(Line(axis(Y)).timeAt(p));
        } else {
            return 0;
        }
    }
    Affine iuct = inverseUnitCircleTransform();
    return Angle(atan2(p * iuct)).radians0(); // return a value in [0, 2pi)
}

Point Ellipse::unitTangentAt(Coord t) const
{
    Point p = Point::polar(t + M_PI/2);
    p *= unitCircleTransform().withoutTranslation();
    p.normalize();
    return p;
}

bool Ellipse::contains(Point const &p) const
{
    Point tp = p * inverseUnitCircleTransform();
    return tp.length() <= 1;
}

std::vector<ShapeIntersection> Ellipse::intersect(Line const &line) const
{

    std::vector<ShapeIntersection> result;

    if (line.isDegenerate()) return result;
    if (ray(X) == 0 || ray(Y) == 0) {
        // TODO intersect with line segment.
        return result;
    }

    // Ax^2 + Bxy + Cy^2 + Dx + Ey + F
    Coord A, B, C, D, E, F;
    coefficients(A, B, C, D, E, F);
    Affine iuct = inverseUnitCircleTransform();

    // generic case
    Coord a, b, c;
    line.coefficients(a, b, c);
    Point lv = line.vector();

    if (fabs(lv[X]) > fabs(lv[Y])) {
        // y = -a/b x - c/b
        Coord q = -a/b;
        Coord r = -c/b;

        // substitute that into the ellipse equation, making it quadratic in x
        Coord I = A + B*q + C*q*q;          // x^2 terms
        Coord J = B*r + C*2*q*r + D + E*q;  // x^1 terms
        Coord K = C*r*r + E*r + F;          // x^0 terms
        std::vector<Coord> xs = solve_quadratic(I, J, K);

        for (unsigned i = 0; i < xs.size(); ++i) {
            Point p(xs[i], q*xs[i] + r);
            result.push_back(ShapeIntersection(atan2(p * iuct), line.timeAt(p), p));
        }
    } else {
        Coord q = -b/a;
        Coord r = -c/a;

        Coord I = A*q*q + B*q + C;
        Coord J = A*2*q*r + B*r + D*q + E;
        Coord K = A*r*r + D*r + F;
        std::vector<Coord> xs = solve_quadratic(I, J, K);

        for (unsigned i = 0; i < xs.size(); ++i) {
            Point p(q*xs[i] + r, xs[i]);
            result.push_back(ShapeIntersection(atan2(p * iuct), line.timeAt(p), p));
        }
    }
    return result;
}

std::vector<ShapeIntersection> Ellipse::intersect(LineSegment const &seg) const
{
    // we simply re-use the procedure for lines and filter out
    // results where the line time value is outside of the unit interval.
    std::vector<ShapeIntersection> result = intersect(Line(seg));
    filter_line_segment_intersections(result);
    return result;
}

std::vector<ShapeIntersection> Ellipse::intersect(Ellipse const &other) const
{
    // handle degenerate cases first
    if (ray(X) == 0 || ray(Y) == 0) {
        
    }
    // intersection of two ellipses can be solved analytically.
    // http://maptools.home.comcast.net/~maptools/BivariateQuadratics.pdf

    Coord A, B, C, D, E, F;
    Coord a, b, c, d, e, f;

    // NOTE: the order of coefficients is different to match the convention in the PDF above
    // Ax^2 + Bx^2 + Cx + Dy + Exy + F
    this->coefficients(A, E, B, C, D, F);
    other.coefficients(a, e, b, c, d, f);

    // Assume that Q is the ellipse equation given by uppercase letters
    // and R is the equation given by lowercase ones. An intersection exists when
    // there is a coefficient mu such that
    // mu Q + R = 0
    //
    // This can be written in the following way:
    //
    //                    |  ff  cc/2 dd/2 | |1|
    // mu Q + R = [1 x y] | cc/2  aa  ee/2 | |x| = 0
    //                    | dd/2 ee/2  bb  | |y|
    //
    // where aa = mu A + a and so on. The determinant can be explicitly written out,
    // giving an equation which is cubic in mu and can be solved analytically.

    Coord I, J, K, L;
    I = (-E*E*F + 4*A*B*F + C*D*E - A*D*D - B*C*C) / 4;
    J = -((E*E - 4*A*B) * f + (2*E*F - C*D) * e + (2*A*D - C*E) * d +
          (2*B*C - D*E) * c + (C*C - 4*A*F) * b + (D*D - 4*B*F) * a) / 4;
    K = -((e*e - 4*a*b) * F + (2*e*f - c*d) * E + (2*a*d - c*e) * D +
          (2*b*c - d*e) * C + (c*c - 4*a*f) * B + (d*d - 4*b*f) * A) / 4;
    L = (-e*e*f + 4*a*b*f + c*d*e - a*d*d - b*c*c) / 4;

    std::vector<Coord> mus = solve_cubic(I, J, K, L);
    Coord mu = infinity();
    std::vector<ShapeIntersection> result;

    // Now that we have solved for mu, we need to check whether the conic
    // determined by mu Q + R is reducible to a product of two lines. If it's not,
    // it means that there are no intersections. If it is, the intersections of these
    // lines with the original ellipses (if there are any) give the coordinates
    // of intersections.

    // Prefer middle root if there are three.
    // Out of three possible pairs of lines that go through four points of intersection
    // of two ellipses, this corresponds to cross-lines. These intersect the ellipses
    // at less shallow angles than the other two options.
    if (mus.size() == 3) {
        std::swap(mus[1], mus[0]);
    }
    for (unsigned i = 0; i < mus.size(); ++i) {
        Coord aa = mus[i] * A + a;
        Coord bb = mus[i] * B + b;
        Coord ee = mus[i] * E + e;
        Coord delta = ee*ee - 4*aa*bb;
        if (delta < 0) continue;
        mu = mus[i];
        break;
    }

    // if no suitable mu was found, there are no intersections
    if (mu == infinity()) return result;

    Coord aa = mu * A + a;
    Coord bb = mu * B + b;
    Coord cc = mu * C + c;
    Coord dd = mu * D + d;
    Coord ee = mu * E + e;
    Coord ff = mu * F + f;

    Line lines[2];

    if (aa != 0) {
        bb /= aa; cc /= aa; dd /= aa; ee /= aa; /*ff /= aa;*/
        Coord s = (ee + std::sqrt(ee*ee - 4*bb)) / 2;
        Coord q = ee - s;
        Coord alpha = (dd - cc*q) / (s - q);
        Coord beta = cc - alpha;

        lines[0] = Line(1, q, alpha);
        lines[1] = Line(1, s, beta);
    } else if (bb != 0) {
        cc /= bb; /*dd /= bb;*/ ee /= bb; ff /= bb;
        Coord s = ee;
        Coord q = 0;
        Coord alpha = cc / ee;
        Coord beta = ff * ee / cc;

        lines[0] = Line(q, 1, alpha);
        lines[1] = Line(s, 1, beta);
    } else {
        lines[0] = Line(ee, 0, dd);
        lines[1] = Line(0, 1, cc/ee);
    }

    // intersect with the obtained lines and report intersections
    for (unsigned li = 0; li < 2; ++li) {
        std::vector<ShapeIntersection> as = intersect(lines[li]);
        std::vector<ShapeIntersection> bs = other.intersect(lines[li]);

        if (!as.empty() && as.size() == bs.size()) {
            for (unsigned i = 0; i < as.size(); ++i) {
                ShapeIntersection ix(as[i].first, bs[i].first,
                    middle_point(as[i].point(), bs[i].point()));
                result.push_back(ix);
            }
        }
    }
    return result;
}

std::vector<ShapeIntersection> Ellipse::intersect(D2<Bezier> const &b) const
{
    Coord A, B, C, D, E, F;
    coefficients(A, B, C, D, E, F);

    Bezier x = A*b[X]*b[X] + B*b[X]*b[Y] + C*b[Y]*b[Y] + D*b[X] + E*b[Y] + F;
    std::vector<Coord> r = x.roots();

    std::vector<ShapeIntersection> result;
    for (unsigned i = 0; i < r.size(); ++i) {
        Point p = b.valueAt(r[i]);
        result.push_back(ShapeIntersection(timeAt(p), r[i], p));
    }
    return result;
}

bool Ellipse::operator==(Ellipse const &other) const
{
    if (_center != other._center) return false;

    Ellipse a = this->canonicalForm();
    Ellipse b = other.canonicalForm();

    if (a._rays != b._rays) return false;
    if (a._angle != b._angle) return false;

    return true;
}


bool are_near(Ellipse const &a, Ellipse const &b, Coord precision)
{
    // We want to know whether no point on ellipse a is further than precision
    // from the corresponding point on ellipse b. To check this, we compute
    // the four extreme points at the end of each ray for each ellipse
    // and check whether they are sufficiently close.

    // First, we need to correct the angles on the ellipses, so that they are
    // no further than M_PI/4 apart. This can always be done by rotating
    // and exchanging axes.
    Ellipse ac = a, bc = b;
    if (distance(ac.rotationAngle(), bc.rotationAngle()).radians0() >= M_PI/2) {
        ac.setRotationAngle(ac.rotationAngle() + M_PI);
    }
    if (distance(ac.rotationAngle(), bc.rotationAngle()) >= M_PI/4) {
        Angle d1 = distance(ac.rotationAngle() + M_PI/2, bc.rotationAngle());
        Angle d2 = distance(ac.rotationAngle() - M_PI/2, bc.rotationAngle());
        Coord adj = d1.radians0() < d2.radians0() ? M_PI/2 : -M_PI/2;
        ac.setRotationAngle(ac.rotationAngle() + adj);
        ac.setRays(ac.ray(Y), ac.ray(X));
    }

    // Do the actual comparison by computing four points on each ellipse.
    Point tps[] = {Point(1,0), Point(0,1), Point(-1,0), Point(0,-1)};
    for (unsigned i = 0; i < 4; ++i) {
        if (!are_near(tps[i] * ac.unitCircleTransform(),
                      tps[i] * bc.unitCircleTransform(),
                      precision))
            return false;
    }
    return true;
}

std::ostream &operator<<(std::ostream &out, Ellipse const &e)
{
    out << "Ellipse(" << e.center() << ", " << e.rays()
        << ", " << format_coord_nice(e.rotationAngle()) << ")";
    return out;
}

}  // end namespace Geom


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


