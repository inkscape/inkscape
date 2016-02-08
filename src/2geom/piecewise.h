/** @file
 * @brief Piecewise function class
 *//*
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

#ifndef LIB2GEOM_SEEN_PIECEWISE_H
#define LIB2GEOM_SEEN_PIECEWISE_H

#include <vector>
#include <map>
#include <utility>
#include <boost/concept_check.hpp>
#include <2geom/concepts.h>
#include <2geom/math-utils.h>
#include <2geom/sbasis.h>


namespace Geom {
/**
 * @brief Function defined as discrete pieces.
 *
 * The Piecewise class manages a sequence of elements of a type as segments and
 * the ’cuts’ between them. These cuts are time values which separate the pieces.
 * This function representation allows for more interesting functions, as it provides
 * a viable output for operations such as inversion, which may require multiple
 * SBasis to properly invert the original.
 *
 * As for technical details, while the actual SBasis segments begin on the ﬁrst
 * cut and end on the last, the function is deﬁned throughout all inputs by ex-
 * tending the ﬁrst and last segments. The exact switching between segments is
 * arbitrarily such that beginnings (t=0) have preference over endings (t=1). This
 * only matters if it is discontinuous at the location.
 * \f[
 *      f(t) \rightarrow \left\{ 
 *      \begin{array}{cc}
 *      s_1,& t <= c_2 \\
 *      s_2,& c_2 <= t <= c_3\\
 *      \ldots \\
 *      s_n,& c_n <= t
 *      \end{array}\right.
 * \f]
 *
 * @ingroup Fragments
 */
template <typename T>
class Piecewise {
  BOOST_CLASS_REQUIRE(T, Geom, FragmentConcept);

  public:
    std::vector<double> cuts;
    std::vector<T> segs;
    //segs[i] stretches from cuts[i] to cuts[i+1].

    Piecewise() {}

    explicit Piecewise(const T &s) {
        push_cut(0.);
        push_seg(s);
        push_cut(1.);
    }

    unsigned input_dim(){return 1;}

    typedef typename T::output_type output_type;

    explicit Piecewise(const output_type & v) {
        push_cut(0.);
        push_seg(T(v));
        push_cut(1.);
    }

    inline void reserve(unsigned i) { segs.reserve(i); cuts.reserve(i + 1); }

    inline T const& operator[](unsigned i) const { return segs[i]; }
    inline T&       operator[](unsigned i)       { return segs[i]; }
    inline output_type operator()(double t) const { return valueAt(t); }
    inline output_type valueAt(double t) const {
        unsigned n = segN(t);
        return segs[n](segT(t, n));
    }
    inline output_type firstValue() const {
        return valueAt(cuts.front());
    }
    inline output_type lastValue() const {
        return valueAt(cuts.back());
    }

    /**
    *  The size of the returned vector equals n_derivs+1.
    */
    std::vector<output_type> valueAndDerivatives(double t, unsigned n_derivs) const {
        unsigned n = segN(t);
        std::vector<output_type> ret, val = segs[n].valueAndDerivatives(segT(t, n), n_derivs);
        double mult = 1;
        for(unsigned i = 0; i < val.size(); i++) {
            ret.push_back(val[i] * mult);
            mult /= cuts[n+1] - cuts[n];
        }
        return ret;
    }

    //TODO: maybe it is not a good idea to have this?
    Piecewise<T> operator()(SBasis f);
    Piecewise<T> operator()(Piecewise<SBasis>f);

    inline unsigned size() const { return segs.size(); }
    inline bool empty() const { return segs.empty(); }
    inline void clear() {
        segs.clear();
        cuts.clear();
    }

    /**Convenience/implementation hiding function to add segment/cut pairs.
     * Asserts that basic size and order invariants are correct
     */
    inline void push(const T &s, double to) {
        assert(cuts.size() - segs.size() == 1);
        push_seg(s);
        push_cut(to);
    }
#ifdef CPP11
    inline void push(T &&s, double to) {
        assert(cuts.size() - segs.size() == 1);
        push_seg(s);
        push_cut(to);
    }
#endif
    //Convenience/implementation hiding function to add cuts.
    inline void push_cut(double c) {
        ASSERT_INVARIANTS(cuts.empty() || c > cuts.back());
        cuts.push_back(c);
    }
    //Convenience/implementation hiding function to add segments.
    inline void push_seg(const T &s) { segs.push_back(s); }
#ifdef CPP11
    inline void push_seg(T &&s) { segs.emplace_back(s); }
#endif

