#include <2geom/sweep-bounds.h>

#include <algorithm>

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
    bool operator==(Event const &other) const {
        return other.x == x && other.ix == ix && other.closing == closing;
    }
};

std::vector<std::vector<unsigned> > fake_cull(unsigned a, unsigned b);

/**
 * \brief Make a list of pairs of self intersections in a list of Rects.
 * 
 * \param rs: vector of Rect.
 * \param d: dimension to sweep along
 *
 * [(A = rs[i], B = rs[j]) for i,J in enumerate(pairs) for j in J]
 * then A.left <= B.left
 */

std::vector<std::vector<unsigned> > sweep_bounds(std::vector<Rect> rs, Dim2 d) {
    std::vector<Event> events; events.reserve(rs.size()*2);
    std::vector<std::vector<unsigned> > pairs(rs.size());

    for(unsigned i = 0; i < rs.size(); i++) {
        events.push_back(Event(rs[i][d].min(), i, false));
        events.push_back(Event(rs[i][d].max(), i, true));
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
                if(rs[jx][1-d].intersects(rs[ix][1-d])) {
                    pairs[jx].push_back(ix);
                }
            }
            open.push_back(ix);
        }
    }
    return pairs;
}

/**
 * \brief Make a list of pairs of red-blue intersections between two lists of Rects.
 * 
 * \param a: vector of Rect.
 * \param b: vector of Rect.
 * \param d: dimension to scan along
 *
 * [(A = rs[i], B = rs[j]) for i,J in enumerate(pairs) for j in J]
 * then A.left <= B.left, A in a, B in b
 */
std::vector<std::vector<unsigned> > sweep_bounds(std::vector<Rect> a, std::vector<Rect> b, Dim2 d) {
    std::vector<std::vector<unsigned> > pairs(a.size());
    if(a.empty() || b.empty()) return pairs;
    std::vector<Event> events[2];
    events[0].reserve(a.size()*2);
    events[1].reserve(b.size()*2);

    for(unsigned n = 0; n < 2; n++) {
        unsigned sz = n ? b.size() : a.size();
        events[n].reserve(sz*2);
        for(unsigned i = 0; i < sz; i++) {
            Rect r = n ? b[i] : a[i];
            events[n].push_back(Event(r[d].min(), i, false));
            events[n].push_back(Event(r[d].max(), i, true));
        }
        std::sort(events[n].begin(), events[n].end());
    }

    std::vector<unsigned> open[2];
    bool n = events[1].front() < events[0].front();
    {// As elegant as putting the initialiser in the for was, it upsets some legacy compilers (MS VS C++)
    unsigned i[] = {0,0}; 
    for(; i[n] < events[n].size();) {
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
                    if(a[jx][1-d].intersects(b[ix][1-d])) {
                        pairs[jx].push_back(ix);
                    }
                }
            } else {
                //n = 0
                //opening an A, add all open b
                for(unsigned j = 0; j < open[1].size(); j++) {
                    unsigned jx = open[1][j];
                    if(b[jx][1-d].intersects(a[ix][1-d])) {
                        pairs[ix].push_back(jx);
                    }
                }
            }
            open[n].push_back(ix);
        }
        i[n]++;
	if(i[n]>=events[n].size()) {break;}
        n = (events[!n][i[!n]] < events[n][i[n]]) ? !n : n;
    }}
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
