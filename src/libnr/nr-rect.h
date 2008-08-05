#ifndef LIBNR_NR_RECT_H_SEEN
#define LIBNR_NR_RECT_H_SEEN

/** \file
 * Definitions of NRRect and NR::Rect types, and some associated functions \& macros.
 */
/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Nathan Hurst <njh@mail.csse.monash.edu.au>
 *   MenTaLguY <mental@rydia.net>
 *
 * This code is in public domain
 */


#include <stdexcept>
#include <limits>

#include "libnr/nr-values.h"
#include <libnr/nr-coord.h>
#include <libnr/nr-i-coord.h>
#include <libnr/nr-dim2.h>
#include <libnr/nr-point.h>
#include <boost/optional.hpp>
#include <libnr/nr-point-matrix-ops.h>
#include <libnr/nr-forward.h>

namespace NR {

/** A rectangle is always aligned to the X and Y axis.  This means it
 * can be defined using only 4 coordinates, and determining
 * intersection is very efficient.  The points inside a rectangle are
 * min[dim] <= _pt[dim] <= max[dim].  A rectangle may be empty, in the
 * sense of having zero area, but it will always contain at least one
 * point.  Infinities are also permitted.
 */
class Rect {
public:
    Rect() : _min(-_inf(), -_inf()), _max(_inf(), _inf()) {}
    Rect(Point const &p0, Point const &p1);

    Point const &min() const { return _min; }
    Point const &max() const { return _max; }

    /** returns the four corners of the rectangle in order
     *  (clockwise if +Y is up, anticlockwise if +Y is down) */
    Point corner(unsigned i) const;

    /** returns a vector from min to max. */
    Point dimensions() const;

    /** returns the midpoint of this rect. */
    Point midpoint() const;

    Point cornerFarthestFrom(Point const &p) const;

    /** True iff either width or height is less than \a epsilon. */
    bool isEmpty(double epsilon=1e-6) const {
        return isEmpty<X>(epsilon) || isEmpty<Y>(epsilon);
    }

    bool intersects(Rect const &r) const {
        return intersects<X>(r) && intersects<Y>(r);
    }
    bool contains(Rect const &r) const {
        return contains<X>(r) && contains<Y>(r);
    }
    bool contains(Point const &p) const {
        return contains<X>(p) && contains<Y>(p);
    }

    double area() const {
        return extent<X>() * extent<Y>();
    }

    double maxExtent() const {
        return MAX(extent<X>(), extent<Y>());
    }

    double extent(Dim2 const axis) const {
        switch (axis) {
            case X: return extent<X>();
            case Y: return extent<Y>();
            default: g_error("invalid axis value %d", (int) axis); return 0;
        };
    }

    double extent(unsigned i) const throw(std::out_of_range) {
        switch (i) {
            case 0: return extent<X>();
            case 1: return extent<Y>();
            default: throw std::out_of_range("Dimension out of range");
        };
    }

    /**
        \brief  Remove some precision from the Rect
        \param  places  The number of decimal places left in the end

        This function just calls round on the \c _min and \c _max points.
    */
    inline void round(int places = 0) {
        _min.round(places);
        _max.round(places);
        return;
    }

    /** Translates the rectangle by p. */
    void offset(Point p);

    /** Makes this rectangle large enough to include the point p. */
    void expandTo(Point p);

    /** Makes this rectangle large enough to include the rectangle r. */
    void expandTo(Rect const &r);

    inline void move_left (gdouble by) {
        _min[NR::X] += by;
    }
    inline void move_right (gdouble by) {
        _max[NR::X] += by;
    }
    inline void move_top (gdouble by) {
        _min[NR::Y] += by;
    }
    inline void move_bottom (gdouble by) {
        _max[NR::Y] += by;
    }

    void growBy (gdouble by);

    /** Scales the rect by s, with origin at 0, 0 */
    inline Rect operator*(double const s) const {
        return Rect(s * min(), s * max());
    }

    /** Transforms the rect by m. Note that it gives correct results only for scales and translates */
    inline Rect operator*(Matrix const m) const {
        return Rect(_min * m, _max * m);
    }

    inline bool operator==(Rect const &in_rect) {
        return ((this->min() == in_rect.min()) && (this->max() == in_rect.max()));
    }

    friend inline std::ostream &operator<<(std::ostream &out_file, NR::Rect const &in_rect);

private:
//    Rect(Nothing) : _min(1, 1), _max(-1, -1) {}

    static double _inf() {
        return std::numeric_limits<double>::infinity();
    }

    template <NR::Dim2 axis>
    double extent() const {
        return _max[axis] - _min[axis];
    }

    template <NR::Dim2 axis>
    bool isEmpty(double epsilon) const {
        return extent<axis>() < epsilon;
    }

    template <Dim2 axis>
    bool intersects(Rect const &r) const {
        return _max[axis] >= r._min[axis] && _min[axis] <= r._max[axis];
    }

    template <Dim2 axis>
    bool contains(Rect const &r) const {
        return contains(r._min) && contains(r._max);
    }

    template <Dim2 axis>
    bool contains(Point const &p) const {
        return p[axis] >= _min[axis] && p[axis] <= _max[axis];
    }

    Point _min, _max;