    /**Returns the segment index which corresponds to a 'global' piecewise time.
     * Also takes optional low/high parameters to expedite the search for the segment.
     */
    inline unsigned segN(double t, int low = 0, int high = -1) const {
        high = (high == -1) ? size() : high;
        if(t < cuts[0]) return 0;
        if(t >= cuts[size()]) return size() - 1;
        while(low < high) {
            int mid = (high + low) / 2; //Lets not plan on having huge (> INT_MAX / 2) cut sequences
            double mv = cuts[mid];
            if(mv < t) {
                if(t < cuts[mid + 1]) return mid; else low = mid + 1;
            } else if(t < mv) {
                if(cuts[mid - 1] < t) return mid - 1; else high = mid - 1;
            } else {
                return mid;
            }
        }
        return low;
    }

    /**Returns the time within a segment, given the 'global' piecewise time.
     * Also takes an optional index parameter which may be used for efficiency or to find the time on a
     * segment outside its range.  If it is left to its default, -1, it will call segN to find the index.
     */
    inline double segT(double t, int i = -1) const {
        if(i == -1) i = segN(t);
        assert(i >= 0);
        return (t - cuts[i]) / (cuts[i+1] - cuts[i]);
    }

    inline double mapToDomain(double t, unsigned i) const {
        return (1-t)*cuts[i] + t*cuts[i+1]; //same as: t * (cuts[i+1] - cuts[i]) + cuts[i]
    }

    //Offsets the piecewise domain
    inline void offsetDomain(double o) {
        assert(IS_FINITE(o));
        if(o != 0)
            for(unsigned i = 0; i <= size(); i++)
                cuts[i] += o;
    }

    //Scales the domain of the function by a value. 0 will result in an empty Piecewise.
    inline void scaleDomain(double s) {
        assert(s > 0);
        if(s == 0) {
            cuts.clear(); segs.clear();
            return;
        }
        for(unsigned i = 0; i <= size(); i++)
            cuts[i] *= s;
    }

    //Retrieves the domain in interval form
    inline Interval domain() const { return Interval(cuts.front(), cuts.back()); }

    //Transforms the domain into another interval
    inline void setDomain(Interval dom) {
        if(empty()) return;
        /* dom can not be empty
        if(dom.empty()) {
            cuts.clear(); segs.clear();
            return;
        }*/
        double cf = cuts.front();
        double o = dom.min() - cf, s = dom.extent() / (cuts.back() - cf);
        for(unsigned i = 0; i <= size(); i++)
            cuts[i] = (cuts[i] - cf) * s + o;
        //fix floating point precision errors.
        cuts[0] = dom.min();
        cuts[size()] = dom.max();
    }

    //Concatenates this Piecewise function with another, offseting time of the other to match the end.
    inline void concat(const Piecewise<T> &other) {
        if(other.empty()) return;

        if(empty()) {
            cuts = other.cuts; segs = other.segs;
            return;
        }

        segs.insert(segs.end(), other.segs.begin(), other.segs.end());
        double t = cuts.back() - other.cuts.front();
        cuts.reserve(cuts.size() + other.size());
        for(unsigned i = 0; i < other.size(); i++)
            push_cut(other.cuts[i + 1] + t);
    }

    //Like concat, but ensures continuity.
    inline void continuousConcat(const Piecewise<T> &other) {
        boost::function_requires<AddableConcept<typename T::output_type> >();
        if(other.empty()) return;

        if(empty()) { segs = other.segs; cuts = other.cuts; return; }

        typename T::output_type y = segs.back().at1() - other.segs.front().at0();
        double t = cuts.back() - other.cuts.front();
        reserve(size() + other.size());
        for(unsigned i = 0; i < other.size(); i++)
            push(other[i] + y, other.cuts[i + 1] + t);
    }

