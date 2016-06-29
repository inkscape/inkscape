/** Geometric operators on D2<SBasis> (1D->2D).
 * Copyright 2012 JBC Engelen
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

#include <2geom/sbasis-geometric.h>
#include <2geom/sbasis.h>
#include <2geom/sbasis-math.h>
#include <2geom/sbasis-geometric.h>

//namespace Geom{
using namespace Geom;
using namespace std;

//Some utils first.
//TODO: remove this!! 
/** 
 * Return a list of doubles that appear in both a and b to within error tol
 * a, b, vector of double
 * tol tolerance
 */
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

//------------------------------------------------------------------------------
static SBasis divide_by_sk(SBasis const &a, int k) {
    if ( k>=(int)a.size()){
        //make sure a is 0?
        return SBasis();
    }
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
    while ((M[0].size()>1||M[1].size()>1) &&
           fabs(M[0].at0())<ZERO && 
           fabs(M[1].at0())<ZERO &&
           fabs(M[0].at1())<ZERO && 
           fabs(M[1].at1())<ZERO){
        M[0] = divide_by_sk(M[0],1);
        M[1] = divide_by_sk(M[1],1);
    }
    while ((M[0].size()>1||M[1].size()>1) &&
           fabs(M[0].at0())<ZERO && fabs(M[1].at0())<ZERO){
        M[0] = divide_by_t0k(M[0],1);
        M[1] = divide_by_t0k(M[1],1);
    }
    while ((M[0].size()>1||M[1].size()>1) && 
           fabs(M[0].at1())<ZERO && fabs(M[1].at1())<ZERO){
        M[0] = divide_by_t1k(M[0],1);
        M[1] = divide_by_t1k(M[1],1);
    }
    return M;
}

/*static D2<SBasis> RescaleForNonVanishing(D2<SBasis> const &MM, double ZERO=1.e-4){
    std::vector<double> levels;
    levels.push_back(-ZERO);
    levels.push_back(ZERO);
    //std::vector<std::vector<double> > mr = multi_roots(MM, levels);
    }*/


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

