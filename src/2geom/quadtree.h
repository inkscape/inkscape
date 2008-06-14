#include <vector>
#include <cassert>

#include "d2.h"

namespace Geom{

class Quad{
public:
    Quad* children[4];
    std::vector<int> data;
    Quad() {
        for(int i = 0; i < 4; i++)
            children[i] = 0;
    }
    typedef std::vector<int>::iterator iterator;
    Rect bounds(unsigned i, double x, double y, double d) {
        double dd = d/2;
        switch(i % 4) {
            case 0:
                return Rect(Interval(x, x+dd), Interval(y, y+dd));
            case 1:
                return Rect(Interval(x+dd, x+d), Interval(y, y+dd));
            case 2:
                return Rect(Interval(x, x+dd), Interval(y+dd, y+d));
            case 3:
                return Rect(Interval(x+dd, x+d), Interval(y+dd, y+d));
            default: 
                /* just to suppress warning message
                 * this case should be never reached */
                assert(false);
        }        
    }
};

class QuadTree{
public:
    Quad* root;
    double scale;
    double bx0, bx1;
    double by0, by1;

    QuadTree() : root(0), scale(1) {}

    Quad* search(double x0, double y0, double x1, double y1);
    void insert(double x0, double y0, double x1, double y1, int shape);
    Quad* search(Geom::Rect const &r);
    void insert(Geom::Rect const &r, int shape);
    void erase(Quad *q, int shape);
};

};

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
