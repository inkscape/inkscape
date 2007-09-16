#include "sbasis-geometric.h"
#include "sbasis.h"
#include "sbasis-math.h"
//#include "solver.h"
#include "sbasis-geometric.h"

/** Geometric operators on D2<SBasis> (1D->2D).
 * Copyright 2007 JF Barraud
 * Copyright 2007 N Hurst
 *
 * The functions defined in this header related to 2d geometric operations such as arc length,
 * unit_vector, curvature, and centroid.  Most are built on top of unit_vector, which takes an
 * arbitrary D2 and returns a D2 with unit length with the same direction.
 *
 * Todo/think about:
 *  arclength D2 -> sbasis (giving arclength function)
 *  does uniform_speed return natural parameterisation?
 *  integrate sb2d code from normal-bundle
 *  angle(md<2>) -> sbasis (gives angle from vector - discontinuous?)
 *  osculating circle center?
 *  
 **/

//namespace Geom{
using namespace Geom;
using namespace std;

//Some utils first.
//TODO: remove this!! 
static vector<double> 
vect_intersect(vector<double> const &a, vector<double> const &b, double tol=0.){
    vector<double> inter;
    unsigned i=0,j=0;
    while ( i<a.size() && j<b.size() ){
        if (fabs(a[i]-b[j])<tol){
            inter.push_back(a[i]);
            i+=1;
            j+=1;
        }else if (a[i]<b[j]){
            i+=1;
        }else if (a[i]>b[j]){
            j+=1;
        }
    }
    return inter;
}

static SBasis divide_by_sk(SBasis const &a, int k) {
    assert( k<(int)a.size());
    if(k < 0) return shift(a,-k);
    SBasis c;
    c.insert(c.begin(), a.begin()+k, a.end());
    return c;
}

static SBasis divide_by_t0k(SBasis const &a, int k) {
    if(k < 0) {
        SBasis c = Linear(0,1);
        for (int i=2; i<=-k; i++){
            c*=c;
        }
        c*=a;
        return(c);
    }else{
        SBasis c = Linear(1,0);
        for (int i=2; i<=k; i++){
            c*=c;
        }
        c*=a;
        return(divide_by_sk(c,k));
    }
}

static SBasis divide_by_t1k(SBasis const &a, int k) {
    if(k < 0) {
        SBasis c = Linear(1,0);
        for (int i=2; i<=-k; i++){
            c*=c;
        }
        c*=a;
        return(c);
    }else{
        SBasis c = Linear(0,1);
        for (int i=2; i<=k; i++){
            c*=c;
        }
        c*=a;
        return(divide_by_sk(c,k));
    }
}

static D2<SBasis> RescaleForNonVanishingEnds(D2<SBasis> const &MM, double ZERO=1.e-4){
    D2<SBasis> M = MM;
    //TODO: divide by all the s at once!!!
    while (fabs(M[0].at0())<ZERO && 
           fabs(M[1].at0())<ZERO &&
           fabs(M[0].at1())<ZERO && 
           fabs(M[1].at1())<ZERO){
        M[0] = divide_by_sk(M[0],1);
        M[1] = divide_by_sk(M[1],1);
    }
    while (fabs(M[0].at0())<ZERO && fabs(M[1].at0())<ZERO){
        M[0] = divide_by_t0k(M[0],1);
        M[1] = divide_by_t0k(M[1],1);
    }
    while (fabs(M[0].at1())<ZERO && fabs(M[1].at1())<ZERO){
        M[0] = divide_by_t1k(M[0],1);
        M[1] = divide_by_t1k(M[1],1);
    }
    return M;
}

//=================================================================
//TODO: what's this for?!?!
Piecewise<D2<SBasis> > 
Geom::cutAtRoots(Piecewise<D2<SBasis> > const &M, double ZERO){
    vector<double> rts;
    for (unsigned i=0; i<M.size(); i++){
        vector<double> seg_rts = roots((M.segs[i])[0]);
        seg_rts = vect_intersect(seg_rts, roots((M.segs[i])[1]), ZERO);
        Linear mapToDom = Linear(M.cuts[i],M.cuts[i+1]);
        for (unsigned r=0; r<seg_rts.size(); r++){
            seg_rts[r]= mapToDom(seg_rts[r]);
        }
        rts.insert(rts.end(),seg_rts.begin(),seg_rts.end());
    }
    return partition(M,rts);
}

