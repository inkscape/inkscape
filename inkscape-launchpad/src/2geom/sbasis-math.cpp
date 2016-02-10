/*
 *  sbasis-math.cpp - some std functions to work with (pw)s-basis
 *
 *  Authors:
 *   Jean-Francois Barraud
 *
 * Copyright (C) 2006-2007 authors
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

//this a first try to define sqrt, cos, sin, etc...
//TODO: define a truncated compose(sb,sb, order) and extend it to pw<sb>.
//TODO: in all these functions, compute 'order' according to 'tol'.

#include <2geom/d2.h>
#include <2geom/sbasis-math.h>
#include <stdio.h>
#include <math.h>
//#define ZERO 1e-3


namespace Geom {


//-|x|-----------------------------------------------------------------------
/** Return the absolute value of a function pointwise.
 \param f function
*/
Piecewise<SBasis> abs(SBasis const &f){
    return abs(Piecewise<SBasis>(f));
}
/** Return the absolute value of a function pointwise.
 \param f function
*/
Piecewise<SBasis> abs(Piecewise<SBasis> const &f){
    Piecewise<SBasis> absf=partition(f,roots(f));
    for (unsigned i=0; i<absf.size(); i++){
        if (absf.segs[i](.5)<0) absf.segs[i]*=-1;
    }
    return absf;
}

//-max(x,y), min(x,y)--------------------------------------------------------
/** Return the greater of the two functions pointwise.
 \param f, g two functions
*/
Piecewise<SBasis> max(          SBasis  const &f,           SBasis  const &g){
    return max(Piecewise<SBasis>(f),Piecewise<SBasis>(g));
}
/** Return the greater of the two functions pointwise.
 \param f, g two functions
*/
Piecewise<SBasis> max(Piecewise<SBasis> const &f,           SBasis  const &g){
    return max(f,Piecewise<SBasis>(g));
}
/** Return the greater of the two functions pointwise.
 \param f, g two functions
*/
Piecewise<SBasis> max(          SBasis  const &f, Piecewise<SBasis> const &g){
    return max(Piecewise<SBasis>(f),g);
}
/** Return the greater of the two functions pointwise.
 \param f, g two functions
*/
Piecewise<SBasis> max(Piecewise<SBasis> const &f, Piecewise<SBasis> const &g){
    Piecewise<SBasis> max=partition(f,roots(f-g));
    Piecewise<SBasis> gg =partition(g,max.cuts);
    max = partition(max,gg.cuts);
    for (unsigned i=0; i<max.size(); i++){
        if (max.segs[i](.5)<gg.segs[i](.5)) max.segs[i]=gg.segs[i];
    }
    return max;
}

/** Return the more negative of the two functions pointwise.
 \param f, g two functions
*/
Piecewise<SBasis> 
min(          SBasis  const &f,           SBasis  const &g){ return -max(-f,-g); }
/** Return the more negative of the two functions pointwise.
 \param f, g two functions
*/
Piecewise<SBasis> 
min(Piecewise<SBasis> const &f,           SBasis  const &g){ return -max(-f,-g); }
/** Return the more negative of the two functions pointwise.
 \param f, g two functions
*/
Piecewise<SBasis> 
min(          SBasis  const &f, Piecewise<SBasis> const &g){ return -max(-f,-g); }
/** Return the more negative of the two functions pointwise.
 \param f, g two functions
*/
Piecewise<SBasis> 
min(Piecewise<SBasis> const &f, Piecewise<SBasis> const &g){ return -max(-f,-g); }


//-sign(x)---------------------------------------------------------------
/** Return the sign of the two functions pointwise.
 \param f function
*/
Piecewise<SBasis> signSb(SBasis const &f){
    return signSb(Piecewise<SBasis>(f));
}
/** Return the sign of the two functions pointwise.
 \param f function
*/
Piecewise<SBasis> signSb(Piecewise<SBasis> const &f){
    Piecewise<SBasis> sign=partition(f,roots(f));
    for (unsigned i=0; i<sign.size(); i++){
        sign.segs[i] = (sign.segs[i](.5)<0)? Linear(-1.):Linear(1.);
    }
    return sign;
}