    //returns true if the Piecewise<T> meets some basic invariants.
    inline bool invariants() const {
        // segs between cuts
        if(!(segs.size() + 1 == cuts.size() || (segs.empty() && cuts.empty())))
            return false;
        // cuts in order
        for(unsigned i = 0; i < segs.size(); i++)
            if(cuts[i] >= cuts[i+1])
                return false;
        return true;
    }

};

/**
 *  ...
 *  \return ...
 *  \relates Piecewise
 */
template<typename T>
inline typename FragmentConcept<T>::BoundsType bounds_fast(const Piecewise<T> &f) {
    boost::function_requires<FragmentConcept<T> >();

    if(f.empty()) return typename FragmentConcept<T>::BoundsType();
    typename FragmentConcept<T>::BoundsType ret(bounds_fast(f[0]));
    for(unsigned i = 1; i < f.size(); i++)
        ret.unionWith(bounds_fast(f[i]));
    return ret;
}

/**
 *  ...
 *  \return ...
 *  \relates Piecewise
 */
template<typename T>
inline typename FragmentConcept<T>::BoundsType bounds_exact(const Piecewise<T> &f) {
    boost::function_requires<FragmentConcept<T> >();

    if(f.empty()) return typename FragmentConcept<T>::BoundsType();
    typename FragmentConcept<T>::BoundsType ret(bounds_exact(f[0]));
    for(unsigned i = 1; i < f.size(); i++)
        ret.unionWith(bounds_exact(f[i]));
    return ret;
}

/**
 *  ...
 *  \return ...
 *  \relates Piecewise
 */
template<typename T>
inline typename FragmentConcept<T>::BoundsType bounds_local(const Piecewise<T> &f, const OptInterval &_m) {
    boost::function_requires<FragmentConcept<T> >();

    if(f.empty() || !_m) return typename FragmentConcept<T>::BoundsType();
    Interval const &m = *_m;
    if(m.isSingular()) return typename FragmentConcept<T>::BoundsType(f(m.min()));

    unsigned fi = f.segN(m.min()), ti = f.segN(m.max());
    double ft = f.segT(m.min(), fi), tt = f.segT(m.max(), ti);

    if(fi == ti) return bounds_local(f[fi], Interval(ft, tt));

    typename FragmentConcept<T>::BoundsType ret(bounds_local(f[fi], Interval(ft, 1.)));
    for(unsigned i = fi + 1; i < ti; i++)
        ret.unionWith(bounds_exact(f[i]));
    if(tt != 0.) ret.unionWith(bounds_local(f[ti], Interval(0., tt)));

    return ret;
}

/**
 *  Returns a portion of a piece of a Piecewise<T>, given the piece's index and a to/from time.
 *  \relates Piecewise
 */
template<typename T>
T elem_portion(const Piecewise<T> &a, unsigned i, double from, double to) {
    assert(i < a.size());
    double rwidth = 1 / (a.cuts[i+1] - a.cuts[i]);
    return portion( a[i], (from - a.cuts[i]) * rwidth, (to - a.cuts[i]) * rwidth );
}

/**Piecewise<T> partition(const Piecewise<T> &pw, std::vector<double> const &c);
 * Further subdivides the Piecewise<T> such that there is a cut at every value in c.
 * Precondition: c sorted lower to higher.
 *
 * //Given Piecewise<T> a and b:
 * Piecewise<T> ac = a.partition(b.cuts);
 * Piecewise<T> bc = b.partition(a.cuts);
 * //ac.cuts should be equivalent to bc.cuts
 *
 * \relates Piecewise
 */
