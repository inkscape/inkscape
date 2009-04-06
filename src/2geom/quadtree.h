/**
 * \file
 * \brief  \todo brief description
 *
 * Authors:
 *      ? <?@?.?>
 * 
 * Copyright ?-?  authors
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 *
 */

#include <vector>
#include <cassert>

#include <2geom/d2.h>

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
private:
    bool clean_root();
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
