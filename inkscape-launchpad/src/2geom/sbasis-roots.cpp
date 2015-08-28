/**
 * @file
 * @brief Root finding for sbasis functions.
 *//*
 * Authors: 
 *   Nathan Hurst <njh@njhurst.com>
 *   JF Barraud
 * Copyright 2006-2007 Authors
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
 
 /*
 * It is more efficient to find roots of f(t) = c_0, c_1, ... all at once, rather than iterating.
 *
 * Todo/think about:
 *  multi-roots using bernstein method, one approach would be:
    sort c
    take median and find roots of that
    whenever a segment lies entirely on one side of the median,
    find the median of the half and recurse.

    in essence we are implementing quicksort on a continuous function

 *  the gsl poly roots finder is faster than bernstein too, but we don't use it for 3 reasons:

 a) it requires convertion to poly, which is numerically unstable

 b) it requires gsl (which is currently not a dependency, and would bring in a whole slew of unrelated stuff)

 c) it finds all roots, even complex ones.  We don't want to accidently treat a nearly real root as a real root

From memory gsl poly roots was about 10 times faster than bernstein in the case where all the roots
are in [0,1] for polys of order 5.  I spent some time working out whether eigenvalue root finding
could be done directly in sbasis space, but the maths was too hard for me. -- njh

jfbarraud: eigenvalue root finding could be done directly in sbasis space ?

njh: I don't know, I think it should.  You would make a matrix whose characteristic polynomial was
correct, but do it by putting the sbasis terms in the right spots in the matrix.  normal eigenvalue
root finding makes a matrix that is a diagonal + a row along the top.  This matrix has the property
that its characteristic poly is just the poly whose coefficients are along the top row.

Now an sbasis function is a linear combination of the poly coeffs.  So it seems to me that you
should be able to put the sbasis coeffs directly into a matrix in the right spots so that the
characteristic poly is the sbasis.  You'll still have problems b) and c).

We might be able to lift an eigenvalue solver and include that directly into 2geom.  Eigenvalues
also allow you to find intersections of multiple curves but require solving n*m x n*m matrices.

 **/

#include <cmath>
#include <map>

#include <2geom/sbasis.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/solver.h>

using namespace std;