//-Sqrt----------------------------------------------------------
static Piecewise<SBasis> sqrt_internal(SBasis const &f, 
                                    double tol, 
                                    int order){
    SBasis sqrtf;
    if(f.isZero() || order == 0){
        return Piecewise<SBasis>(sqrtf);
    }
    if (f.at0()<-tol*tol && f.at1()<-tol*tol){
        return sqrt_internal(-f,tol,order);
    }else if (f.at0()>tol*tol && f.at1()>tol*tol){
        sqrtf.resize(order+1, Linear(0,0));
        sqrtf[0] = Linear(std::sqrt(f[0][0]), std::sqrt(f[0][1]));
        SBasis r = f - multiply(sqrtf, sqrtf); // remainder    
        for(unsigned i = 1; int(i) <= order && i<r.size(); i++) {
            Linear ci(r[i][0]/(2*sqrtf[0][0]), r[i][1]/(2*sqrtf[0][1]));
            SBasis cisi = shift(ci, i);
            r -= multiply(shift((sqrtf*2 + cisi), i), SBasis(ci));
            r.truncate(order+1);
            sqrtf[i] = ci;
            if(r.tailError(i) == 0) // if exact
                break;
        }
    }else{
        sqrtf = Linear(std::sqrt(fabs(f.at0())), std::sqrt(fabs(f.at1())));
    }

    double err = (f - multiply(sqrtf, sqrtf)).tailError(0);
    if (err<tol){
        return Piecewise<SBasis>(sqrtf);
    }

    Piecewise<SBasis> sqrtf0,sqrtf1;
    sqrtf0 = sqrt_internal(compose(f,Linear(0.,.5)),tol,order);
    sqrtf1 = sqrt_internal(compose(f,Linear(.5,1.)),tol,order);
    sqrtf0.setDomain(Interval(0.,.5));
    sqrtf1.setDomain(Interval(.5,1.));
    sqrtf0.concat(sqrtf1);
    return sqrtf0;
}

/** Compute the sqrt of a function.
 \param f function
*/
Piecewise<SBasis> sqrt(SBasis const &f, double tol, int order){
    return sqrt(max(f,Linear(tol*tol)),tol,order);
}

/** Compute the sqrt of a function.
 \param f function
*/
Piecewise<SBasis> sqrt(Piecewise<SBasis> const &f, double tol, int order){
    Piecewise<SBasis> result;
    Piecewise<SBasis> zero = Piecewise<SBasis>(Linear(tol*tol));
    zero.setDomain(f.domain());
    Piecewise<SBasis> ff=max(f,zero);

    for (unsigned i=0; i<ff.size(); i++){
        Piecewise<SBasis> sqrtfi = sqrt_internal(ff.segs[i],tol,order);
        sqrtfi.setDomain(Interval(ff.cuts[i],ff.cuts[i+1]));
        result.concat(sqrtfi);
    }
    return result;
}

//-Yet another sin/cos--------------------------------------------------------------

/** Compute the sine of a function.
 \param f function
 \param tol maximum error
 \param order maximum degree polynomial to use
*/
Piecewise<SBasis> sin(          SBasis  const &f, double tol, int order){return(cos(-f+M_PI/2,tol,order));}
/** Compute the sine of a function.
 \param f function
 \param tol maximum error
 \param order maximum degree polynomial to use
*/
Piecewise<SBasis> sin(Piecewise<SBasis> const &f, double tol, int order){return(cos(-f+M_PI/2,tol,order));}

/** Compute the cosine of a function.
 \param f function
 \param tol maximum error
 \param order maximum degree polynomial to use
*/
Piecewise<SBasis> cos(Piecewise<SBasis> const &f, double tol, int order){
    Piecewise<SBasis> result;
    for (unsigned i=0; i<f.size(); i++){
        Piecewise<SBasis> cosfi = cos(f.segs[i],tol,order);
        cosfi.setDomain(Interval(f.cuts[i],f.cuts[i+1]));
        result.concat(cosfi);
    }
    return result;
}

/** Compute the cosine of a function.
 \param f function
 \param tol maximum error
 \param order maximum degree polynomial to use
*/
Piecewise<SBasis> cos(          SBasis  const &f, double tol, int order){
    double alpha = (f.at0()+f.at1())/2.;
    SBasis x = f-alpha;
    double d = x.tailError(0),err=1;
    //estimate cos(x)-sum_0^order (-1)^k x^2k/2k! by the first neglicted term
    for (int i=1; i<=2*order; i++) err*=d/i;
    
    if (err<tol){
        SBasis xk=Linear(1), c=Linear(1), s=Linear(0);
        for (int k=1; k<=2*order; k+=2){
            xk*=x/k;
            //take also truncature errors into account...
            err+=xk.tailError(order);
            xk.truncate(order);
            s+=xk;
            xk*=-x/(k+1);
            //take also truncature errors into account...
            err+=xk.tailError(order);
            xk.truncate(order);
            c+=xk;
        }
        if (err<tol){
            return Piecewise<SBasis>(std::cos(alpha)*c-std::sin(alpha)*s);
        }
    }
    Piecewise<SBasis> c0,c1;
    c0 = cos(compose(f,Linear(0.,.5)),tol,order);
    c1 = cos(compose(f,Linear(.5,1.)),tol,order);
    c0.setDomain(Interval(0.,.5));
    c1.setDomain(Interval(.5,1.));
    c0.concat(c1);
    return c0;
}

