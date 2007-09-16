#ifndef __GEOM_PATH_INTERSECTION_H
#define __GEOM_PATH_INTERSECTION_H

#include "path.h"

#include "crossing.h"

#include "sweep.h"

namespace Geom {

int winding(Path const &path, Point p);
bool path_direction(Path const &p);

inline bool contains(Path const & p, Point i, bool evenodd = true) {
    return (evenodd ? winding(p, i) % 2 : winding(p, i)) != 0;
}

template<typename T>
Crossings curve_sweep(Path const &a, Path const &b) {
    T t;
    Crossings ret;
    std::vector<Rect> bounds_a = bounds(a), bounds_b = bounds(b);
    std::vector<std::vector<unsigned> > ixs = sweep_bounds(bounds_a, bounds_b);
    for(unsigned i = 0; i < a.size(); i++) {
        for(std::vector<unsigned>::iterator jp = ixs[i].begin(); jp != ixs[i].end(); jp++) {
            Crossings cc = t.crossings(a[i], b[*jp]);
            offset_crossings(cc, i, *jp);
            ret.insert(ret.end(), cc.begin(), cc.end());
        }
    }
    return ret;
}

struct SimpleCrosser : public Crosser<Path> {
    Crossings crossings(Curve const &a, Curve const &b);
    Crossings crossings(Path const &a, Path const &b) { return curve_sweep<SimpleCrosser>(a, b); }
    CrossingSet crossings(std::vector<Path> const &a, std::vector<Path> const &b) { return Crosser<Path>::crossings(a, b); }
};

struct MonoCrosser : public Crosser<Path> {
    Crossings crossings(Path const &a, Path const &b) { return crossings(std::vector<Path>(1,a), std::vector<Path>(1,b))[0]; }
    CrossingSet crossings(std::vector<Path> const &a, std::vector<Path> const &b);
};

typedef SimpleCrosser DefaultCrosser;

std::vector<double> path_mono_splits(Path const &p);

CrossingSet crossings_among(std::vector<Path> const & p);
Crossings self_crossings(Path const & a);

inline Crossings crossings(Path const & a, Path const & b) {
    DefaultCrosser c = DefaultCrosser();
    return c.crossings(a, b);
}

inline CrossingSet crossings(std::vector<Path> const & a, std::vector<Path> const & b) {
    DefaultCrosser c = DefaultCrosser();
    return c.crossings(a, b);
}

}

#endif