/** Return a function which gives the angle of vect at each point.
 \param vect a piecewise parameteric curve.
 \param tol the maximum error allowed.
 \param order the maximum degree to use for approximation
 \relates Piecewise
*/
Piecewise<SBasis>
Geom::atan2(Piecewise<D2<SBasis> > const &vect, double tol, unsigned order){
    Piecewise<SBasis> result;
    Piecewise<D2<SBasis> > v = cutAtRoots(vect,tol);
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
/** Return a function which gives the angle of vect at each point.
 \param vect a piecewise parameteric curve.
 \param tol the maximum error allowed.
 \param order the maximum degree to use for approximation
 \relates Piecewise, D2
*/
Piecewise<SBasis>
Geom::atan2(D2<SBasis> const &vect, double tol, unsigned order){
    return atan2(Piecewise<D2<SBasis> >(vect),tol,order);
}

/** tan2 is the pseudo-inverse of atan2.  It takes an angle and returns a unit_vector that points in the direction of angle.
 \param angle a piecewise function of angle wrt t.
 \param tol the maximum error allowed.
 \param order the maximum degree to use for approximation
 \relates D2, SBasis
*/
D2<Piecewise<SBasis> >
Geom::tan2(SBasis const &angle, double tol, unsigned order){
    return tan2(Piecewise<SBasis>(angle), tol, order);
}

/** tan2 is the pseudo-inverse of atan2.  It takes an angle and returns a unit_vector that points in the direction of angle.
 \param angle a piecewise function of angle wrt t.
 \param tol the maximum error allowed.
 \param order the maximum degree to use for approximation
 \relates Piecewise, D2
*/
D2<Piecewise<SBasis> >
Geom::tan2(Piecewise<SBasis> const &angle, double tol, unsigned order){
    return D2<Piecewise<SBasis> >(cos(angle, tol, order), sin(angle, tol, order));
}

/** Return a Piecewise<D2<SBasis> > which points in the same direction as V_in, but has unit_length.
 \param V_in the original path.
 \param tol the maximum error allowed.
 \param order the maximum degree to use for approximation

unitVector(x,y) is computed as (b,-a) where a and b are solutions of:
     ax+by=0 (eqn1)   and   a^2+b^2=1 (eqn2)

 \relates Piecewise, D2
*/
Piecewise<D2<SBasis> >
Geom::unitVector(D2<SBasis> const &V_in, double tol, unsigned order){
    //TODO: Handle vanishing vectors...
    // -This approach is numerically bad. Find a stable way to rescale V_in to have non vanishing ends.
    // -This done, unitVector will have jumps at zeros: fill the gaps with arcs of circles.
    D2<SBasis> V = RescaleForNonVanishingEnds(V_in);
    if (V[0].isZero(tol) && V[1].isZero(tol))
        return Piecewise<D2<SBasis> >(D2<SBasis>(Linear(1),SBasis()));

    SBasis x = V[0], y = V[1];
    SBasis r_eqn1, r_eqn2;

    Point v0 = unit_vector(V.at0());
    Point v1 = unit_vector(V.at1());
    SBasis a = SBasis(order+1, Linear(0.));
    a[0] = Linear(-v0[1],-v1[1]);
    SBasis b = SBasis(order+1, Linear(0.));
    b[0] = Linear( v0[0], v1[0]);

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
        a0 = r0/dot(v0,V.at0())*v0[0]-rr0/2*v0[1];
        b0 = r0/dot(v0,V.at0())*v0[1]+rr0/2*v0[0];
        a1 = r1/dot(v1,V.at1())*v1[0]-rr1/2*v1[1];
        b1 = r1/dot(v1,V.at1())*v1[1]+rr1/2*v1[0];

        a[k] = Linear(a0,a1);
        b[k] = Linear(b0,b1);

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

/** Return a Piecewise<D2<SBasis> > which points in the same direction as V_in, but has unit_length.
 \param V_in the original path.
 \param tol the maximum error allowed.
 \param order the maximum degree to use for approximation

unitVector(x,y) is computed as (b,-a) where a and b are solutions of:
     ax+by=0 (eqn1)   and   a^2+b^2=1 (eqn2)

 \relates Piecewise
*/
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

/** returns a function giving the arclength at each point in M.
 \param M the Element.
 \param tol the maximum error allowed.
 \relates Piecewise
*/
Piecewise<SBasis> 
Geom::arcLengthSb(Piecewise<D2<SBasis> > const &M, double tol){
    Piecewise<D2<SBasis> > dM = derivative(M);
    Piecewise<SBasis> dMlength = sqrt(dot(dM,dM),tol,3);
    Piecewise<SBasis> length = integral(dMlength);
    length-=length.segs.front().at0();
    return length;
}

/** returns a function giving the arclength at each point in M.
 \param M the Element.
 \param tol the maximum error allowed.
 \relates Piecewise, D2
*/
Piecewise<SBasis> 
Geom::arcLengthSb(D2<SBasis> const &M, double tol){
    return arcLengthSb(Piecewise<D2<SBasis> >(M), tol);
}

#if 0
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
#endif

/** returns a function giving the curvature at each point in M.
 \param M the Element.
 \param tol the maximum error allowed.
 \relates Piecewise, D2
 \todo claimed incomplete.  Check.
*/
Piecewise<SBasis>
Geom::curvature(D2<SBasis> const &M, double tol) {
    D2<SBasis> dM=derivative(M);
    Piecewise<D2<SBasis> > unitv = unitVector(dM,tol);
    Piecewise<SBasis> dMlength = dot(Piecewise<D2<SBasis> >(dM),unitv);
    Piecewise<SBasis> k = cross(derivative(unitv),unitv);
    k = divide(k,dMlength,tol,3);
    return(k);
}

/** returns a function giving the curvature at each point in M.
 \param M the Element.
 \param tol the maximum error allowed.
 \relates Piecewise
 \todo claimed incomplete.  Check.
*/
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

/** Reparameterise M to have unit speed.
 \param M the Element.
 \param tol the maximum error allowed.
 \param order the maximum degree to use for approximation
 \relates Piecewise, D2
*/
Piecewise<D2<SBasis> >
Geom::arc_length_parametrization(D2<SBasis> const &M,
                           unsigned order,
                           double tol){
    Piecewise<D2<SBasis> > u;
    u.push_cut(0);

    Piecewise<SBasis> s = arcLengthSb(Piecewise<D2<SBasis> >(M),tol);
    for (unsigned i=0; i < s.size();i++){
        double t0=s.cuts[i],t1=s.cuts[i+1];
        if ( are_near(s(t0),s(t1)) ) {
            continue;
        }
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

/** Reparameterise M to have unit speed.
 \param M the Element.
 \param tol the maximum error allowed.
 \param order the maximum degree to use for approximation
 \relates Piecewise
*/
Piecewise<D2<SBasis> >
Geom::arc_length_parametrization(Piecewise<D2<SBasis> > const &M,
                                 unsigned order,
                                 double tol){
    Piecewise<D2<SBasis> > result;
    for (unsigned i=0; i<M.size(); i++) {
        result.concat( arc_length_parametrization(M[i],order,tol) );
    }
    return result;
}

#include <gsl/gsl_integration.h>
static double sb_length_integrating(double t, void* param) {
    SBasis* pc = (SBasis*)param;
    return sqrt((*pc)(t));
}

/** Calculates the length of a D2<SBasis> through gsl integration.
 \param B the Element.
 \param tol the maximum error allowed.
 \param result variable to be incremented with the length of the path
 \param abs_error variable to be incremented with the estimated error
 \relates D2
If you only want the length, this routine may be faster/more accurate.
*/
void Geom::length_integrating(D2<SBasis> const &B, double &result, double &abs_error, double tol) {
    D2<SBasis> dB = derivative(B);
    SBasis dB2 = dot(dB, dB);
        
    gsl_function F;
    gsl_integration_workspace * w 
        = gsl_integration_workspace_alloc (20);
    F.function = &sb_length_integrating;
    F.params = (void*)&dB2;
    double quad_result, err;
    /* We could probably use the non adaptive code here if we removed any cusps first. */
         
    gsl_integration_qag (&F, 0, 1, 0, tol, 20, 
                         GSL_INTEG_GAUSS21, w, &quad_result, &err);
        
    abs_error += err;
    result += quad_result;
}

/** Calculates the length of a D2<SBasis> through gsl integration.
 \param s the Element.
 \param tol the maximum error allowed.
 \relates D2
If you only want the total length, this routine faster and more accurate than constructing an arcLengthSb.
*/
double
Geom::length(D2<SBasis> const &s,
                 double tol){
    double result = 0;
    double abs_error = 0;
    length_integrating(s, result, abs_error, tol);
    return result;
}
/** Calculates the length of a Piecewise<D2<SBasis> > through gsl integration.
 \param s the Element.
 \param tol the maximum error allowed.
 \relates Piecewise
If you only want the total length, this routine faster and more accurate than constructing an arcLengthSb.
*/
double
Geom::length(Piecewise<D2<SBasis> > const &s,
                 double tol){
    double result = 0;
    double abs_error = 0;
    for (unsigned i=0; i < s.size();i++){
        length_integrating(s[i], result, abs_error, tol);
    }
    return result;
}

/**
 * Centroid using sbasis integration.
 \param p the Element.
 \param centroid on return contains the centroid of the shape
 \param area on return contains the signed area of the shape.
 \relates Piecewise
This approach uses green's theorem to compute the area and centroid using integrals.  For curved shapes this is much faster than converting to polyline.  Note that without an uncross operation the output is not the absolute area.

 * Returned values: 
    0 for normal execution;
    2 if area is zero, meaning centroid is meaningless.

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

/**
 * Find cubics with prescribed curvatures at both ends.
 *
 *  this requires to solve a system of the form
 *
 * \f[
 *  \lambda_1 = a_0 \lambda_0^2 + c_0
 *  \lambda_0 = a_1 \lambda_1^2 + c_1
 * \f]
 *
 * which is a deg 4 equation in lambda 0.
 * Below are basic functions dedicated to solving this assuming a0 and a1 !=0.
 */

static OptInterval
find_bounds_for_lambda0(double aa0,double aa1,double cc0,double cc1,
    int insist_on_speeds_signs){

    double a0=aa0,a1=aa1,c0=cc0,c1=cc1;
    Interval result;
    bool flip = a1<0;
    if (a1<0){a1=-a1; c1=-c1;}
    if (a0<0){a0=-a0; c0=-c0;}
    double a = (a0<a1 ? a0 : a1);
    double c = (c0<c1 ? c0 : c1);
    double delta = 1-4*a*c;
    if ( delta < 0 )
        return OptInterval();//return empty interval
    double lambda_max = (1+std::sqrt(delta))/2/a;
    
    result = Interval(c,lambda_max);
    if (flip) 
        result *= -1;
    if (insist_on_speeds_signs == 1){
        if (result.max() < 0)//Caution: setMin with max<new min...
            return OptInterval();//return empty interval
        result.setMin(0);
    }
    result = Interval(result.min()-.1,result.max()+.1);//just in case all our approx. were exact...
    return result;
}

static 
std::vector<double>
solve_lambda0(double a0,double a1,double c0,double c1,
             int insist_on_speeds_signs){

    SBasis p(3, Linear());
    p[0] = Linear( a1*c0*c0+c1, a1*a0*(a0+ 2*c0) +a1*c0*c0 +c1 -1  );
    p[1] = Linear( -a1*a0*(a0+2*c0), -a1*a0*(3*a0+2*c0) );
    p[2] = Linear( a1*a0*a0 );

    OptInterval domain = find_bounds_for_lambda0(a0,a1,c0,c1,insist_on_speeds_signs);
    if ( !domain ) 
        return std::vector<double>();
    p = compose(p,Linear(domain->min(),domain->max()));
    std::vector<double>rts = roots(p);
    for (unsigned i=0; i<rts.size(); i++){
        rts[i] = domain->min() + rts[i] * domain->extent();
    }
    return rts;
}

/**
* \brief returns the cubics fitting direction and curvature of a given
* input curve at two points.
* 
* The input can be the 
*    value, speed, and acceleration
* or
*    value, speed, and cross(acceleration,speed) 
* of the original curve at the both ends.
* (the second is often technically usefull, as it avoids unnecessary division by |v|^2) 
* Recall that K=1/R=cross(acceleration,speed)/|speed|^3.
*
* Moreover, a 7-th argument 'insist_on_speed_signs' can be supplied to select solutions:  
* If insist_on_speed_signs == 1, only consider solutions where speeds at both ends are positively
* proportional to the given ones.
* If insist_on_speed_signs == 0, allow speeds to point in the opposite direction (both at the same time) 
* If insist_on_speed_signs == -1, allow speeds to point in both direction independantly. 
*
* \relates D2
*/
std::vector<D2<SBasis> >
Geom::cubics_fitting_curvature(Point const &M0,   Point const &M1,
                         Point const &dM0,  Point const &dM1,
                         double d2M0xdM0,  double d2M1xdM1,
                         int insist_on_speed_signs,
                         double epsilon){
    std::vector<D2<SBasis> > result;

    //speed of cubic bezier will be lambda0*dM0 and lambda1*dM1,
    //with lambda0 and lambda1 s.t. curvature at both ends is the same
    //as the curvature of the given curve.
    std::vector<double> lambda0,lambda1;
    double dM1xdM0=cross(dM1,dM0);
    if (fabs(dM1xdM0)<epsilon){
        if (fabs(d2M0xdM0)<epsilon || fabs(d2M1xdM1)<epsilon){
            return result;
        }
        double lbda02 = 6.*cross(M1-M0,dM0)/d2M0xdM0;
        double lbda12 =-6.*cross(M1-M0,dM1)/d2M1xdM1;
        if (lbda02<0 || lbda12<0){
            return result;
        }
        lambda0.push_back(std::sqrt(lbda02) );
        lambda1.push_back(std::sqrt(lbda12) );
    }else{
        //solve:  lambda1 = a0 lambda0^2 + c0
        //        lambda0 = a1 lambda1^2 + c1
        double a0,c0,a1,c1;
        a0 = -d2M0xdM0/2/dM1xdM0;
        c0 =  3*cross(M1-M0,dM0)/dM1xdM0;
        a1 = -d2M1xdM1/2/dM1xdM0;
        c1 = -3*cross(M1-M0,dM1)/dM1xdM0;

        if (fabs(a0)<epsilon){
            lambda1.push_back( c0 );
            lambda0.push_back( a1*c0*c0 + c1 );
        }else if (fabs(a1)<epsilon){
            lambda0.push_back( c1 );
            lambda1.push_back( a0*c1*c1 + c0 );
        }else{
            //find lamda0 by solving a deg 4 equation d0+d1*X+...+d4*X^4=0
            vector<double> solns=solve_lambda0(a0,a1,c0,c1,insist_on_speed_signs);
            for (unsigned i=0;i<solns.size();i++){
                double lbda0=solns[i];
                double lbda1=c0+a0*lbda0*lbda0;
                //is this solution pointing in the + direction at both ends?
                if (lbda0>=0. && lbda1>=0.){
                    lambda0.push_back( lbda0);
                    lambda1.push_back( lbda1);
                }
                //is this solution pointing in the - direction at both ends?
                else if (lbda0<=0. && lbda1<=0. && insist_on_speed_signs<=0){
                    lambda0.push_back( lbda0);
                    lambda1.push_back( lbda1);
                }
                //ok,this solution is pointing in the + and - directions.
                else if (insist_on_speed_signs<0){
                    lambda0.push_back( lbda0);
                    lambda1.push_back( lbda1);
                }
            }
        }
    }
    
    for (unsigned i=0; i<lambda0.size(); i++){
        Point V0 = lambda0[i]*dM0;
        Point V1 = lambda1[i]*dM1;
        D2<SBasis> cubic;
        for(unsigned dim=0;dim<2;dim++){
            SBasis c(2, Linear());
            c[0] = Linear(M0[dim],M1[dim]);
            c[1] = Linear( M0[dim]-M1[dim]+V0[dim],
                           -M0[dim]+M1[dim]-V1[dim]);
            cubic[dim] = c;
        }
#if 0
           Piecewise<SBasis> k = curvature(result);
           double dM0_l = dM0.length();
           double dM1_l = dM1.length();
           g_warning("Target radii: %f, %f", dM0_l*dM0_l*dM0_l/d2M0xdM0,dM1_l*dM1_l*dM1_l/d2M1xdM1);
           g_warning("Obtained radii: %f, %f",1/k.valueAt(0),1/k.valueAt(1));
#endif
        result.push_back(cubic);
    }
    return(result);
}

std::vector<D2<SBasis> >
Geom::cubics_fitting_curvature(Point const &M0,   Point const &M1,
                         Point const &dM0,  Point const &dM1,
                         Point const &d2M0, Point const &d2M1,
                         int insist_on_speed_signs,
                         double epsilon){
    double d2M0xdM0 = cross(d2M0,dM0);
    double d2M1xdM1 = cross(d2M1,dM1);
    return cubics_fitting_curvature(M0,M1,dM0,dM1,d2M0xdM0,d2M1xdM1,insist_on_speed_signs,epsilon);
}

std::vector<D2<SBasis> >
Geom::cubics_with_prescribed_curvature(Point const &M0,   Point const &M1,
                                 Point const &dM0,  Point const &dM1,
                                 double k0, double k1,
                                 int insist_on_speed_signs,
                                 double epsilon){
    double length;
    length = dM0.length();
    double d2M0xdM0 = k0*length*length*length;
    length = dM1.length();
    double d2M1xdM1 = k1*length*length*length;
    return cubics_fitting_curvature(M0,M1,dM0,dM1,d2M0xdM0,d2M1xdM1,insist_on_speed_signs,epsilon);
}


namespace Geom {
/**
* \brief returns all the parameter values of A whose tangent passes through P.
* \relates D2
*/
std::vector<double> find_tangents(Point P, D2<SBasis> const &A) {
    SBasis crs (cross(A - P, derivative(A)));
    return roots(crs);
}

/**
* \brief returns all the parameter values of A whose normal passes through P.
* \relates D2
*/
std::vector<double> find_normals(Point P, D2<SBasis> const &A) {
    SBasis crs (dot(A - P, derivative(A)));
    return roots(crs);
}

/**
* \brief returns all the parameter values of A whose normal is parallel to vector V.
* \relates D2
*/
std::vector<double> find_normals_by_vector(Point V, D2<SBasis> const &A) {
    SBasis crs = dot(derivative(A), V);
    return roots(crs);
}
/**
* \brief returns all the parameter values of A whose tangent is parallel to vector V.
* \relates D2
*/
std::vector<double> find_tangents_by_vector(Point V, D2<SBasis> const &A) {
    SBasis crs = dot(derivative(A), rot90(V));
    return roots(crs);
}

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
