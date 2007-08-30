#ifdef _2GEOM_D2  /*This is intentional: we don't actually want anyone to
                    include this, other than D2.h.  If somone else tries, D2
                    won't be defined.  If it is, this will already be included. */
#ifndef __2GEOM_SBASIS_CURVE_H
#define __2GEOM_SBASIS_CURVE_H

#include "sbasis.h"
#include "sbasis-2d.h"
#include "piecewise.h"
#include "matrix.h"

//TODO: implement intersect

namespace Geom {

inline D2<SBasis> compose(D2<SBasis> const & a, SBasis const & b) {
    return D2<SBasis>(compose(a[X], b), compose(a[Y], b));
}

SBasis L2(D2<SBasis> const & a, unsigned k);
double L2(D2<double> const & a);

D2<SBasis> multiply(Linear const & a, D2<SBasis> const & b);
inline D2<SBasis> operator*(Linear const & a, D2<SBasis> const & b) { return multiply(a, b); }
D2<SBasis> multiply(SBasis const & a, D2<SBasis> const & b);
inline D2<SBasis> operator*(SBasis const & a, D2<SBasis> const & b) { return multiply(a, b); }
D2<SBasis> truncate(D2<SBasis> const & a, unsigned terms);

unsigned sbasis_size(D2<SBasis> const & a);
double tail_error(D2<SBasis> const & a, unsigned tail);

//Piecewise<D2<SBasis> > specific decls:

Piecewise<D2<SBasis> > sectionize(D2<Piecewise<SBasis> > const &a);
D2<Piecewise<SBasis> > make_cuts_independant(Piecewise<D2<SBasis> > const &a);
Piecewise<D2<SBasis> > rot90(Piecewise<D2<SBasis> > const &a);
Piecewise<SBasis> dot(Piecewise<D2<SBasis> > const &a, Piecewise<D2<SBasis> > const &b);
Piecewise<SBasis> cross(Piecewise<D2<SBasis> > const &a, Piecewise<D2<SBasis> > const &b);

Piecewise<D2<SBasis> > operator*(Piecewise<D2<SBasis> > const &a, Matrix const &m);

Piecewise<D2<SBasis> > force_continuity(Piecewise<D2<SBasis> > const &f, 
                                        double tol=0,
                                        bool closed=false);

class CoordIterator
: public std::iterator<std::input_iterator_tag, SBasis const>
{
public:
  CoordIterator(std::vector<D2<SBasis> >::const_iterator const &iter, unsigned d) : impl_(iter), ix_(d) {}

  inline bool operator==(CoordIterator const &other) { return other.impl_ == impl_; }
  inline bool operator!=(CoordIterator const &other) { return other.impl_ != impl_; }

  inline SBasis operator*() const {
        return (*impl_)[ix_];
  }

  inline CoordIterator &operator++() {
    ++impl_;
    return *this;
  }
  inline CoordIterator operator++(int) {
    CoordIterator old=*this;
    ++(*this);
    return old;
  }

private:
  std::vector<D2<SBasis> >::const_iterator impl_;
  unsigned ix_;
};

inline CoordIterator iterateCoord(Piecewise<D2<SBasis> > const &a, unsigned d) {
    return CoordIterator(a.segs.begin(), d);
}

//bounds specializations with order
inline Rect bounds_fast(D2<SBasis> const & s, unsigned order=0) {
    return Rect(bounds_fast(s[X], order),
                bounds_fast(s[Y], order));
}
inline Rect bounds_local(D2<SBasis> const & s, Interval i, unsigned order=0) {
    return Rect(bounds_local(s[X], i, order),
                bounds_local(s[Y], i, order));
}

}

#endif
#endif
