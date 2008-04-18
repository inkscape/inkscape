#include <vector>
#include <cassert>

class Quad{
public:
    Quad* children[4];
    std::vector<int> data;
    Quad() {
        for(int i = 0; i < 4; i++)
            children[i] = 0;
    }
    typedef std::vector<int>::iterator iterator;
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
    void erase(Quad *q, int shape);
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