Piecewise<SBasis>
Geom::atan2(Piecewise<D2<SBasis> > const &vect, double tol, unsigned order){
    Piecewise<SBasis> result;
    Piecewise<D2<SBasis> > v = cutAtRoots(vect);
    result.cuts.push_back(v.cuts.front());
    for (unsigned i=0; i<v.size(); i++){

        D2<SBasis> vi = RescaleForNonVanishingEnds(v.segs[i]);
        SBasis x=vi[0], y=vi[1];
        Piecewise<SBasis> angle;
        angle = divide (x*derivative(y)-y*derivative(x), x*x+y*y, tol, order);

        //TODO: I don't understand this - sign.
        angle = integral(-angle);
        Point vi0 = vi.at0(); 
        angle += -std::atan2(vi0[1],vi0[0]) - angle[0].at0();
        //TODO: deal with 2*pi jumps form one seg to the other...
        //TODO: not exact at t=1 because of the integral.
        //TODO: force continuity?

        angle.setDomain(Interval(v.cuts[i],v.cuts[i+1]));
        result.concat(angle);   
    }
    return result;
}
Piecewise<SBasis>
Geom::atan2(D2<SBasis> const &vect, double tol, unsigned order){
    return atan2(Piecewise<D2<SBasis> >(vect),tol,order);
}

//unitVector(x,y) is computed as (b,-a) where a and b are solutions of:
//     ax+by=0 (eqn1)   and   a^2+b^2=1 (eqn2)
Piecewise<D2<SBasis> >
Geom::unitVector(D2<SBasis> const &V_in, double tol, unsigned order){
    D2<SBasis> V = RescaleForNonVanishingEnds(V_in);
    if (V[0].empty() && V[1].empty())
        return Piecewise<D2<SBasis> >(D2<SBasis>(Linear(1),SBasis()));
    SBasis x = V[0], y = V[1], a, b;
    SBasis r_eqn1, r_eqn2;

    Point v0 = unit_vector(V.at0());
    Point v1 = unit_vector(V.at1());
    a.push_back(Linear(-v0[1],-v1[1]));
    b.push_back(Linear( v0[0], v1[0]));

    r_eqn1 = -(a*x+b*y);
    r_eqn2 = Linear(1.)-(a*a+b*b);

    for (unsigned k=1; k<=order; k++){
        double r0  = (k<r_eqn1.size())? r_eqn1.at(k).at0() : 0;
        double r1  = (k<r_eqn1.size())? r_eqn1.at(k).at1() : 0;
        double rr0 = (k<r_eqn2.size())? r_eqn2.at(k).at0() : 0;
        double rr1 = (k<r_eqn2.size())? r_eqn2.at(k).at1() : 0;
        double a0,a1,b0,b1;// coeffs in a[k] and b[k]

        //the equations to solve at this point are:
        // a0*x(0)+b0*y(0)=r0 & 2*a0*a(0)+2*b0*b(0)=rr0
        //and
        // a1*x(1)+b1*y(1)=r1 & 2*a1*a(1)+2*b1*b(1)=rr1
        a0 = r0/dot(v0,V(0))*v0[0]-rr0/2*v0[1];
        b0 = r0/dot(v0,V(0))*v0[1]+rr0/2*v0[0];
        a1 = r1/dot(v1,V(1))*v1[0]-rr1/2*v1[1];
        b1 = r1/dot(v1,V(1))*v1[1]+rr1/2*v1[0];

        a.push_back(Linear(a0,a1));        
        b.push_back(Linear(b0,b1));
        //TODO: use "incremental" rather than explicit formulas.
        r_eqn1 = -(a*x+b*y);
        r_eqn2 = Linear(1)-(a*a+b*b);
    }
    
    //our candidate is:
    D2<SBasis> unitV;
    unitV[0] =  b;
    unitV[1] = -a;

    //is it good?
    double rel_tol = std::max(1.,std::max(V_in[0].tailError(0),V_in[1].tailError(0)))*tol;

    if (r_eqn1.tailError(order)>rel_tol || r_eqn2.tailError(order)>tol){
        //if not: subdivide and concat results.
        Piecewise<D2<SBasis> > unitV0, unitV1;
        unitV0 = unitVector(compose(V,Linear(0,.5)),tol,order);
        unitV1 = unitVector(compose(V,Linear(.5,1)),tol,order);
        unitV0.setDomain(Interval(0.,.5));
        unitV1.setDomain(Interval(.5,1.));
        unitV0.concat(unitV1);
        return(unitV0);
    }else{
        //if yes: return it as pw.
        Piecewise<D2<SBasis> > result;
        result=(Piecewise<D2<SBasis> >)unitV;
        return result;
    }
}

Piecewise<D2<SBasis> >
Geom::unitVector(Piecewise<D2<SBasis> > const &V, double tol, unsigned order){
    Piecewise<D2<SBasis> > result;
    Piecewise<D2<SBasis> > VV = cutAtRoots(V);
    result.cuts.push_back(VV.cuts.front());
    for (unsigned i=0; i<VV.size(); i++){
        Piecewise<D2<SBasis> > unit_seg;
        unit_seg = unitVector(VV.segs[i],tol, order);
        unit_seg.setDomain(Interval(VV.cuts[i],VV.cuts[i+1]));
        result.concat(unit_seg);   
    }
    return result;
}

