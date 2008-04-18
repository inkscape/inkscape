#ifndef __2GEOM_SWEEP_H__
#define __2GEOM_SWEEP_H__

#include <vector>
#include "d2.h"

namespace Geom {

struct Event {
    double x;
    unsigned ix;
    bool closing;
    Event(double pos, unsigned i, bool c) : x(pos), ix(i), closing(c) {}
// Lexicographic ordering by x then closing
    bool operator<(Event const &other) const {
        if(x < other.x) return true;
        if(x > other.x) return false;
        return closing < other.closing;
    }

};
std::vector<std::vector<unsigned> > sweep_bounds(std::vector<Rect>);
std::vector<std::vector<unsigned> > sweep_bounds(std::vector<Rect>, std::vector<Rect>);

std::vector<std::vector<unsigned> > fake_cull(unsigned a, unsigned b);

}

#endif

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