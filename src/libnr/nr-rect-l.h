#ifndef SEEN_NR_RECT_L_H
#define SEEN_NR_RECT_L_H

#include <libnr/nr-i-coord.h>
#include <boost/optional.hpp>
#include <libnr/nr-rect.h>
#include <libnr/nr-point-l.h>

struct NRRectL {
    boost::optional<NR::Rect> upgrade() const;
    NR::ICoord x0, y0, x1, y1;
};


namespace NR {


class IRect {
public:
    IRect(const NRRectL& r) : _min(r.x0, r.y0), _max(r.x1, r.y1) {}
    IRect(const IRect& r) : _min(r._min), _max(r._max) {}
    IRect(const IPoint &p0, const IPoint &p1) : _min(p0), _max(p1) {}
    
    /** as not all Rects are representable by IRects this gives the smallest IRect that contains
     * r. */
    IRect(const Rect& r);
    
    operator Rect() {
        return Rect(Point(_min), Point(_max));
    }

    const IPoint &min() const { return _min; }
    const IPoint &max() const { return _max; }
    
    /** returns a vector from min to max. */
    IPoint dimensions() const;
    
    /** does this rectangle have zero area? */
    bool isEmpty() const {
        return isEmpty<X>() && isEmpty<Y>();
    }
    
    bool intersects(const IRect &r) const {
        return intersects<X>(r) && intersects<Y>(r);
    }
    bool contains(const IRect &r) const {
        return contains<X>(r) && contains<Y>(r);
    }
    bool contains(const IPoint &p) const {
        return contains<X>(p) && contains<Y>(p);
    }
    
    ICoord maxExtent() const {
        return MAX(extent<X>(), extent<Y>());
    }

    ICoord extent(Dim2 axis) const {
        switch (axis) {
        case X: return extent<X>();
        case Y: return extent<Y>();
        };
    }

    ICoord extent(unsigned i) const throw(std::out_of_range) {
        switch (i) {
        case 0: return extent<X>();
        case 1: return extent<Y>();
        default: throw std::out_of_range("Dimension out of range");
        };
    }

    /** Translates the rectangle by p. */
    void offset(IPoint p);
	
    /** Makes this rectangle large enough to include the point p. */
    void expandTo(IPoint p);

    /** Makes this rectangle large enough to include the rectangle r. */
    void expandTo(const IRect &r);
	
    /** Returns the set of points shared by both rectangles. */
    static boost::optional<IRect> intersection(const IRect &a, const IRect &b);

    /** Returns the smallest rectangle that encloses both rectangles. */
    static IRect union_bounds(const IRect &a, const IRect &b);

    bool operator==(const IRect &other) const {
        return (min() == other.min()) && (max() == other.max());
    }

    bool operator!=(const IRect &other) const {
        return (min() != other.min()) || (max() != other.max());
    }

private:
    IRect() {}

    template <NR::Dim2 axis>
    ICoord extent() const {
        return _max[axis] - _min[axis];
    }

    template <Dim2 axis>
    bool isEmpty() const {
        return !( _min[axis] < _max[axis] );
    }

    template <Dim2 axis>
    bool intersects(const IRect &r) const {
        return _max[axis] >= r._min[axis] && _min[axis] <= r._max[axis];
    }

    template <Dim2 axis>
    bool contains(const IRect &r) const {
        return contains(r._min) && contains(r._max);
    }

    template <Dim2 axis>
    bool contains(const IPoint &p) const {
        return p[axis] >= _min[axis] && p[axis] <= _max[axis];
    }

    IPoint _min, _max;
};



}  // namespace NR

#endif /* !SEEN_NR_RECT_L_H */

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