namespace Geom{

/** Find the smallest interval that bounds a
 \param a sbasis function
 \returns inteval

*/

#ifdef USE_SBASIS_OF
OptInterval bounds_exact(SBasisOf<double> const &a) {
    Interval result = Interval(a.at0(), a.at1());
    SBasisOf<double> df = derivative(a);
    vector<double>extrema = roots(df);
    for (unsigned i=0; i<extrema.size(); i++){
        result.extendTo(a(extrema[i]));
    }
    return result;
}
#else
OptInterval bounds_exact(SBasis const &a) {
    Interval result = Interval(a.at0(), a.at1());
    SBasis df = derivative(a);
    vector<double>extrema = roots(df);
    for (unsigned i=0; i<extrema.size(); i++){
        result.expandTo(a(extrema[i]));
    }
    return result;
}
#endif

/** Find a small interval that bounds a
 \param a sbasis function
 \returns inteval

*/
// I have no idea how this works, some clever bounding argument by jfb.
#ifdef USE_SBASIS_OF
OptInterval bounds_fast(const SBasisOf<double> &sb, int order) {
#else
OptInterval bounds_fast(const SBasis &sb, int order) {
#endif
    Interval res(0,0); // an empty sbasis is 0.

    for(int j = sb.size()-1; j>=order; j--) {
        double a=sb[j][0];
        double b=sb[j][1];

        double v, t = 0;
        v = res.min();
        if (v<0) t = ((b-a)/v+1)*0.5;
        if (v>=0 || t<0 || t>1) {
            res.setMin(std::min(a,b));
        } else {
            res.setMin(lerp(t, a+v*t, b));
        }

        v = res.max();
        if (v>0) t = ((b-a)/v+1)*0.5;
        if (v<=0 || t<0 || t>1) {
            res.setMax(std::max(a,b));
        }else{
            res.setMax(lerp(t, a+v*t, b));
        }
    }
    if (order>0) res*=std::pow(.25,order);
    return res;
}

/** Find a small interval that bounds a(t) for t in i to order order
 \param sb sbasis function
 \param i domain interval
 \param order number of terms
 \return interval

*/
#ifdef USE_SBASIS_OF
OptInterval bounds_local(const SBasisOf<double> &sb, const OptInterval &i, int order) {
#else
OptInterval bounds_local(const SBasis &sb, const OptInterval &i, int order) {
#endif
    double t0=i->min(), t1=i->max(), lo=0., hi=0.;
    for(int j = sb.size()-1; j>=order; j--) {
        double a=sb[j][0];
        double b=sb[j][1];

        double t = 0;
        if (lo<0) t = ((b-a)/lo+1)*0.5;
        if (lo>=0 || t<t0 || t>t1) {
            lo = std::min(a*(1-t0)+b*t0+lo*t0*(1-t0),a*(1-t1)+b*t1+lo*t1*(1-t1));
        }else{
            lo = lerp(t, a+lo*t, b);
        }

        if (hi>0) t = ((b-a)/hi+1)*0.5;
        if (hi<=0 || t<t0 || t>t1) {
            hi = std::max(a*(1-t0)+b*t0+hi*t0*(1-t0),a*(1-t1)+b*t1+hi*t1*(1-t1));
        }else{
            hi = lerp(t, a+hi*t, b);
        }
    }
    Interval res = Interval(lo,hi);
    if (order>0) res*=std::pow(.25,order);
    return res;
}

//-- multi_roots ------------------------------------
// goal: solve f(t)=c for several c at once.
/* algo: -compute f at both ends of the given segment [a,b].
         -compute bounds m<df(t)<M for df on the segment.
         let c and C be the levels below and above f(a):
         going from f(a) down to c with slope m takes at least time (f(a)-c)/m
         going from f(a)  up  to C with slope M takes at least time (C-f(a))/M
         From this we conclude there are no roots before a'=a+min((f(a)-c)/m,(C-f(a))/M).
         Do the same for b: compute some b' such that there are no roots in (b',b].
         -if [a',b'] is not empty, repeat the process with [a',(a'+b')/2] and [(a'+b')/2,b'].
  unfortunately, extra care is needed about rounding errors, and also to avoid the repetition of roots,
  making things tricky and unpleasant...
*/
//TODO: Make sure the code is "rounding-errors proof" and take care about repetition of roots!


static int upper_level(vector<double> const &levels,double x,double tol=0.){
    return(upper_bound(levels.begin(),levels.end(),x-tol)-levels.begin());
}

#ifdef USE_SBASIS_OF
static void multi_roots_internal(SBasis const &f,
				 SBasis const &df,
#else
static void multi_roots_internal(SBasis const &f,
				 SBasis const &df,
#endif
				 std::vector<double> const &levels,
				 std::vector<std::vector<double> > &roots,
				 double htol,
				 double vtol,
				 double a,
				 double fa,
				 double b,
				 double fb){

    if (f.isZero(0)){
        int idx;
        idx=upper_level(levels,0,vtol);
        if (idx<(int)levels.size()&&fabs(levels.at(idx))<=vtol){
            roots[idx].push_back(a);
            roots[idx].push_back(b);
        }
        return;
    }
////usefull?
//     if (f.size()==1){
//         int idxa=upper_level(levels,fa);
//         int idxb=upper_level(levels,fb);
//         if (fa==fb){
//             if (fa==levels[idxa]){
//                 roots[a]=idxa;
//                 roots[b]=idxa;
//             }
//             return;
//         }
//         int idx_min=std::min(idxa,idxb);
//         int idx_max=std::max(idxa,idxb);
//         if (idx_max==levels.size()) idx_max-=1;
//         for(int i=idx_min;i<=idx_max; i++){
//             double t=a+(b-a)*(levels[i]-fa)/(fb-fa);
//             if(a<t&&t<b) roots[t]=i;
//         }
//         return;
//     }
    if ((b-a)<htol){
        //TODO: use different tol for t and f ?
        //TODO: unsigned idx ? (remove int casts when fixed)
        int idx=std::min(upper_level(levels,fa,vtol),upper_level(levels,fb,vtol));
        if (idx==(int)levels.size()) idx-=1;
        double c=levels.at(idx);
        if((fa-c)*(fb-c)<=0||fabs(fa-c)<vtol||fabs(fb-c)<vtol){
            roots[idx].push_back((a+b)/2);
        }
        return;
    }

    int idxa=upper_level(levels,fa,vtol);
    int idxb=upper_level(levels,fb,vtol);

    Interval bs = *bounds_local(df,Interval(a,b));

    //first times when a level (higher or lower) can be reached from a or b.
    double ta_hi,tb_hi,ta_lo,tb_lo;
    ta_hi=ta_lo=b+1;//default values => no root there.
    tb_hi=tb_lo=a-1;//default values => no root there.

    if (idxa<(int)levels.size() && fabs(fa-levels.at(idxa))<vtol){//a can be considered a root.
        //ta_hi=ta_lo=a;
        roots[idxa].push_back(a);
        ta_hi=ta_lo=a+htol;
    }else{
        if (bs.max()>0 && idxa<(int)levels.size())
            ta_hi=a+(levels.at(idxa  )-fa)/bs.max();
        if (bs.min()<0 && idxa>0)
            ta_lo=a+(levels.at(idxa-1)-fa)/bs.min();
    }
    if (idxb<(int)levels.size() && fabs(fb-levels.at(idxb))<vtol){//b can be considered a root.
        //tb_hi=tb_lo=b;
        roots[idxb].push_back(b);
        tb_hi=tb_lo=b-htol;
    }else{
        if (bs.min()<0 && idxb<(int)levels.size())
            tb_hi=b+(levels.at(idxb  )-fb)/bs.min();
        if (bs.max()>0 && idxb>0)
            tb_lo=b+(levels.at(idxb-1)-fb)/bs.max();
    }

    double t0,t1;
    t0=std::min(ta_hi,ta_lo);
    t1=std::max(tb_hi,tb_lo);
    //hum, rounding errors frighten me! so I add this +tol...
    if (t0>t1+htol) return;//no root here.

    if (fabs(t1-t0)<htol){
        multi_roots_internal(f,df,levels,roots,htol,vtol,t0,f(t0),t1,f(t1));
    }else{
        double t,t_left,t_right,ft,ft_left,ft_right;
        t_left =t_right =t =(t0+t1)/2;
        ft_left=ft_right=ft=f(t);
        int idx=upper_level(levels,ft,vtol);
        if (idx<(int)levels.size() && fabs(ft-levels.at(idx))<vtol){//t can be considered a root.
            roots[idx].push_back(t);
            //we do not want to count it twice (from the left and from the right)
            t_left =t-htol/2;
            t_right=t+htol/2;
            ft_left =f(t_left);
            ft_right=f(t_right);
        }
        multi_roots_internal(f,df,levels,roots,htol,vtol,t0     ,f(t0)   ,t_left,ft_left);
        multi_roots_internal(f,df,levels,roots,htol,vtol,t_right,ft_right,t1    ,f(t1)  );
    }
}

/** Solve f(t)=c for several c at once.
    \param f sbasis function
    \param levels vector of 'y' values
    \param htol, vtol 
    \param a, b left and right bounds
    \returns a vector of vectors, one for each y giving roots

Effectively computes:
results = roots(f(y_i)) for all y_i

* algo: -compute f at both ends of the given segment [a,b].
         -compute bounds m<df(t)<M for df on the segment.
         let c and C be the levels below and above f(a):
         going from f(a) down to c with slope m takes at least time (f(a)-c)/m
         going from f(a)  up  to C with slope M takes at least time (C-f(a))/M
         From this we conclude there are no roots before a'=a+min((f(a)-c)/m,(C-f(a))/M).
         Do the same for b: compute some b' such that there are no roots in (b',b].
         -if [a',b'] is not empty, repeat the process with [a',(a'+b')/2] and [(a'+b')/2,b'].
  unfortunately, extra care is needed about rounding errors, and also to avoid the repetition of roots,
  making things tricky and unpleasant...

TODO: Make sure the code is "rounding-errors proof" and take care about repetition of roots!
*/
std::vector<std::vector<double> > multi_roots(SBasis const &f,
                                      std::vector<double> const &levels,
                                      double htol,
                                      double vtol,
                                      double a,
                                      double b){

    std::vector<std::vector<double> > roots(levels.size(), std::vector<double>());

    SBasis df=derivative(f);
    multi_roots_internal(f,df,levels,roots,htol,vtol,a,f(a),b,f(b));

    return(roots);
}


static bool compareIntervalMin( Interval I, Interval J ){
	return I.min()<J.min();
}
static bool compareIntervalMax( Interval I, Interval J ){
	return I.max()<J.max();
}

//find the first interval whose max is >= x
static unsigned upper_level(vector<Interval> const &levels, double x ){
    return( lower_bound( levels.begin(), levels.end(), Interval(x,x), compareIntervalMax) - levels.begin() );
}

static std::vector<Interval> fuseContiguous(std::vector<Interval> const &sets, double tol=0.){
	std::vector<Interval> result;
	if (sets.empty() ) return result;
	result.push_back( sets.front() );
	for (unsigned i=1; i < sets.size(); i++ ){
		if ( result.back().max() + tol >= sets[i].min() ){
			result.back().unionWith( sets[i] );
		}else{
			result.push_back( sets[i] );
		}
	}
	return result;
}

/** level_sets internal method.
* algorithm: (~adaptation of Newton method versus 'accroissements finis')
   -compute f at both ends of the given segment [a,b].
   -compute bounds m<df(t)<M for df on the segment.
    Suppose f(a) is between two 'levels' c and C. Then
      f wont enter c before a + (f(a)-c.max())/m
      f wont enter C before a + (C.min()-f(a))/M
    From this we conclude nothing happens before a'=a+min((f(a)-c.max())/m,(C.min()-f(a))/M).
    We do the same for b: compute some b' such that nothing happens in (b',b].
    -if [a',b'] is not empty, repeat the process with [a',(a'+b')/2] and [(a'+b')/2,b'].

    If f(a) or f(b) belongs to some 'level' C, then use the same argument to find a' or b' such
    that f remains in C on [a,a'] or [b',b]. In case f is monotonic, we also know f won't enter another
    level before or after some time, allowing us to restrict the search a little more.

  unfortunately, extra care is needed about rounding errors, and also to avoid the repetition of roots,
  making things tricky and unpleasant...
*/

static void level_sets_internal(SBasis const &f,
				 SBasis const &df,
				 std::vector<Interval> const &levels,
				 std::vector<std::vector<Interval> > &solsets,
				 double a,
				 double fa,
				 double b,
				 double fb,
				 double tol=1e-5){

    if (f.isZero(0)){
        unsigned idx;
        idx=upper_level( levels, 0. );
        if (idx<levels.size() && levels[idx].contains(0.)){
            solsets[idx].push_back( Interval(a,b) ) ;
        }
        return;
    }

    unsigned idxa=upper_level(levels,fa);
    unsigned idxb=upper_level(levels,fb);

    Interval bs = *bounds_local(df,Interval(a,b));

    //first times when a level (higher or lower) can be reached from a or b.
    double ta_hi; // f remains below next level for t<ta_hi
    double ta_lo; // f remains above prev level for t<ta_lo
    double tb_hi; // f remains below next level for t>tb_hi
    double tb_lo; // f remains above next level for t>tb_lo

    ta_hi=ta_lo=b+1;//default values => no root there.
    tb_hi=tb_lo=a-1;//default values => no root there.

    //--- if f(a) belongs to a level.-------
    if ( idxa < levels.size() && levels[idxa].contains( fa ) ){
    	//find the first time when we may exit this level.
    	ta_lo = a + ( levels[idxa].min() - fa)/bs.min();
    	ta_hi = a + ( levels[idxa].max() - fa)/bs.max();
    	if ( ta_lo < a || ta_lo > b ) ta_lo = b;
    	if ( ta_hi < a || ta_hi > b  ) ta_hi = b;
    	//move to that time for the next iteration.
    	solsets[idxa].push_back( Interval( a, std::min( ta_lo, ta_hi ) ) );
    }else{
        //--- if f(b) does not belong to a level.-------
        if ( idxa == 0 ){
        	ta_lo = b;
        }else{
        	ta_lo = a + ( levels[idxa-1].max() - fa)/bs.min();
        	if ( ta_lo < a ) ta_lo = b;
        }
        if ( idxa == levels.size() ){
        	ta_hi = b;
        }else{
        	ta_hi = a + ( levels[idxa].min() - fa)/bs.max();
        	if ( ta_hi < a ) ta_hi = b;
        }
    }

    //--- if f(b) belongs to a level.-------
    if (idxb<levels.size() && levels.at(idxb).contains(fb)){
    	//find the first time from b when we may exit this level.
    	tb_lo = b + ( levels[idxb].min() - fb ) / bs.max();
    	tb_hi = b + ( levels[idxb].max() - fb ) / bs.min();
    	if ( tb_lo > b || tb_lo < a ) tb_lo = a;
    	if ( tb_hi > b || tb_hi < a ) tb_hi = a;
    	//move to that time for the next iteration.
    	solsets[idxb].push_back( Interval( std::max( tb_lo, tb_hi ), b) );
    }else{
        //--- if f(b) does not belong to a level.-------
        if ( idxb == 0 ){
        	tb_lo = a;
        }else{
        	tb_lo = b + ( levels[idxb-1].max() - fb)/bs.max();
        	if ( tb_lo > b ) tb_lo = a;
        }
        if ( idxb == levels.size() ){
        	tb_hi = a;
        }else{
        	tb_hi = b + ( levels[idxb].min() - fb)/bs.min();
        	if ( tb_hi > b ) tb_hi = a;
        }


    	if ( bs.min() < 0 && idxb < levels.size() )
            tb_hi = b + ( levels[idxb  ].min() - fb ) / bs.min();
        if ( bs.max() > 0 && idxb > 0 )
            tb_lo = b + ( levels[idxb-1].max() - fb ) / bs.max();
    }

    //let [t0,t1] be the next interval where to search.
    double t0=std::min(ta_hi,ta_lo);
    double t1=std::max(tb_hi,tb_lo);

    if (t0>=t1) return;//no root here.

    //if the interval is smaller than our resolution:
    //pretend f simultaneously meets all the levels between f(t0) and f(t1)...
    if ( t1 - t0 <= tol ){
    	Interval f_t0t1 ( f(t0), f(t1) );
    	unsigned idxmin = std::min(idxa, idxb);
    	unsigned idxmax = std::max(idxa, idxb);
    	//push [t0,t1] into all crossed level. Cheat to avoid overlapping intervals on different levels?
    	if ( idxmax > idxmin ){
        	for (unsigned idx = idxmin; idx < idxmax; idx++){
    			solsets[idx].push_back( Interval( t0, t1 ) );
        	}
    	}
    	if ( idxmax < levels.size() && f_t0t1.intersects( levels[idxmax] ) ){
    		solsets[idxmax].push_back( Interval( t0, t1 ) );
    	}
    	return;
    }

	//To make sure we finally exit the level jump at least by tol:
    t0 = std::min( std::max( t0, a + tol ), b );
    t1 = std::max( std::min( t1, b - tol ), a );

    double t =(t0+t1)/2;
    double ft=f(t);
    level_sets_internal( f, df, levels, solsets, t0, f(t0), t, ft );
    level_sets_internal( f, df, levels, solsets, t, ft, t1, f(t1) );
}

std::vector<std::vector<Interval> > level_sets(SBasis const &f,
                                      std::vector<Interval> const &levels,
                                      double a, double b, double tol){

    std::vector<std::vector<Interval> > solsets(levels.size(), std::vector<Interval>());

    SBasis df=derivative(f);
    level_sets_internal(f,df,levels,solsets,a,f(a),b,f(b),tol);
    // Fuse overlapping intervals...
    for (unsigned i=0; i<solsets.size(); i++){
    	if ( solsets[i].size() == 0 ) continue;
    	std::sort( solsets[i].begin(), solsets[i].end(), compareIntervalMin );
    	solsets[i] = fuseContiguous( solsets[i], tol );
    }
    return solsets;
}

std::vector<Interval> level_set (SBasis const &f, double level, double vtol, double a, double b, double tol){
	Interval fat_level( level - vtol, level + vtol );
	return level_set(f, fat_level, a, b, tol);
}
std::vector<Interval> level_set (SBasis const &f, Interval const &level, double a, double b, double tol){
	std::vector<Interval> levels(1,level);
	return level_sets(f,levels, a, b, tol).front() ;
}
std::vector<std::vector<Interval> > level_sets (SBasis const &f, std::vector<double> const &levels, double vtol, double a, double b, double tol){
	std::vector<Interval> fat_levels( levels.size(), Interval());
	for (unsigned i = 0; i < levels.size(); i++){
		fat_levels[i] = Interval( levels[i]-vtol, levels[i]+vtol);
	}
	return level_sets(f, fat_levels, a, b, tol);
}


//-------------------------------------
//-------------------------------------


void subdiv_sbasis(SBasis const & s,
                   std::vector<double> & roots,
                   double left, double right) {
    OptInterval bs = bounds_fast(s);
    if(!bs || bs->min() > 0 || bs->max() < 0)
        return; // no roots here
    if(s.tailError(1) < 1e-7) {
        double t = s[0][0] / (s[0][0] - s[0][1]);
        roots.push_back(left*(1-t) + t*right);
        return;
    }
    double middle = (left + right)/2;
    subdiv_sbasis(compose(s, Linear(0, 0.5)), roots, left, middle);
    subdiv_sbasis(compose(s, Linear(0.5, 1.)), roots, middle, right);
}

// It is faster to use the bernstein root finder for small degree polynomials (<100?.

std::vector<double> roots1(SBasis const & s) {
    std::vector<double> res;
    double d = s[0][0] - s[0][1];
    if(d != 0) {
        double r = s[0][0] / d;
        if(0 <= r && r <= 1)
            res.push_back(r);
    }
    return res;
}

std::vector<double> roots1(SBasis const & s, Interval const ivl) {
    std::vector<double> res;
    double d = s[0][0] - s[0][1];
    if(d != 0) {
        double r = s[0][0] / d;
        if(ivl.contains(r))
            res.push_back(r);
    }
    return res;
}

/** Find all t s.t s(t) = 0
 \param a sbasis function
 \see Bezier::roots
 \returns vector of zeros (roots)

*/
std::vector<double> roots(SBasis const & s) {
    switch(s.size()) {
        case 0:
            assert(false);
            return std::vector<double>();
        case 1:
            return roots1(s);
        default:
        {
            Bezier bz;
            sbasis_to_bezier(bz, s);
            return bz.roots();
        }
    }
}
std::vector<double> roots(SBasis const & s, Interval const ivl) {
    switch(s.size()) {
        case 0:
        	assert(false);
            return std::vector<double>();
        case 1:
            return roots1(s, ivl);
        default:
        {
            Bezier bz;
            sbasis_to_bezier(bz, s);
            return bz.roots(ivl);
        }
    }
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