template<typename T>
Piecewise<T> partition(const Piecewise<T> &pw, std::vector<double> const &c) {
    assert(pw.invariants());
    if(c.empty()) return Piecewise<T>(pw);

    Piecewise<T> ret = Piecewise<T>();
    ret.reserve(c.size() + pw.cuts.size() - 1);

    if(pw.empty()) {
        ret.cuts = c;
        for(unsigned i = 0; i < c.size() - 1; i++)
            ret.push_seg(T());
        return ret;
    }

    unsigned si = 0, ci = 0;     //Segment index, Cut index

    //if the cuts have something earlier than the Piecewise<T>, add portions of the first segment
    while(c[ci] < pw.cuts.front() && ci < c.size()) {
        bool isLast = (ci == c.size()-1 || c[ci + 1] >= pw.cuts.front());
        ret.push_cut(c[ci]);
        ret.push_seg( elem_portion(pw, 0, c[ci], isLast ? pw.cuts.front() : c[ci + 1]) );
        ci++;
    }

    ret.push_cut(pw.cuts[0]);
    double prev = pw.cuts[0];    //previous cut
    //Loop which handles cuts within the Piecewise<T> domain
    //Should have the cuts = segs + 1 invariant
    while(si < pw.size() && ci <= c.size()) {
        if(ci == c.size() && prev <= pw.cuts[si]) { //cuts exhausted, straight copy the rest
            ret.segs.insert(ret.segs.end(), pw.segs.begin() + si, pw.segs.end());
            ret.cuts.insert(ret.cuts.end(), pw.cuts.begin() + si + 1, pw.cuts.end());
            return ret;
        }else if(ci == c.size() || c[ci] >= pw.cuts[si + 1]) {  //no more cuts within this segment, finalize
            if(prev > pw.cuts[si]) {      //segment already has cuts, so portion is required
                ret.push_seg(portion(pw[si], pw.segT(prev, si), 1.0));
            } else {                     //plain copy is fine
                ret.push_seg(pw[si]);
            }
            ret.push_cut(pw.cuts[si + 1]);
            prev = pw.cuts[si + 1];
            si++;
        } else if(c[ci] == pw.cuts[si]){                  //coincident
            //Already finalized the seg with the code immediately above
            ci++;
        } else {                                         //plain old subdivision
            ret.push(elem_portion(pw, si, prev, c[ci]), c[ci]);
            prev = c[ci];
            ci++;
        }
    }

    //input cuts extend further than this Piecewise<T>, extend the last segment.
    while(ci < c.size()) {
        if(c[ci] > prev) {
            ret.push(elem_portion(pw, pw.size() - 1, prev, c[ci]), c[ci]);
            prev = c[ci];
        }
        ci++;
    }
    return ret;
}

/**
 *  Returns a Piecewise<T> with a defined domain of [min(from, to), max(from, to)].
 *  \relates Piecewise
 */
template<typename T>
Piecewise<T> portion(const Piecewise<T> &pw, double from, double to) {
    if(pw.empty() || from == to) return Piecewise<T>();

    Piecewise<T> ret;

    double temp = from;
    from = std::min(from, to);
    to = std::max(temp, to);

    unsigned i = pw.segN(from);
    ret.push_cut(from);
    if(i == pw.size() - 1 || to <= pw.cuts[i + 1]) {    //to/from inhabit the same segment
        ret.push(elem_portion(pw, i, from, to), to);
        return ret;
    }
    ret.push_seg(portion( pw[i], pw.segT(from, i), 1.0 ));
    i++;
    unsigned fi = pw.segN(to, i);
    ret.reserve(fi - i + 1);
    if (to == pw.cuts[fi]) fi-=1;

    ret.segs.insert(ret.segs.end(), pw.segs.begin() + i, pw.segs.begin() + fi);  //copy segs
    ret.cuts.insert(ret.cuts.end(), pw.cuts.begin() + i, pw.cuts.begin() + fi + 1);  //and their cuts

    ret.push_seg( portion(pw[fi], 0.0, pw.segT(to, fi)));
    if(to != ret.cuts.back()) ret.push_cut(to);
    ret.invariants();
    return ret;
}

//TODO: seems like these should be mutating
/**
 *  ...
 *  \return ...
 *  \relates Piecewise
 */
template<typename T>
Piecewise<T> remove_short_cuts(Piecewise<T> const &f, double tol) {
    if(f.empty()) return f;
    Piecewise<T> ret;
    ret.reserve(f.size());
    ret.push_cut(f.cuts[0]);
    for(unsigned i=0; i<f.size(); i++){
        if (f.cuts[i+1]-f.cuts[i] >= tol || i==f.size()-1) {
            ret.push(f[i], f.cuts[i+1]);
        }
    }
    return ret;
}

//TODO: seems like these should be mutating
/**
 *  ...
 *  \return ...
 *  \relates Piecewise
 */
