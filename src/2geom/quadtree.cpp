#include <2geom/quadtree.h>

namespace Geom{
Quad* QuadTree::search(Rect const &r) {
    return search(r[0].min(), r[1].min(),
                  r[0].max(), r[1].max());
}

void QuadTree::insert(Rect const &r, int shape) {
    insert(r[0].min(), r[1].min(),
           r[0].max(), r[1].max(), shape);
}


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


/*
Comments by Vangelis (use with caution :P )

Insert Rect (x0, y0), (x1, y1) in the QuadTree Q.

===================================================================================
* QuadTree Q has: Quadtree's Quad root R, QuadTree's bounding box B. 

* Each Quad has a Quad::data where we store the id of the Rect that belong to 
this Quad. (In reality we'll store a pointer to the shape).

* Each Quad has 4 Quad children: 0, 1, 2, 3. Each child Quad represents one of the following quarters
of the bounding box B:

+---------------------+
|          |          |
|  NW=0    |  NE=1    |
|          |          |
|          |          |
+---------------------+
|          |          |
|  SW=2    |  SE=3    |
|          |          |
|          |          |
+---------------------+ 

Each Quad can further be divided in 4 Quads as above and so on. Below there is an example 
 
       Root
      / || \
    /  /  \  \
   0  1   2   3
     /\
  / | | \
  0 1 2 3

+---------------------+
|          | 1-0 | 1-1|
|    0     |     |    |
|          |-----|----|
|          | 1-2 | 1-3|
|          |     |    |
+---------------------+
|          |          |
|          |          |
|     2    |     3    |
|          |          |
+---------------------+ 



===================================================================================
Insert Rect (x0, y0), (x1, y1) in the QuadTree Q. Algorithm:
1) check if Rect is bigger than QuadTree's bounding box
2) find in which Quad we should add the Rect:



-----------------------------------------------------------------------------------
How we find in which Quad we should add the Rect R:

Q = Quadtree's Quad root
B = QuadTree's bounding box B
WHILE (Q) {
    IF ( Rect cannot fit in one unique quarter of B ){
        Q = current Quad ;
        BREAK;
    }
    IF ( Rect can fit in the quarter I ) {
        IF (Q.children[I] doesn't exist) {
            create the Quad Q.children[I];
        }
        B = bounding box of the Quad Q.children[I] ;
        Q = Q.children[I] ;
        CHECK(R, B) ;
    }
}
add Rect R to Q ;


*/
    
void QuadTree::insert(double x0, double y0, double x1, double y1, int shape) {
    // loop until a quad would break the box.

    // empty root => empty QuadTree. Create initial bounding box (0,0), (1,1)
    if(root == 0) {
        root = new Quad;
            
        bx0 = 0;
        bx1 = 1;
        by0 = 0;
        by1 = 1;
    }
    Quad *q = root;

    //A temp bounding box. Same as root's bounting box (ie of the whole QuadTree)
    double bxx0 = bx0, bxx1 = bx1;
    double byy0 = by0, byy1 = by1;

    while((bxx0 > x0) ||
          (bxx1 < x1) ||
          (byy0 > y0) ||
          (byy1 < y1)) { 
        // QuadTree has small size, can't accomodate new rect. Double the size:
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
        //check if root is empty (no rects, no quad children)
        if( clean_root() ){
            root = q;
        }
        else{
            q->children[i] = root;
            root = q;
        }
        bx0 = bxx0;
        bx1 = bxx1;
        by0 = byy0;
        by1 = byy1;
    }

    while(q) {
        // Find the center of the temp bounding box
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
        } else{
            // rect does not fit in one unique quarter (in X axis) of the temp bounding box
            break;
        }
        if(y0 >= cy) {
            i += 2;
            byy0 = cy;
        } else if(y1 <= cy) {
            byy1 = cy;
        } else{
            // rect does not fit in one unique quarter (in Y axis) of the temp bounding box
            break;
        }

        // check if rect's bounding box has size 1x1. This means that rect is defined by 2 points
        // that are in the same place.
        if( ( fabs(bxx0 - bxx1) < 1.0 ) && ( fabs(byy0 - byy1) < 1.0 )){
            bxx0 = floor(bxx0);
            bxx1 = floor(bxx1);
            byy0 = floor(byy0);
            byy1 = floor(byy1);
            break;
        }

        /*
        1 rect does fit in one unique quarter of the temp bounding box. And we have found which.
        2 temp bounding box = bounding box of this quarter. 
        3 "Go in" this quarter (create if doesn't exist)
        */
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
Returns:
false:  if root isn't empty
true:   if root is empty it cleans root
*/
bool QuadTree::clean_root() { 
    assert(root);

    // false if root *has* rects assigned to it.
    bool all_clean = root->data.empty(); 

    // if root has children we get false
    for(unsigned i = 0; i < 4; i++)
    {
        if(root->children[i])
        {
            all_clean = false;
        }
    }

    if(all_clean)
    {
        delete root;
        root=0;
        return true;
    }
    return false;
}

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
