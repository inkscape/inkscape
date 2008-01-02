#include "d2.h"
/* One would think that we would include d2-sbasis.h, however,
 * you cannot actually include it in anything - only d2 may import it.
 * This is due to the trickinesses of template submatching. */

namespace Geom {

SBasis L2(D2<SBasis> const & a, unsigned k) { return sqrt(dot(a, a), k); }

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

Piecewise<D2<SBasis> > operator*(Piecewise<D2<SBasis> > const &a, Matrix const &m) {
  Piecewise<D2<SBasis> > result;
  if(a.empty()) return result;
  result.push_cut(a.cuts[0]);
  for (unsigned i = 0; i < a.size(); i++) {
    result.push(a[i] * m, a.cuts[i+1]);
  }
  return result;
}

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
                SBasis &prev_sb=result.segs[prev][dim];
                SBasis &cur_sb =result.segs[cur][dim];
                Coord const c=pt0[dim];
                if (prev_sb.empty()) {
                  prev_sb.push_back(Linear(0.0, c));
                } else {
                  prev_sb[0][1] = c;
                }
                if (cur_sb.empty()) {
                  cur_sb.push_back(Linear(c, 0.0));
                } else {
                  cur_sb[0][0] = c;
                }
            }
        }
        prev = cur++;
    }
    return result;
}
}