template<typename T>
Piecewise<T> remove_short_cuts_extending(Piecewise<T> const &f, double tol) {
    if(f.empty()) return f;
    Piecewise<T> ret;
    ret.reserve(f.size());
    ret.push_cut(f.cuts[0]);
    double last = f.cuts[0]; // last cut included
    for(unsigned i=0; i<f.size(); i++){
        if (f.cuts[i+1]-f.cuts[i] >= tol) {
            ret.push(elem_portion(f, i, last, f.cuts[i+1]), f.cuts[i+1]);
            last = f.cuts[i+1];
        }
    }
    return ret;
}

/**
 *  ...
 *  \return ...
 *  \relates Piecewise
 */
template<typename T>
std::vector<double> roots(const Piecewise<T> &pw) {
    std::vector<double> ret;
    for(unsigned i = 0; i < pw.size(); i++) {
        std::vector<double> sr = roots(pw[i]);
        for (unsigned j = 0; j < sr.size(); j++) ret.push_back(sr[j] * (pw.cuts[i + 1] - pw.cuts[i]) + pw.cuts[i]);

    }
    return ret;
}

//IMPL: OffsetableConcept
/**
 *  ...
 *  \return \f$ a + b = \f$
 *  \relates Piecewise
 */
template<typename T>
Piecewise<T> operator+(Piecewise<T> const &a, typename T::output_type b) {
    boost::function_requires<OffsetableConcept<T> >();
//TODO:empty
    Piecewise<T> ret;
    ret.segs.reserve(a.size());
    ret.cuts = a.cuts;
    for(unsigned i = 0; i < a.size();i++)
        ret.push_seg(a[i] + b);
    return ret;
}
template<typename T>
Piecewise<T> operator-(Piecewise<T> const &a, typename T::output_type b) {
    boost::function_requires<OffsetableConcept<T> >();
//TODO: empty
    Piecewise<T> ret;
    ret.segs.reserve(a.size());
    ret.cuts = a.cuts;
    for(unsigned i = 0; i < a.size();i++)
        ret.push_seg(a[i] - b);
    return ret;
}
template<typename T>
Piecewise<T>& operator+=(Piecewise<T>& a, typename T::output_type b) {
    boost::function_requires<OffsetableConcept<T> >();

    if(a.empty()) { a.push_cut(0.); a.push(T(b), 1.); return a; }

    for(unsigned i = 0; i < a.size();i++)
        a[i] += b;
    return a;
}
template<typename T>
Piecewise<T>& operator-=(Piecewise<T>& a, typename T::output_type b) {
    boost::function_requires<OffsetableConcept<T> >();

    if(a.empty()) { a.push_cut(0.); a.push(T(-b), 1.); return a; }

    for(unsigned i = 0;i < a.size();i++)
        a[i] -= b;
    return a;
}

//IMPL: ScalableConcept
/**
 *  ...
 *  \return \f$ -a = \f$
 *  \relates Piecewise
 */
template<typename T>
Piecewise<T> operator-(Piecewise<T> const &a) {
    boost::function_requires<ScalableConcept<T> >();

    Piecewise<T> ret;
    ret.segs.reserve(a.size());
    ret.cuts = a.cuts;
    for(unsigned i = 0; i < a.size();i++)
        ret.push_seg(- a[i]);
    return ret;
}
/**
 *  ...
 *  \return \f$ a * b = \f$
 *  \relates Piecewise
 */
template<typename T>
Piecewise<T> operator*(Piecewise<T> const &a, double b) {
    boost::function_requires<ScalableConcept<T> >();

    if(a.empty()) return Piecewise<T>();

    Piecewise<T> ret;
    ret.segs.reserve(a.size());
    ret.cuts = a.cuts;
    for(unsigned i = 0; i < a.size();i++)
        ret.push_seg(a[i] * b);
    return ret;
}
/**
 *  ...
 *  \return \f$ a * b = \f$
 *  \relates Piecewise
 */
template<typename T>
Piecewise<T> operator*(Piecewise<T> const &a, T b) {
    boost::function_requires<ScalableConcept<T> >();

    if(a.empty()) return Piecewise<T>();

    Piecewise<T> ret;
    ret.segs.reserve(a.size());
    ret.cuts = a.cuts;
    for(unsigned i = 0; i < a.size();i++)
        ret.push_seg(a[i] * b);
    return ret;
}
/**
 *  ...
 *  \return \f$ a / b = \f$
 *  \relates Piecewise
 */
