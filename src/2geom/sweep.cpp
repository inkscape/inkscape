#include "sweep.h"

#include <algorithm>

namespace Geom {

std::vector<std::vector<unsigned> > sweep_bounds(std::vector<Rect> rs) {
    std::vector<Event> events; events.reserve(rs.size()*2);
    std::vector<std::vector<unsigned> > pairs(rs.size());

    for(unsigned i = 0; i < rs.size(); i++) {
        events.push_back(Event(rs[i].left(), i, false));
        events.push_back(Event(rs[i].right(), i, true));
    }
    std::sort(events.begin(), events.end());

    std::vector<unsigned> open;
    for(unsigned i = 0; i < events.size(); i++) {
        unsigned ix = events[i].ix;
        if(events[i].closing) {
            std::vector<unsigned>::iterator iter = std::find(open.begin(), open.end(), ix);
            //if(iter != open.end())
            open.erase(iter);
        } else {
            for(unsigned j = 0; j < open.size(); j++) {
                unsigned jx = open[j];
                if(rs[jx][Y].intersects(rs[ix][Y])) {
                    pairs[jx].push_back(ix);
                }
            }
            open.push_back(ix);
        }
    }
    return pairs;
}

std::vector<std::vector<unsigned> > sweep_bounds(std::vector<Rect> a, std::vector<Rect> b) {
    std::vector<std::vector<unsigned> > pairs(a.size());
    if(a.empty() || b.empty()) return pairs;
    std::vector<Event> events[2];
    events[0].reserve(a.size()*2);
    events[1].reserve(b.size()*2);

    for(unsigned n = 0; n < 2; n++) {
        unsigned sz = n ? b.size() : a.size();
        events[n].reserve(sz*2);
        for(unsigned i = 0; i < sz; i++) {
            events[n].push_back(Event(n ? b[i].left() : a[i].left(), i, false));
            events[n].push_back(Event(n ? b[i].right() : a[i].right(), i, true));
        }
        std::sort(events[n].begin(), events[n].end());
    }

    std::vector<unsigned> open[2];
    bool n = events[1].front() < events[0].front();
    for(unsigned i[] = {0,0}; i[n] < events[n].size();) {
        unsigned ix = events[n][i[n]].ix;
        bool closing = events[n][i[n]].closing;
        //std::cout << n << "[" << ix << "] - " << (closing ? "closer" : "opener") << "\n";
        if(closing) {
            open[n].erase(std::find(open[n].begin(), open[n].end(), ix));
        } else {
            if(n) {
                //n = 1
                //opening a B, add to all open a
                for(unsigned j = 0; j < open[0].size(); j++) {
                    unsigned jx = open[0][j];
                    if(a[jx][Y].intersects(b[ix][Y])) {
                        pairs[jx].push_back(ix);
                    }
                }
            } else {
                //n = 0
                //opening an A, add all open b
                for(unsigned j = 0; j < open[1].size(); j++) {
                    unsigned jx = open[1][j];
                    if(b[jx][Y].intersects(a[ix][Y])) {
                        pairs[ix].push_back(jx);
                    }
                }
            }
            open[n].push_back(ix);
        }
        i[n]++;
        n = (events[!n][i[!n]] < events[n][i[n]]) ? !n : n;
    }
    return pairs;
}

//Fake cull, until the switch to the real sweep is made.
std::vector<std::vector<unsigned> > fake_cull(unsigned a, unsigned b) {
    std::vector<std::vector<unsigned> > ret;

    std::vector<unsigned> all;
    for(unsigned j = 0; j < b; j++)
        all.push_back(j);

    for(unsigned i = 0; i < a; i++)
        ret.push_back(all);

    return ret;
}

}
