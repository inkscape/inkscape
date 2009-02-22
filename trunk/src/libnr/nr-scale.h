#ifndef SEEN_NR_SCALE_H
#define SEEN_NR_SCALE_H
#include <libnr/nr-point.h>
#include <libnr/nr-point-ops.h>

namespace NR {

class scale {
private:
    Point _p;

private:
    scale();

public:
    explicit scale(Point const &p) : _p(p) {}
    scale(double const x, double const y) : _p(x, y) {}
    explicit scale(double const s) : _p(s, s) {}
    inline Coord operator[](Dim2 const d) const { return _p[d]; }
    inline Coord operator[](unsigned const d) const { return _p[d]; }
    inline Coord &operator[](Dim2 const d) { return _p[d]; }
    inline Coord &operator[](unsigned const d) { return _p[d]; }

    bool operator==(scale const &o) const {
        return _p == o._p;
    }

    bool operator!=(scale const &o) const {
        return _p != o._p;
    }
    
    scale inverse() const {
        return scale(1/_p[0], 1/_p[1]);
    }
    
    NR::Point point() const {
        return _p;    
    }
};

} /* namespace NR */


#endif /* !SEEN_NR_SCALE_H */

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