template<typename T>
Piecewise<T> operator/(Piecewise<T> const &a, double b) {
    boost::function_requires<ScalableConcept<T> >();

    //FIXME: b == 0?
    if(a.empty()) return Piecewise<T>();

    Piecewise<T> ret;
    ret.segs.reserve(a.size());
    ret.cuts = a.cuts;
    for(unsigned i = 0; i < a.size();i++)
        ret.push_seg(a[i] / b);
    return ret;
}
template<typename T>
Piecewise<T>& operator*=(Piecewise<T>& a, double b) {
    boost::function_requires<ScalableConcept<T> >();

    for(unsigned i = 0; i < a.size();i++)
        a[i] *= b;
    return a;
}
template<typename T>
Piecewise<T>& operator/=(Piecewise<T>& a, double b) {
    boost::function_requires<ScalableConcept<T> >();

    //FIXME: b == 0?

    for(unsigned i = 0; i < a.size();i++)
        a[i] /= b;
    return a;
}

//IMPL: AddableConcept
/**
 *  ...
 *  \return \f$ a + b = \f$
 *  \relates Piecewise
 */
template<typename T>
Piecewise<T> operator+(Piecewise<T> const &a, Piecewise<T> const &b) {
    boost::function_requires<AddableConcept<T> >();

    Piecewise<T> pa = partition(a, b.cuts), pb = partition(b, a.cuts);
    Piecewise<T> ret;
    assert(pa.size() == pb.size());
    ret.segs.reserve(pa.size());
    ret.cuts = pa.cuts;
    for (unsigned i = 0; i < pa.size(); i++)
        ret.push_seg(pa[i] + pb[i]);
    return ret;
}
/**
 *  ...
 *  \return \f$ a - b = \f$
 *  \relates Piecewise
 */
template<typename T>
Piecewise<T> operator-(Piecewise<T> const &a, Piecewise<T> const &b) {
    boost::function_requires<AddableConcept<T> >();

    Piecewise<T> pa = partition(a, b.cuts), pb = partition(b, a.cuts);
    Piecewise<T> ret = Piecewise<T>();
    assert(pa.size() == pb.size());
    ret.segs.reserve(pa.size());
    ret.cuts = pa.cuts;
    for (unsigned i = 0; i < pa.size(); i++)
        ret.push_seg(pa[i] - pb[i]);
    return ret;
}
template<typename T>
inline Piecewise<T>& operator+=(Piecewise<T> &a, Piecewise<T> const &b) {
    a = a+b;
    return a;
}
template<typename T>
inline Piecewise<T>& operator-=(Piecewise<T> &a, Piecewise<T> const &b) {
    a = a-b;
    return a;
}

/**
 *  ...
 *  \return \f$ a \cdot b = \f$
 *  \relates Piecewise
 */
template<typename T1,typename T2>
Piecewise<T2> operator*(Piecewise<T1> const &a, Piecewise<T2> const &b) {
    //function_requires<MultiplicableConcept<T1> >();
    //function_requires<MultiplicableConcept<T2> >();

    Piecewise<T1> pa = partition(a, b.cuts);
    Piecewise<T2> pb = partition(b, a.cuts);
    Piecewise<T2> ret = Piecewise<T2>();
    assert(pa.size() == pb.size());
    ret.segs.reserve(pa.size());
    ret.cuts = pa.cuts;
    for (unsigned i = 0; i < pa.size(); i++)
        ret.push_seg(pa[i] * pb[i]);
    return ret;
}

/**
 *  ...
 *  \return \f$ a \cdot b \f$
 *  \relates Piecewise
 */
template<typename T>
inline Piecewise<T>& operator*=(Piecewise<T> &a, Piecewise<T> const &b) {
    a = a * b;
    return a;
}