Piecewise<SBasis> 
Geom::arcLengthSb(Piecewise<D2<SBasis> > const &M, double tol){
    Piecewise<D2<SBasis> > dM = derivative(M);
    Piecewise<SBasis> dMlength = sqrt(dot(dM,dM),tol,3);
    Piecewise<SBasis> length = integral(dMlength);
    length-=length.segs.front().at0();
    return length;
}
Piecewise<SBasis> 
Geom::arcLengthSb(D2<SBasis> const &M, double tol){
    return arcLengthSb(Piecewise<D2<SBasis> >(M), tol);
}

double
Geom::length(D2<SBasis> const &M,
                 double tol){
    Piecewise<SBasis> length = arcLengthSb(M, tol);
    return length.segs.back().at1();
}
double
Geom::length(Piecewise<D2<SBasis> > const &M,
                 double tol){
    Piecewise<SBasis> length = arcLengthSb(M, tol);
    return length.segs.back().at1();
}


// incomplete.
Piecewise<SBasis>
Geom::curvature(D2<SBasis> const &M, double tol) {
    D2<SBasis> dM=derivative(M);
    Piecewise<SBasis> result;
    Piecewise<D2<SBasis> > unitv = unitVector(dM,tol);
    Piecewise<SBasis> dMlength = dot(Piecewise<D2<SBasis> >(dM),unitv);
    Piecewise<SBasis> k = cross(derivative(unitv),unitv);
    k = divide(k,dMlength,tol,3);
    return(k);
}

Piecewise<SBasis> 
Geom::curvature(Piecewise<D2<SBasis> > const &V, double tol){
    Piecewise<SBasis> result;
    Piecewise<D2<SBasis> > VV = cutAtRoots(V);
    result.cuts.push_back(VV.cuts.front());
    for (unsigned i=0; i<VV.size(); i++){
        Piecewise<SBasis> curv_seg;
        curv_seg = curvature(VV.segs[i],tol);
        curv_seg.setDomain(Interval(VV.cuts[i],VV.cuts[i+1]));
        result.concat(curv_seg);
    }
    return result;
}

//=================================================================

Piecewise<D2<SBasis> >
Geom::arc_length_parametrization(D2<SBasis> const &M,
                           unsigned order,
                           double tol){
    Piecewise<D2<SBasis> > u;
    u.push_cut(0);

    Piecewise<SBasis> s = arcLengthSb(Piecewise<D2<SBasis> >(M),tol);
    for (unsigned i=0; i < s.size();i++){
        double t0=s.cuts[i],t1=s.cuts[i+1];
        D2<SBasis> sub_M = compose(M,Linear(t0,t1));
        D2<SBasis> sub_u;
        for (unsigned dim=0;dim<2;dim++){
            SBasis sub_s = s.segs[i];
            sub_s-=sub_s.at0();
            sub_s/=sub_s.at1();
            sub_u[dim]=compose_inverse(sub_M[dim],sub_s, order, tol);
        }
        u.push(sub_u,s(t1));
    }
    return u;
}

Piecewise<D2<SBasis> >
Geom::arc_length_parametrization(Piecewise<D2<SBasis> > const &M,
                                 unsigned order,
                                 double tol){
    Piecewise<D2<SBasis> > result;
    for (unsigned i=0; i<M.size(); i++ ){
        Piecewise<D2<SBasis> > uniform_seg=arc_length_parametrization(M[i],order,tol);
        result.concat(uniform_seg);
    }
    return(result);
}

/** centroid using sbasis integration.
 * This approach uses green's theorem to compute the area and centroid using integrals.  For curved
 * shapes this is much faster than converting to polyline.

 * Returned values: 
    0 for normal execution;
    2 if area is zero, meaning centroid is meaningless.

 * Copyright Nathan Hurst 2006
 */

unsigned Geom::centroid(Piecewise<D2<SBasis> > const &p, Point& centroid, double &area) {
    Point centroid_tmp(0,0);
    double atmp = 0;
    for(unsigned i = 0; i < p.size(); i++) {
        SBasis curl = dot(p[i], rot90(derivative(p[i])));
        SBasis A = integral(curl);
        D2<SBasis> C = integral(multiply(curl, p[i]));
        atmp += A.at1() - A.at0();
        centroid_tmp += C.at1()- C.at0(); // first moment.
    }
// join ends
    centroid_tmp *= 2;
    Point final = p[p.size()-1].at1(), initial = p[0].at0();
    const double ai = cross(final, initial);
    atmp += ai;
    centroid_tmp += (final + initial)*ai; // first moment.
    
    area = atmp / 2;
    if (atmp != 0) {
        centroid = centroid_tmp / (3 * atmp);
        return 0;
    }
    return 2;
}

//}; // namespace


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
