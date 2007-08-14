#include "quadtree.h"

Quad* QuadTree::search(double x0, double y0, double x1, double y1) {
    Quad *q = root;
        
    double bxx0 = bx1, bxx1 = bx1;
    double byy0 = by1, byy1 = by1;
    while(q) {
        double cx = (bxx0 + bxx1)/2;
        double cy = (byy0 + byy1)/2;
        unsigned i = 0;
        if(x0 >= cx) {
            i += 1;
            bxx0 = cx; // zoom in a quad
        } else if(x1 <= cx) {
            bxx1 = cx;
        } else
            break;
        if(y0 >= cy) {
            i += 2;
            byy0 = cy;
        } else if(y1 <= cy) {
            byy1 = cy;
        } else
            break;
        
        assert(i < 4);
        Quad *qq = q->children[i];
        if(qq == 0) break; // last non-null
        q = qq;
    }
    return q;
}
    
void QuadTree::insert(double x0, double y0, double x1, double y1, int shape) {
    // loop until a quad would break the box.
    if(root == 0) {
        root = new Quad;
            
        bx0 = 0;
        bx1 = 1;
        by0 = 0;
        by1 = 1;
    }
    Quad *q = root;
    
    double bxx0 = bx0, bxx1 = bx1;
    double byy0 = by0, byy1 = by1;
    while((bxx0 > x0) ||
          (bxx1 < x1) ||
          (byy0 > y0) ||
          (byy1 < y1)) { // too small initial size - double
        unsigned i = 0;
        if(bxx0 > x0) {
            bxx0 = 2*bxx0 - bxx1;
            i += 1;
        } else {
            bxx1 = 2*bxx1 - bxx0;
        }
        if(byy0 > y0) {
            byy0 = 2*byy0 - byy1;
            i += 2;
        } else {
            byy1 = 2*byy1 - byy0;
        }
        q = new Quad;
        q->children[i] = root;
        root = q;
        bx0 = bxx0;
        bx1 = bxx1;
        by0 = byy0;
        by1 = byy1;
    }
    
    while(q) {
        double cx = (bxx0 + bxx1)/2;
        double cy = (byy0 + byy1)/2;
        unsigned i = 0;
        assert(x0 >= bxx0);
        assert(x1 <= bxx1);
        assert(y0 >= byy0);
        assert(y1 <= byy1);
        if(x0 >= cx) {
            i += 1;
            bxx0 = cx; // zoom in a quad
        } else if(x1 <= cx) {
            bxx1 = cx;
        } else
            break;
        if(y0 >= cy) {
            i += 2;
            byy0 = cy;
        } else if(y1 <= cy) {
            byy1 = cy;
        } else
            break;
            
        assert(i < 4);
        Quad *qq = q->children[i];
        if(qq == 0) {
            qq = new Quad;
            q->children[i] = qq;
        }
        q = qq;
    }
    q->data.push_back(shape);
}
void QuadTree::erase(Quad *q, int shape) {
    for(Quad::iterator i = q->data.begin();  i != q->data.end(); i++) {
        if(*i == shape) {
            q->data.erase(i);
            if(q->data.empty()) {

            }
        }
    }
    return;
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(substatement-open . 0))
  indent-tabs-mode:nil
  c-brace-offset:0
  fill-column:99
  End:
  vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/