Piecewise<SBasis> divide(Piecewise<SBasis> const &a, Piecewise<SBasis> const &b, unsigned k);
//TODO: replace divide(a,b,k) by divide(a,b,tol,k)?
//TODO: atm, relative error is ~(tol/a)%. Find a way to make it independant of a.
//Nota: the result is 'truncated' where b is smaller than 'zero': ~ a/max(b,zero).
Piecewise<SBasis>
divide(Piecewise<SBasis> const &a, Piecewise<SBasis> const &b, double tol, unsigned k, double zero=1.e-3);
Piecewise<SBasis>
divide(SBasis const &a, Piecewise<SBasis> const &b, double tol, unsigned k, double zero=1.e-3);
Piecewise<SBasis>
divide(Piecewise<SBasis> const &a, SBasis const &b, double tol, unsigned k, double zero=1.e-3);
Piecewise<SBasis>
divide(SBasis const &a, SBasis const &b, double tol, unsigned k, double zero=1.e-3);

//Composition: functions called compose_* are pieces of compose that are factored out in pw.cpp.
std::map<double,unsigned> compose_pullback(std::vector<double> const &cuts, SBasis const &g);
int compose_findSegIdx(std::map<double,unsigned>::iterator  const &cut,
                       std::map<double,unsigned>::iterator  const &next,
                       std::vector<double>  const &levels,
                       SBasis const &g);

/**
 *  ...
 *  \return ...
 *  \relates Piecewise
 */
template<typename T>
Piecewise<T> compose(Piecewise<T> const &f, SBasis const &g){
    /// \todo add concept check
    Piecewise<T> result;
    if (f.empty()) return result;
    if (g.isZero()) return Piecewise<T>(f(0));
    if (f.size()==1){
        double t0 = f.cuts[0], width = f.cuts[1] - t0;
        return (Piecewise<T>) compose(f.segs[0],compose(Linear(-t0 / width, (1-t0) / width), g));
    }

    //first check bounds...
    Interval bs = *bounds_fast(g);
    if (f.cuts.front() > bs.max()  || bs.min() > f.cuts.back()){
        int idx = (bs.max() < f.cuts[1]) ? 0 : f.cuts.size()-2;
        double t0 = f.cuts[idx], width = f.cuts[idx+1] - t0;
        return (Piecewise<T>) compose(f.segs[idx],compose(Linear(-t0 / width, (1-t0) / width), g));
    }

    std::vector<double> levels;//we can forget first and last cuts...
    levels.insert(levels.begin(),f.cuts.begin()+1,f.cuts.end()-1);
    //TODO: use a std::vector<pairs<double,unsigned> > instead of a map<double,unsigned>.
    std::map<double,unsigned> cuts_pb = compose_pullback(levels,g);

    //-- Compose each piece of g with the relevant seg of f.
    result.cuts.push_back(0.);
    std::map<double,unsigned>::iterator cut=cuts_pb.begin();
    std::map<double,unsigned>::iterator next=cut; next++;
    while(next!=cuts_pb.end()){
        //assert(std::abs(int((*cut).second-(*next).second))<1);
        //TODO: find a way to recover from this error? the root finder missed some root;
        //  the levels/variations of f might be too close/fast...
        int idx = compose_findSegIdx(cut,next,levels,g);
        double t0=(*cut).first;
        double t1=(*next).first;

        if (!are_near(t0,t1,EPSILON*EPSILON)) { // prevent adding cuts that are extremely close together and that may cause trouble with rounding e.g. when reversing the path
            SBasis sub_g=compose(g, Linear(t0,t1));
            sub_g=compose(Linear(-f.cuts[idx]/(f.cuts[idx+1]-f.cuts[idx]),
                                 (1-f.cuts[idx])/(f.cuts[idx+1]-f.cuts[idx])),sub_g);
            result.push(compose(f[idx],sub_g),t1);
        }

        cut++;
        next++;
    }
    return(result);
}

/**
 *  ...
 *  \return ...
 *  \relates Piecewise
 */
template<typename T>
Piecewise<T> compose(Piecewise<T> const &f, Piecewise<SBasis> const &g){
/// \todo add concept check
  Piecewise<T> result;
  for(unsigned i = 0; i < g.segs.size(); i++){
      Piecewise<T> fgi=compose(f, g.segs[i]);
      fgi.setDomain(Interval(g.cuts[i], g.cuts[i+1]));
      result.concat(fgi);
  }
  return result;
}

