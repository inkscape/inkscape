/*
 * d2.cpp - Lifts one dimensional objects into 2d 
 *
 * Copyright 2007 Michael Sloan <mgsloan@gmail.com>
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
 * in the file COPYING-LGPL-2.1; if not, output to the Free Software
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

#include "d2.h"

namespace Geom {

SBasis L2(D2<SBasis> const & a, unsigned k) { return sqrt(dot(a, a), k); }
double L2(D2<double> const & a) { return hypot(a[0], a[1]); }

D2<SBasis> multiply(Linear const & a, D2<SBasis> const & b) {
    return D2<SBasis>(multiply(a, b[X]), multiply(a, b[Y]));
}

D2<SBasis> multiply(SBasis const & a, D2<SBasis> const & b) {
    return D2<SBasis>(multiply(a, b[X]), multiply(a, b[Y]));
}

D2<SBasis> truncate(D2<SBasis> const & a, unsigned terms) {
    return D2<SBasis>(truncate(a[X], terms), truncate(a[Y], terms));
}

unsigned sbasis_size(D2<SBasis> const & a) {
    return std::max((unsigned) a[0].size(), (unsigned) a[1].size());
}

//TODO: Is this sensical? shouldn't it be like pythagorean or something?
double tail_error(D2<SBasis> const & a, unsigned tail) {
    return std::max(a[0].tailError(tail), a[1].tailError(tail));
}

Piecewise<D2<SBasis> > sectionize(D2<Piecewise<SBasis> > const &a) {
    Piecewise<SBasis> x = partition(a[0], a[1].cuts), y = partition(a[1], a[0].cuts);
    assert(x.size() == y.size());
    Piecewise<D2<SBasis> > ret;
    for(unsigned i = 0; i < x.size(); i++)
        ret.push_seg(D2<SBasis>(x[i], y[i]));
    ret.cuts.insert(ret.cuts.end(), x.cuts.begin(), x.cuts.end());
    return ret;
}

D2<Piecewise<SBasis> > make_cuts_independant(Piecewise<D2<SBasis> > const &a) {
    D2<Piecewise<SBasis> > ret;
    for(unsigned d = 0; d < 2; d++) {
        for(unsigned i = 0; i < a.size(); i++)
            ret[d].push_seg(a[i][d]);
        ret[d].cuts.insert(ret[d].cuts.end(), a.cuts.begin(), a.cuts.end());
    }
    return ret;
}

Piecewise<D2<SBasis> > rot90(Piecewise<D2<SBasis> > const &M){
  Piecewise<D2<SBasis> > result;
  if (M.empty()) return M;
  result.push_cut(M.cuts[0]);
  for (unsigned i=0; i<M.size(); i++){
    result.push(rot90(M[i]),M.cuts[i+1]);
  }
  return result;
}

Piecewise<SBasis> dot(Piecewise<D2<SBasis> > const &a, 
		      Piecewise<D2<SBasis> > const &b){
  Piecewise<SBasis > result;
  if (a.empty() || b.empty()) return result;
  Piecewise<D2<SBasis> > aa = partition(a,b.cuts);
  Piecewise<D2<SBasis> > bb = partition(b,a.cuts);

  result.push_cut(aa.cuts.front());
  for (unsigned i=0; i<aa.size(); i++){
    result.push(dot(aa.segs[i],bb.segs[i]),aa.cuts[i+1]);
  }
  return result;
}

Piecewise<SBasis> cross(Piecewise<D2<SBasis> > const &a, 
			Piecewise<D2<SBasis> > const &b){
  Piecewise<SBasis > result;
  if (a.empty() || b.empty()) return result;
  Piecewise<D2<SBasis> > aa = partition(a,b.cuts);
  Piecewise<D2<SBasis> > bb = partition(b,a.cuts);

  result.push_cut(aa.cuts.front());
  for (unsigned i=0; i<a.size(); i++){
    result.push(cross(aa.segs[i],bb.segs[i]),aa.cuts[i+1]);
  }
  return result;
}

/* Replaced by remove_short_cuts in piecewise.h
//this recursively removes the shortest cut interval until none is shorter than tol.
//TODO: code this in a more efficient way!
Piecewise<D2<SBasis> > remove_short_cuts(Piecewise<D2<SBasis> > const &f, double tol){
    double min = tol;
    unsigned idx = f.size();
    for(unsigned i=0; i<f.size(); i++){
        if (min > f.cuts[i+1]-f.cuts[i]){
            min = f.cuts[i+1]-f.cuts[i];
            idx = int(i);
        }
    }
    if (idx==f.size()){
        return f;
    }
    if (f.size()==1) {
        //removing this seg would result in an empty pw<d2<sb>>...
        return f;
    }
    Piecewise<D2<SBasis> > new_f=f;
    for (int dim=0; dim<2; dim++){
        double v = Hat(f.segs.at(idx)[dim][0]);
        //TODO: what about closed curves?
        if (idx>0 && f.segs.at(idx-1).at1()==f.segs.at(idx).at0()) 
            new_f.segs.at(idx-1)[dim][0][1] = v;
        if (idx<f.size() && f.segs.at(idx+1).at0()==f.segs.at(idx).at1()) 
            new_f.segs.at(idx+1)[dim][0][0] = v;
    }
    double t = (f.cuts.at(idx)+f.cuts.at(idx+1))/2;
    new_f.cuts.at(idx+1) = t;    
    
    new_f.segs.erase(new_f.segs.begin()+idx);
    new_f.cuts.erase(new_f.cuts.begin()+idx);        
    return remove_short_cuts(new_f, tol);
}
*/

//if tol>0, only force continuity where the jump is smaller than tol.
Piecewise<D2<SBasis> > force_continuity(Piecewise<D2<SBasis> > const &f, 
                                        double tol,
                                        bool closed){
    if (f.size()==0) return f;
    Piecewise<D2<SBasis> > result=f;
    unsigned cur   = (closed)? 0:1;
    unsigned prev  = (closed)? f.size()-1:0;
    while(cur<f.size()){
        Point pt0 = f.segs[prev].at1();
        Point pt1 = f.segs[cur ].at0();
        if (tol<=0 || L2sq(pt0-pt1)<tol*tol){
            pt0 = (pt0+pt1)/2;
            for (unsigned dim=0; dim<2; dim++){
                result.segs[prev][dim][0][1]=pt0[dim];
                result.segs[cur ][dim][0][0]=pt0[dim];
            }
        }
        prev = cur++;
    }
    return result;
}
  
};