    friend boost::optional<Rect> intersection(boost::optional<Rect> const &, boost::optional<Rect> const &);
    friend Rect union_bounds(Rect const &, Rect const &);
};

/** Returns the set of points shared by both rectangles. */
boost::optional<Rect> intersection(boost::optional<Rect> const & a, boost::optional<Rect> const & b);

/** Returns the smallest rectangle that encloses both rectangles. */
Rect union_bounds(Rect const &a, Rect const &b);
inline Rect union_bounds(boost::optional<Rect> const & a, Rect const &b) {
    if (a) {
        return union_bounds(*a, b);
    } else {
        return b;
    }
}
inline Rect union_bounds(Rect const &a, boost::optional<Rect> const & b) {
    if (b) {
        return union_bounds(a, *b);
    } else {
        return a;
    }
}
inline boost::optional<Rect> union_bounds(boost::optional<Rect> const & a, boost::optional<Rect> const & b)
{
    if (!a) {
        return b;
    } else if (!b) {
        return a;
    } else {
        return union_bounds(*a, *b);
    }
}

/** A function to print out the rectange if sent to an output
    stream. */
inline std::ostream
&operator<<(std::ostream &out_file, NR::Rect const &in_rect)
{
    out_file << "Rectangle:\n";
    out_file << "\tMin Point -> " << in_rect.min() << "\n";
    out_file << "\tMax Point -> " << in_rect.max() << "\n";

    return out_file;
}

} /* namespace NR */

/* legacy rect stuff */

/* NULL rect is infinite */

struct NRRect {
    NRRect()
    : x0(0), y0(0), x1(0), y1(0)
    {}
    NRRect(NR::Coord xmin, NR::Coord ymin, NR::Coord xmax, NR::Coord ymax)
    : x0(xmin), y0(ymin), x1(xmax), y1(ymax)
    {}
    explicit NRRect(NR::Rect const &rect);
    explicit NRRect(boost::optional<NR::Rect> const &rect);
    operator boost::optional<NR::Rect>() const { return upgrade(); }
    boost::optional<NR::Rect> upgrade() const;

    NR::Coord x0, y0, x1, y1;
};

#define nr_rect_d_set_empty(r) (*(r) = NR_RECT_EMPTY)
#define nr_rect_l_set_empty(r) (*(r) = NR_RECT_L_EMPTY)

/** "Empty" here includes the case of zero width or zero height. */
// TODO convert to static overloaded functions (pointer and ref) once performance can be tested:
#define nr_rect_d_test_empty_ptr(r) ((r) && NR_RECT_DFLS_TEST_EMPTY(r))
#define nr_rect_d_test_empty(r) NR_RECT_DFLS_TEST_EMPTY_REF(r)

// TODO convert to static overloaded functions (pointer and ref) once performance can be tested:
#define nr_rect_l_test_empty_ptr(r) ((r) && NR_RECT_DFLS_TEST_EMPTY(r))
#define nr_rect_l_test_empty(r) NR_RECT_DFLS_TEST_EMPTY_REF(r)

#define nr_rect_d_test_intersect(r0,r1) \
        (!nr_rect_d_test_empty(r0) && !nr_rect_d_test_empty(r1) && \
         !((r0) && (r1) && !NR_RECT_DFLS_TEST_INTERSECT(r0, r1)))

// TODO convert to static overloaded functions (pointer and ref) once performance can be tested:
#define nr_rect_l_test_intersect_ptr(r0,r1) \
        (!nr_rect_l_test_empty_ptr(r0) && !nr_rect_l_test_empty_ptr(r1) && \
         !((r0) && (r1) && !NR_RECT_DFLS_TEST_INTERSECT(r0, r1)))
#define nr_rect_l_test_intersect(r0,r1) \
        (!nr_rect_l_test_empty(r0) && !nr_rect_l_test_empty(r1) && \
         !(!NR_RECT_DFLS_TEST_INTERSECT_REF(r0, r1)))

#define nr_rect_d_point_d_test_inside(r,p) ((p) && (!(r) || (!NR_RECT_DF_TEST_EMPTY(r) && NR_RECT_DF_POINT_DF_TEST_INSIDE(r,p))))
#define nr_rect_l_point_l_test_inside(r,p) ((p) && (!(r) || (!NR_RECT_DFLS_TEST_EMPTY(r) && NR_RECT_LS_POINT_LS_TEST_INSIDE(r,p))))
#define nr_rect_l_test_inside(r,x,y) ((!(r) || (!NR_RECT_DFLS_TEST_EMPTY(r) && NR_RECT_LS_TEST_INSIDE(r,x,y))))

// returns minimal rect which covers all of r0 not covered by r1
NRRectL *nr_rect_l_subtract(NRRectL *d, NRRectL const *r0, NRRectL const *r1);

// returns the area of r
NR::ICoord nr_rect_l_area(NRRectL *r);

/* NULL values are OK for r0 and r1, but not for d */
NRRect *nr_rect_d_intersect(NRRect *d, NRRect const *r0, NRRect const *r1);
NRRectL *nr_rect_l_intersect(NRRectL *d, NRRectL const *r0, NRRectL const *r1);

NRRect *nr_rect_d_union(NRRect *d, NRRect const *r0, NRRect const *r1);
NRRectL *nr_rect_l_union(NRRectL *d, NRRectL const *r0, NRRectL const *r1);

NRRect *nr_rect_union_pt(NRRect *dst, NR::Point const &p);
NRRect *nr_rect_d_union_xy(NRRect *d, NR::Coord x, NR::Coord y);
NRRectL *nr_rect_l_union_xy(NRRectL *d, NR::ICoord x, NR::ICoord y);

NRRect *nr_rect_d_matrix_transform(NRRect *d, NRRect const *s, NR::Matrix const &m);
NRRect *nr_rect_d_matrix_transform(NRRect *d, NRRect const *s, NR::Matrix const *m);
NRRectL *nr_rect_l_enlarge(NRRectL *d, int amount);


#endif /* !LIBNR_NR_RECT_H_SEEN */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
