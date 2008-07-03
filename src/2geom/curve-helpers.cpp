/*
 * 
 * Authors:
 * 		MenTaLguY <mental@rydia.net>
 * 		Marco Cecchetti <mrcekets at gmail.com>
 * 
 * Copyright 2007-2008  authors
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
 */


#include <2geom/curve.h>
#include <2geom/ord.h>


namespace Geom 
{

int CurveHelpers::root_winding(Curve const &c, Point p) {
    std::vector<double> ts = c.roots(p[Y], Y);

    if(ts.empty()) return 0;

    double const fudge = 0.01; //fudge factor used on first and last

    std::sort(ts.begin(), ts.end());

    // winding determined by crossings at roots
    int wind=0;
    // previous time
    double pt = ts.front() - fudge;
    for ( std::vector<double>::iterator ti = ts.begin()
        ; ti != ts.end()
        ; ++ti )
    {
        double t = *ti;
        if ( t <= 0. || t >= 1. ) continue; //skip endpoint roots 
        if ( c.valueAt(t, X) > p[X] ) { // root is ray intersection
            // Get t of next:
            std::vector<double>::iterator next = ti;
            next++;
            double nt;
            if(next == ts.end()) nt = t + fudge; else nt = *next;
            
            // Check before in time and after in time for positions
            // Currently we're using the average times between next and previous segs
            Cmp after_to_ray =  cmp(c.valueAt((t + nt) / 2, Y), p[Y]);
            Cmp before_to_ray = cmp(c.valueAt((t + pt) / 2, Y), p[Y]);
            // if y is included, these will have opposite values, giving order.
            Cmp dt = cmp(after_to_ray, before_to_ray);
            if(dt != EQUAL_TO) //Should always be true, but yah never know..
                wind += dt;
            pt = t;
        }
    }
    
    return wind;
}


}  // end namespace Geom

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
