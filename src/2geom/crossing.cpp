#include "crossing.h"

namespace Geom {

void merge_crossings(Crossings &a, Crossings &b, unsigned i) {
    Crossings n;
    sort_crossings(b, i);
    n.resize(a.size() + b.size());
    std::merge(a.begin(), a.end(), b.begin(), b.end(), n.begin(), CrossingOrder(i));
    a = n;
}

void offset_crossings(Crossings &cr, double a, double b) {
    for(unsigned i = 0; i < cr.size(); i++) {
        cr[i].ta += a;
        cr[i].tb += b;
    }
}

Crossings reverse_ta(Crossings const &cr, std::vector<double> max) {
    Crossings ret;
    for(Crossings::const_iterator i = cr.begin(); i != cr.end(); ++i) {
        double mx = max[i->a];
        ret.push_back(Crossing(i->ta > mx+0.01 ? (1 - (i->ta - mx) + mx) : mx - i->ta,
                               i->tb, !i->dir));
    }
    return ret;
}

Crossings reverse_tb(Crossings const &cr, unsigned split, std::vector<double> max) {
    Crossings ret;
    for(Crossings::const_iterator i = cr.begin(); i != cr.end(); ++i) {
        double mx = max[i->b - split];
        std::cout << i->b << "\n";
        ret.push_back(Crossing(i->ta, i->tb > mx+0.01 ? (1 - (i->tb - mx) + mx) : mx - i->tb,
                               !i->dir));
    }
    return ret;
}

CrossingSet reverse_ta(CrossingSet const &cr, unsigned split, std::vector<double> max) {
    CrossingSet ret;
    for(unsigned i = 0; i < cr.size(); i++) {
        Crossings res = reverse_ta(cr[i], max);
        if(i < split) std::reverse(res.begin(), res.end());
        ret.push_back(res);
    }
    return ret;
}

CrossingSet reverse_tb(CrossingSet const &cr, unsigned split, std::vector<double> max) {
    CrossingSet ret;
    for(unsigned i = 0; i < cr.size(); i++) {
        Crossings res = reverse_tb(cr[i], split, max);
        if(i >= split) std::reverse(res.begin(), res.end());
        ret.push_back(res);
    }
    return ret;
}

void clean(Crossings &cr_a, Crossings &cr_b) {
/*    if(cr_a.empty()) return;
    
    //Remove anything with dupes
    
    for(Eraser<Crossings> i(&cr_a); !i.ended(); i++) {
        const Crossing cur = *i;
        Eraser<Crossings> next(i);
        next++;
        if(near(cur, *next)) {
            cr_b.erase(std::find(cr_b.begin(), cr_b.end(), cur));
            for(i = next; near(*i, cur); i++) {
                cr_b.erase(std::find(cr_b.begin(), cr_b.end(), *i));
            }
            continue;
        }
    }
*/
}

}