//--1/x------------------------------------------------------------
//TODO: this implementation is just wrong. Remove or redo!

void truncateResult(Piecewise<SBasis> &f, int order){
    if (order>=0){
        for (unsigned k=0; k<f.segs.size(); k++){
            f.segs[k].truncate(order);
        }
    }
}

Piecewise<SBasis> reciprocalOnDomain(Interval range, double tol){
    Piecewise<SBasis> reciprocal_fn;
    //TODO: deduce R from tol...
    double R=2.;
    SBasis reciprocal1_R=reciprocal(Linear(1,R),3);
    double a=range.min(), b=range.max();
    if (a*b<0){
        b=std::max(fabs(a),fabs(b));
        a=0;
    }else if (b<0){
        a=-range.max();
        b=-range.min();
    }

    if (a<=tol){
        reciprocal_fn.push_cut(0);
        int i0=(int) floor(std::log(tol)/std::log(R));
        a = std::pow(R,i0);
        reciprocal_fn.push(Linear(1/a),a);
    }else{
        int i0=(int) floor(std::log(a)/std::log(R));
        a = std::pow(R,i0);
        reciprocal_fn.cuts.push_back(a);
    }  

    while (a<b){
        reciprocal_fn.push(reciprocal1_R/a,R*a);
        a*=R;
    }
    if (range.min()<0 || range.max()<0){
        Piecewise<SBasis>reciprocal_fn_neg;
        //TODO: define reverse(pw<sb>);
        reciprocal_fn_neg.cuts.push_back(-reciprocal_fn.cuts.back());
        for (unsigned i=0; i<reciprocal_fn.size(); i++){
            int idx=reciprocal_fn.segs.size()-1-i;
            reciprocal_fn_neg.push_seg(-reverse(reciprocal_fn.segs.at(idx)));
            reciprocal_fn_neg.push_cut(-reciprocal_fn.cuts.at(idx));
        }
        if (range.max()>0){
            reciprocal_fn_neg.concat(reciprocal_fn);
        }
        reciprocal_fn=reciprocal_fn_neg;
    }

    return(reciprocal_fn);
}

Piecewise<SBasis> reciprocal(SBasis const &f, double tol, int order){
    Piecewise<SBasis> reciprocal_fn=reciprocalOnDomain(*bounds_fast(f), tol);
    Piecewise<SBasis> result=compose(reciprocal_fn,f);
    truncateResult(result,order);
    return(result);
}
Piecewise<SBasis> reciprocal(Piecewise<SBasis> const &f, double tol, int order){
    Piecewise<SBasis> reciprocal_fn=reciprocalOnDomain(*bounds_fast(f), tol);
    Piecewise<SBasis> result=compose(reciprocal_fn,f);
    truncateResult(result,order);
    return(result);
}

/**
 * \brief Retruns a Piecewise SBasis with prescribed values at prescribed times.
 * 
 * \param times: vector of times at which the values are given. Should be sorted in increasing order.
 * \param values: vector of prescribed values. Should have the same size as times and be sorted accordingly.
 * \param smoothness: (defaults to 1) regularity class of the result: 0=piecewise linear, 1=continuous derivative, etc...
 */
Piecewise<SBasis> interpolate(std::vector<double> times, std::vector<double> values, unsigned smoothness){
    assert ( values.size() == times.size() );
    if ( values.empty() ) return Piecewise<SBasis>();
    if ( values.size() == 1 ) return Piecewise<SBasis>(values[0]);//what about time??

    SBasis sk = shift(Linear(1.),smoothness);
    SBasis bump_in = integral(sk);
    bump_in -= bump_in.at0();
    bump_in /= bump_in.at1();
    SBasis bump_out = reverse( bump_in );
    
    Piecewise<SBasis> result;
    result.cuts.push_back(times[0]);
    for (unsigned i = 0; i<values.size()-1; i++){
        result.push(bump_out*values[i]+bump_in*values[i+1],times[i+1]);
    }
    return result;
}

}

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