/*
Piecewise<D2<SBasis> > compose(D2<SBasis2d> const &sb2d, Piecewise<D2<SBasis> > const &pwd2sb){
/// \todo add concept check
  Piecewise<D2<SBasis> > result;
  result.push_cut(0.);
  for(unsigned i = 0; i < pwd2sb.size(); i++){
     result.push(compose_each(sb2d,pwd2sb[i]),i+1);
  }
  return result;
}*/

/** Compose an SBasis with the inverse of another.
 * WARNING: It's up to the user to check that the second SBasis is indeed
 * invertible (i.e. strictly increasing or decreasing).
 *  \return \f$ f \cdot g^{-1} \f$
 *  \relates Piecewise
 */
Piecewise<SBasis> pw_compose_inverse(SBasis const &f, SBasis const &g, unsigned order, double zero);



template <typename T>
Piecewise<T> Piecewise<T>::operator()(SBasis f){return compose((*this),f);}
template <typename T>
Piecewise<T> Piecewise<T>::operator()(Piecewise<SBasis>f){return compose((*this),f);}

/**
 *  ...
 *  \return ...
 *  \relates Piecewise
 */
template<typename T>
Piecewise<T> integral(Piecewise<T> const &a) {
    Piecewise<T> result;
    result.segs.resize(a.segs.size());
    result.cuts = a.cuts;
    typename T::output_type c = a.segs[0].at0();
    for(unsigned i = 0; i < a.segs.size(); i++){
        result.segs[i] = integral(a.segs[i])*(a.cuts[i+1]-a.cuts[i]);
        result.segs[i]+= c-result.segs[i].at0();
        c = result.segs[i].at1();
    }
    return result;
}

/**
 *  ...
 *  \return ...
 *  \relates Piecewise
 */
template<typename T>
Piecewise<T> derivative(Piecewise<T> const &a) {
    Piecewise<T> result;
    result.segs.resize(a.segs.size());
    result.cuts = a.cuts;
    for(unsigned i = 0; i < a.segs.size(); i++){
        result.segs[i] = derivative(a.segs[i])/(a.cuts[i+1]-a.cuts[i]);
    }
    return result;
}

std::vector<double> roots(Piecewise<SBasis> const &f);

std::vector<std::vector<double> >multi_roots(Piecewise<SBasis> const &f, std::vector<double> const &values);

//TODO: implement level_sets directly for pwsb instead of sb (and derive it fo sb).
//It should be faster than the reverse as the algorithm may jump over full cut intervals.
std::vector<Interval> level_set(Piecewise<SBasis> const &f, Interval const &level, double tol=1e-5);
std::vector<Interval> level_set(Piecewise<SBasis> const &f, double v, double vtol, double tol=1e-5);
//std::vector<Interval> level_sets(Piecewise<SBasis> const &f, std::vector<Interval> const &levels, double tol=1e-5);
//std::vector<Interval> level_sets(Piecewise<SBasis> const &f, std::vector<double> &v, double vtol, double tol=1e-5);


/**
 *  ...
 *  \return ...
 *  \relates Piecewise
 */
template<typename T>
Piecewise<T> reverse(Piecewise<T> const &f) {
    Piecewise<T> ret = Piecewise<T>();
    ret.reserve(f.size());
    double start = f.cuts[0];
    double end = f.cuts.back();
    for (unsigned i = 0; i < f.cuts.size(); i++) {
        double x = f.cuts[f.cuts.size() - 1 - i];
        ret.push_cut(end - (x - start));
    }
    for (unsigned i = 0; i < f.segs.size(); i++)
        ret.push_seg(reverse(f[f.segs.size() - i - 1]));
    return ret;
}

/**
 *  Interpolates between a and b.
 *  \return a if t = 0, b if t = 1, or an interpolation between a and b for t in [0,1]
 *  \relates Piecewise
 */
template<typename T>
Piecewise<T> lerp(double t, Piecewise<T> const &a, Piecewise<T> b) {
    // Make sure both paths have the same number of segments and cuts at the same locations
    b.setDomain(a.domain());
    Piecewise<T> pA = partition(a, b.cuts);
    Piecewise<T> pB = partition(b, a.cuts);

    return (pA*(1-t)  +  pB*t);
}

}
#endif //LIB2GEOM_SEEN_PIECEWISE_H
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
