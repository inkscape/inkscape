/*
 *  PathSimplify.cpp
 *  nlivarot
 *
 *  Created by fred on Fri Dec 12 2003.
 *
 */

#include <glib.h>
#include <2geom/affine.h>
#include "livarot/Path.h"
#include "livarot/path-description.h"

/*
 * Reassembling polyline segments into cubic bezier patches
 * thes functions do not need the back data. but they are slower than recomposing
 * path descriptions when you have said back data (it's always easier with a model)
 * there's a bezier fitter in bezier-utils.cpp too. the main difference is the way bezier patch are split
 * here: walk on the polyline, trying to extend the portion you can fit by respecting the treshhold, split when 
 * treshhold is exceeded. when encountering a "forced" point, lower the treshhold to favor splitting at that point
 * in bezier-utils: fit the whole polyline, get the position with the higher deviation to the fitted curve, split
 * there and recurse
 */


// algo d'origine: http://www.cs.mtu.edu/~shene/COURSES/cs3621/NOTES/INT-APP/CURVE-APP-global.html

// need the b-spline basis for cubic splines
// pas oublier que c'est une b-spline clampee
// et que ca correspond a une courbe de bezier normale
#define N03(t) ((1.0-t)*(1.0-t)*(1.0-t))
#define N13(t) (3*t*(1.0-t)*(1.0-t))
#define N23(t) (3*t*t*(1.0-t))
#define N33(t) (t*t*t)
// quadratic b-splines (jsut in case)
#define N02(t) ((1.0-t)*(1.0-t))
#define N12(t) (2*t*(1.0-t))
#define N22(t) (t*t)
// linear interpolation b-splines
#define N01(t) ((1.0-t))
#define N11(t) (t)



void Path::Simplify(double treshhold)
{
    if (pts.size() <= 1) {
        return;
    }
    
    Reset();
  
    int lastM = 0;
    while (lastM < int(pts.size())) {
        int lastP = lastM + 1;
        while (lastP < int(pts.size())
               && (pts[lastP].isMoveTo == polyline_lineto
                   || pts[lastP].isMoveTo == polyline_forced))
        {
            lastP++;
        }
        
        DoSimplify(lastM, lastP - lastM, treshhold);

        lastM = lastP;
    }
}


#if 0
// dichomtomic method to get distance to curve approximation
// a real polynomial solver would get the minimum more efficiently, but since the polynom
// would likely be of degree >= 5, that would imply using some generic solver, liek using the sturm metod
static double RecDistanceToCubic(Geom::Point const &iS, Geom::Point const &isD,
                                 Geom::Point const &iE, Geom::Point const &ieD,
                                 Geom::Point &pt, double current, int lev, double st, double et)
{
    if ( lev <= 0 ) {
        return current;
    }
	
    Geom::Point const m = 0.5 * (iS + iE) + 0.125 * (isD - ieD);
    Geom::Point const md = 0.75 * (iE - iS) - 0.125 * (isD + ieD);
    double const mt = (st + et) / 2;
	
    Geom::Point const hisD = 0.5 * isD;
    Geom::Point const hieD = 0.5 * ieD;
	
    Geom::Point const mp = pt - m;
    double nle = Geom::dot(mp, mp);
    
    if ( nle < current ) {

        current = nle;
        nle = RecDistanceToCubic(iS, hisD, m, md, pt, current, lev - 1, st, mt);
        if ( nle < current ) {
            current = nle;
        }
        nle = RecDistanceToCubic(m, md, iE, hieD, pt, current, lev - 1, mt, et);
        if ( nle < current ) {
            current = nle;
        }
        
    } else if ( nle < 2 * current ) {

        nle = RecDistanceToCubic(iS, hisD, m, md, pt, current, lev - 1, st, mt);
        if ( nle < current ) {
            current = nle;
        }
        nle = RecDistanceToCubic(m, md, iE, hieD, pt, current, lev - 1, mt, et);
        if ( nle < current ) {
            current = nle;
        }
    }
    
    return current;
}
#endif

static double DistanceToCubic(Geom::Point const &start, PathDescrCubicTo res, Geom::Point &pt)
{
    Geom::Point const sp = pt - start;
    Geom::Point const ep = pt - res.p;
    double nle = Geom::dot(sp, sp);
    double nnle = Geom::dot(ep, ep);
    if ( nnle < nle ) {
        nle = nnle;
    }
    
    Geom::Point seg = res.p - start;
    nnle = Geom::cross(sp, seg);
    nnle *= nnle;
    nnle /= Geom::dot(seg, seg);
    if ( nnle < nle ) {
        if ( Geom::dot(sp,seg) >= 0 ) {
            seg = start - res.p;
            if ( Geom::dot(ep,seg) >= 0 ) {
                nle = nnle;
            }
        }
    }
    
    return nle;
}


/**
 *    Simplification on a subpath.
 */

void Path::DoSimplify(int off, int N, double treshhold)
{
  // non-dichotomic method: grow an interval of points approximated by a curve, until you reach the treshhold, and repeat
    if (N <= 1) {
        return;
    }
    
    int curP = 0;
  
    fitting_tables data;
    data.Xk = data.Yk = data.Qk = NULL;
    data.tk = data.lk = NULL;
    data.fk = NULL;
    data.totLen = 0;
    data.nbPt = data.maxPt = data.inPt = 0;
  
    Geom::Point const moveToPt = pts[off].p;
    MoveTo(moveToPt);
    Geom::Point endToPt = moveToPt;
  
    while (curP < N - 1) {

        int lastP = curP + 1;
        int M = 2;

        // remettre a zero
        data.inPt = data.nbPt = 0;

        PathDescrCubicTo res(Geom::Point(0, 0), Geom::Point(0, 0), Geom::Point(0, 0));
        bool contains_forced = false;
        int step = 64;
        
        while ( step > 0 ) {   
            int forced_pt = -1;
            int worstP = -1;
            
            do {
                if (pts[off + lastP].isMoveTo == polyline_forced) {
                    contains_forced = true;
                }
                forced_pt = lastP;
                lastP += step;
                M += step;
            } while (lastP < N && ExtendFit(off + curP, M, data,
                                            (contains_forced) ? 0.05 * treshhold : treshhold,
                                            res, worstP) );
            if (lastP >= N) {

                lastP -= step;
                M -= step;
                
            } else {
                // le dernier a echoue
                lastP -= step;
                M -= step;
                
                if ( contains_forced ) {
                    lastP = forced_pt;
                    M = lastP - curP + 1;
                }

                AttemptSimplify(off + curP, M, treshhold, res, worstP);       // ca passe forcement
            }
            step /= 2;
        }
    
        endToPt = pts[off + lastP].p;
        if (M <= 2) {
            LineTo(endToPt);
        } else {
            CubicTo(endToPt, res.start, res.end);
        }
        
        curP = lastP;
    }
  
    if (Geom::LInfty(endToPt - moveToPt) < 0.00001) {
        Close();
    }
  
    g_free(data.Xk);
    g_free(data.Yk);
    g_free(data.Qk);
    g_free(data.tk);
    g_free(data.lk);
    g_free(data.fk);
}


// warning: slow
// idea behing this feature: splotches appear when trying to fit a small number of points: you can
// get a cubic bezier that fits the points very well but doesn't fit the polyline itself
// so we add a bit of the error at the middle of each segment of the polyline
// also we restrict this to <=20 points, to avoid unnecessary computations
#define with_splotch_killer

// primitive= calc the cubic bezier patche that fits Xk and Yk best
// Qk est deja alloue
// retourne false si probleme (matrice non-inversible)
bool Path::FitCubic(Geom::Point const &start, PathDescrCubicTo &res,
                    double *Xk, double *Yk, double *Qk, double *tk, int nbPt)
{
    Geom::Point const end = res.p;
    
    // la matrice tNN
    Geom::Affine M(0, 0, 0, 0, 0, 0);
    for (int i = 1; i < nbPt - 1; i++) {
        M[0] += N13(tk[i]) * N13(tk[i]);
        M[1] += N23(tk[i]) * N13(tk[i]);
        M[2] += N13(tk[i]) * N23(tk[i]);
        M[3] += N23(tk[i]) * N23(tk[i]);
    }
  
    double const det = M.det();
    if (fabs(det) < 0.000001) {
        res.start[0]=res.start[1]=0.0;
        res.end[0]=res.end[1]=0.0;
        return false;
    }
    
    Geom::Affine const iM = M.inverse();
    M = iM;
  
    // phase 1: abcisses
    // calcul des Qk
    Xk[0] = start[0];
    Yk[0] = start[1];
    Xk[nbPt - 1] = end[0];
    Yk[nbPt - 1] = end[1];
  
    for (int i = 1; i < nbPt - 1; i++) {
        Qk[i] = Xk[i] - N03 (tk[i]) * Xk[0] - N33 (tk[i]) * Xk[nbPt - 1];
    }
  
    // le vecteur Q
    Geom::Point Q(0, 0);
    for (int i = 1; i < nbPt - 1; i++) {
        Q[0] += N13 (tk[i]) * Qk[i];
        Q[1] += N23 (tk[i]) * Qk[i];
    }
  
    Geom::Point P = Q * M;
    Geom::Point cp1;
    Geom::Point cp2;
    cp1[Geom::X] = P[Geom::X];
    cp2[Geom::X] = P[Geom::Y];
  
    // phase 2: les ordonnees
    for (int i = 1; i < nbPt - 1; i++) {
        Qk[i] = Yk[i] - N03 (tk[i]) * Yk[0] - N33 (tk[i]) * Yk[nbPt - 1];
    }
  
    // le vecteur Q
    Q = Geom::Point(0, 0);
    for (int i = 1; i < nbPt - 1; i++) {
        Q[0] += N13 (tk[i]) * Qk[i];
        Q[1] += N23 (tk[i]) * Qk[i];
    }
  
    P = Q * M;
    cp1[Geom::Y] = P[Geom::X];
    cp2[Geom::Y] = P[Geom::Y];
  
    res.start = 3.0 * (cp1 - start);
    res.end = 3.0 * (end - cp2 );

    return true;
}


bool Path::ExtendFit(int off, int N, fitting_tables &data, double treshhold, PathDescrCubicTo &res, int &worstP)
{
    if ( N >= data.maxPt ) {
        data.maxPt = 2 * N + 1;
        data.Xk = (double *) g_realloc(data.Xk, data.maxPt * sizeof(double));
        data.Yk = (double *) g_realloc(data.Yk, data.maxPt * sizeof(double));
        data.Qk = (double *) g_realloc(data.Qk, data.maxPt * sizeof(double));
        data.tk = (double *) g_realloc(data.tk, data.maxPt * sizeof(double));
        data.lk = (double *) g_realloc(data.lk, data.maxPt * sizeof(double));
        data.fk = (char *) g_realloc(data.fk, data.maxPt * sizeof(char));
    }
    
    if ( N > data.inPt ) {
        for (int i = data.inPt; i < N; i++) {
            data.Xk[i] = pts[off + i].p[Geom::X];
            data.Yk[i] = pts[off + i].p[Geom::Y];
            data.fk[i] = ( pts[off + i].isMoveTo == polyline_forced ) ? 0x01 : 0x00;        
        }
        data.lk[0] = 0;
        data.tk[0] = 0;
        
        double prevLen = 0;
        for (int i = 0; i < data.inPt; i++) {
            prevLen += data.lk[i];
        }
        data.totLen = prevLen;
        
        for (int i = ( (data.inPt > 0) ? data.inPt : 1); i < N; i++) {
            Geom::Point diff;
            diff[Geom::X] = data.Xk[i] - data.Xk[i - 1];
            diff[Geom::Y] = data.Yk[i] - data.Yk[i - 1];
            data.lk[i] = Geom::L2(diff);
            data.totLen += data.lk[i];
            data.tk[i] = data.totLen;
        }
        
        for (int i = 0; i < data.inPt; i++) {
            data.tk[i] *= prevLen;
            data.tk[i] /= data.totLen;
        }
        
        for (int i = data.inPt; i < N; i++) {
            data.tk[i] /= data.totLen;
        }
        data.inPt = N;
    }
    
    if ( N < data.nbPt ) {
        // We've gone too far; we'll have to recalulate the .tk.
        data.totLen = 0;
        data.tk[0] = 0;
        data.lk[0] = 0;
        for (int i = 1; i < N; i++) {
            data.totLen += data.lk[i];
            data.tk[i] = data.totLen;
        }
        
        for (int i = 1; i < N; i++) {
            data.tk[i] /= data.totLen;
        }
    }
  
    data.nbPt = N;
    
    if ( data.nbPt <= 0 ) {
        return false;
    }
  
    res.p[0] = data.Xk[data.nbPt - 1];
    res.p[1] = data.Yk[data.nbPt - 1];
    res.start[0] = res.start[1] = 0;
    res.end[0] = res.end[1] = 0;
    worstP = 1;
    if ( N <= 2 ) {
        return true;
    }
  
    if ( data.totLen < 0.0001 ) {
        double worstD = 0;
        Geom::Point start;
        worstP = -1;
        start[0] = data.Xk[0];
        start[1] = data.Yk[0];
        for (int i = 1; i < N; i++) {
            Geom::Point nPt;
            bool isForced = data.fk[i];
            nPt[0] = data.Xk[i];
            nPt[1] = data.Yk[i];
      
            double nle = DistanceToCubic(start, res, nPt);
            if ( isForced ) {
                // forced points are favored for splitting the recursion; we do this by increasing their distance
                if ( worstP < 0 || 2*nle > worstD ) {
                    worstP = i;
                    worstD = 2*nle;
                }
            } else {
                if ( worstP < 0 || nle > worstD ) {
                    worstP = i;
                    worstD = nle;
                }
            }
        }
        
        return true;
    }
  
    return AttemptSimplify(data, treshhold, res, worstP);
}


// fit a polyline to a bezier patch, return true is treshhold not exceeded (ie: you can continue)
// version that uses tables from the previous iteration, to minimize amount of work done
bool Path::AttemptSimplify (fitting_tables &data,double treshhold, PathDescrCubicTo & res,int &worstP)
{
    Geom::Point start,end;
    // pour une coordonnee
    Geom::Point cp1, cp2;
  
    worstP = 1;
    if (pts.size() == 2) {
        return true;
    }
  
    start[0] = data.Xk[0];
    start[1] = data.Yk[0];
    cp1[0] = data.Xk[1];
    cp1[1] = data.Yk[1];
    end[0] = data.Xk[data.nbPt - 1];
    end[1] = data.Yk[data.nbPt - 1];
    cp2 = cp1;
  
    if (pts.size()  == 3) {
        // start -> cp1 -> end
        res.start = cp1 - start;
        res.end = end - cp1;
        worstP = 1;
        return true;
    }
  
    if ( FitCubic(start, res, data.Xk, data.Yk, data.Qk, data.tk, data.nbPt) ) {
        cp1 = start + res.start / 3;
        cp2 = end - res.end / 3;
    } else {
        // aie, non-inversible
        double worstD = 0;
        worstP = -1;
        for (int i = 1; i < data.nbPt; i++) {
            Geom::Point nPt;
            nPt[Geom::X] = data.Xk[i];
            nPt[Geom::Y] = data.Yk[i];
            double nle = DistanceToCubic(start, res, nPt);
            if ( data.fk[i] ) {
                // forced points are favored for splitting the recursion; we do this by increasing their distance
                if ( worstP < 0 || 2 * nle > worstD ) {
                    worstP = i;
                    worstD = 2 * nle;
                }
            } else {
                if ( worstP < 0 || nle > worstD ) {
                    worstP = i;
                    worstD = nle;
                }
            }
        }
        return false;
    }
   
    // calcul du delta= pondere par les longueurs des segments
    double delta = 0;
    {
        double worstD = 0;
        worstP = -1;
        Geom::Point prevAppP;
        Geom::Point prevP;
        double prevDist;
        prevP[Geom::X] = data.Xk[0];
        prevP[Geom::Y] = data.Yk[0];
        prevAppP = prevP; // le premier seulement
        prevDist = 0;
#ifdef with_splotch_killer
        if ( data.nbPt <= 20 ) {
            for (int i = 1; i < data.nbPt - 1; i++) {
                Geom::Point curAppP;
                Geom::Point curP;
                double curDist;
                Geom::Point midAppP;
                Geom::Point midP;
                double midDist;
                
                curAppP[Geom::X] = N13(data.tk[i]) * cp1[Geom::X] +
                    N23(data.tk[i]) * cp2[Geom::X] +
                    N03(data.tk[i]) * data.Xk[0] +
                    N33(data.tk[i]) * data.Xk[data.nbPt - 1];
                
                curAppP[Geom::Y] = N13(data.tk[i]) * cp1[Geom::Y] +
                    N23(data.tk[i]) * cp2[Geom::Y] +
                    N03(data.tk[i]) * data.Yk[0] +
                    N33(data.tk[i]) * data.Yk[data.nbPt - 1];
                
                curP[Geom::X] = data.Xk[i];
                curP[Geom::Y] = data.Yk[i];
                double mtk = 0.5 * (data.tk[i] + data.tk[i - 1]);
                
                midAppP[Geom::X] = N13(mtk) * cp1[Geom::X] +
                    N23(mtk) * cp2[Geom::X] +
                    N03(mtk) * data.Xk[0] +
                    N33(mtk) * data.Xk[data.nbPt - 1];
                
                midAppP[Geom::Y] = N13(mtk) * cp1[Geom::Y] +
                    N23(mtk) * cp2[Geom::Y] +
                    N03(mtk) * data.Yk[0] +
                    N33(mtk) * data.Yk[data.nbPt - 1];
                
                midP = 0.5 * (curP + prevP);
        
                Geom::Point diff = curAppP - curP;
                curDist = dot(diff, diff);
                diff = midAppP - midP;
                midDist = dot(diff, diff);
        
                delta += 0.3333 * (curDist + prevDist + midDist) * data.lk[i];
                if ( curDist > worstD ) {
                    worstD = curDist;
                    worstP = i;
                } else if ( data.fk[i] && 2 * curDist > worstD ) {
                    worstD = 2*curDist;
                    worstP = i;
                }
                prevP = curP;
                prevAppP = curAppP;
                prevDist = curDist;
            }
            delta /= data.totLen;
            
        } else {
#endif
            for (int i = 1; i < data.nbPt - 1; i++) {
                Geom::Point curAppP;
                Geom::Point curP;
                double    curDist;
        
                curAppP[Geom::X] = N13(data.tk[i]) * cp1[Geom::X] +
                    N23(data.tk[i]) * cp2[Geom::X] +
                    N03(data.tk[i]) * data.Xk[0] +
                    N33(data.tk[i]) * data.Xk[data.nbPt - 1];
                
                curAppP[Geom::Y] = N13(data.tk[i]) * cp1[Geom::Y] +
                    N23(data.tk[i]) * cp2[Geom::Y] +
                    N03(data.tk[i]) * data.Yk[0] +
                    N33(data.tk[i]) * data.Yk[data.nbPt - 1];
                
                curP[Geom::X] = data.Xk[i];
                curP[Geom::Y] = data.Yk[i];
      
                Geom::Point diff = curAppP-curP;
                curDist = dot(diff, diff);
                delta += curDist;
        
                if ( curDist > worstD ) {
                    worstD = curDist;
                    worstP = i;
                } else if ( data.fk[i] && 2 * curDist > worstD ) {
                    worstD = 2*curDist;
                    worstP = i;
                }
                prevP = curP;
                prevAppP = curAppP;
                prevDist = curDist;
            }
#ifdef with_splotch_killer
        }
#endif
    }
  
    if (delta < treshhold * treshhold) {
        // premier jet
    
        // Refine a little.
        for (int i = 1; i < data.nbPt - 1; i++) {
            Geom::Point pt(data.Xk[i], data.Yk[i]);
            data.tk[i] = RaffineTk(pt, start, cp1, cp2, end, data.tk[i]);
            if (data.tk[i] < data.tk[i - 1]) {
                // Force tk to be monotonic non-decreasing.
                data.tk[i] = data.tk[i - 1];
	    }
        }
    
        if ( FitCubic(start, res, data.Xk, data.Yk, data.Qk, data.tk, data.nbPt) == false) {
            // ca devrait jamais arriver, mais bon
            res.start = 3.0 * (cp1 - start);
            res.end = 3.0 * (end - cp2 );
            return true;
        }
        
        double ndelta = 0;
        {
            double worstD = 0;
            worstP = -1;
            Geom::Point prevAppP;
            Geom::Point prevP(data.Xk[0], data.Yk[0]);
            double prevDist = 0;
            prevAppP = prevP; // le premier seulement
#ifdef with_splotch_killer
            if ( data.nbPt <= 20 ) {
                for (int i = 1; i < data.nbPt - 1; i++) {
                    Geom::Point curAppP;
                    Geom::Point curP;
                    double  curDist;
                    Geom::Point midAppP;
                    Geom::Point midP;
                    double  midDist;
          
                    curAppP[Geom::X] = N13(data.tk[i]) * cp1[Geom::X] +
                        N23(data.tk[i]) * cp2[Geom::X] +
                        N03(data.tk[i]) * data.Xk[0] +
                        N33(data.tk[i]) * data.Xk[data.nbPt - 1];
                    
                    curAppP[Geom::Y] = N13(data.tk[i]) * cp1[Geom::Y] +
                        N23(data.tk[i]) * cp2[Geom::Y] +
                        N03(data.tk[i]) * data.Yk[0] +
                        N33(data.tk[i]) * data.Yk[data.nbPt - 1];
                    
                    curP[Geom::X] = data.Xk[i];
                    curP[Geom::Y] = data.Yk[i];
                    double mtk = 0.5 * (data.tk[i] + data.tk[i - 1]);
                    
                    midAppP[Geom::X] = N13(mtk) * cp1[Geom::X] +
                        N23(mtk) * cp2[Geom::X] +
                        N03(mtk) * data.Xk[0] +
                        N33(mtk) * data.Xk[data.nbPt - 1];
                    
                    midAppP[Geom::Y] = N13(mtk) * cp1[Geom::Y] +
                        N23(mtk) * cp2[Geom::Y] +
                        N03(mtk) * data.Yk[0] +
                        N33(mtk) * data.Yk[data.nbPt - 1];
                    
                    midP = 0.5 * (curP + prevP);
          
                    Geom::Point diff = curAppP - curP;
                    curDist = dot(diff, diff);
          
                    diff = midAppP - midP;
                    midDist = dot(diff, diff);
          
                    ndelta += 0.3333 * (curDist + prevDist + midDist) * data.lk[i];
          
                    if ( curDist > worstD ) {
                        worstD = curDist;
                        worstP = i;
                    } else if ( data.fk[i] && 2 * curDist > worstD ) {
                        worstD = 2*curDist;
                        worstP = i;
                    }
                    
                    prevP = curP;
                    prevAppP = curAppP;
                    prevDist = curDist;
                }
                ndelta /= data.totLen;
            } else {
#endif
                for (int i = 1; i < data.nbPt - 1; i++) {
                    Geom::Point curAppP;
                    Geom::Point curP;
                    double    curDist;
                    
                    curAppP[Geom::X] = N13(data.tk[i]) * cp1[Geom::X] +
                        N23(data.tk[i]) * cp2[Geom::X] +
                        N03(data.tk[i]) * data.Xk[0] +
                        N33(data.tk[i]) * data.Xk[data.nbPt - 1];
                    
                    curAppP[Geom::Y] = N13(data.tk[i]) * cp1[Geom::Y] +
                        N23(data.tk[i]) * cp2[1] +
                        N03(data.tk[i]) * data.Yk[0] +
                        N33(data.tk[i]) * data.Yk[data.nbPt - 1];
                    
                    curP[Geom::X] = data.Xk[i];
                    curP[Geom::Y] = data.Yk[i];
        
                    Geom::Point diff = curAppP - curP;
                    curDist = dot(diff, diff);

                    ndelta += curDist;

                    if ( curDist > worstD ) {
                        worstD = curDist;
                        worstP = i;
                    } else if ( data.fk[i] && 2 * curDist > worstD ) {
                        worstD = 2 * curDist;
                        worstP = i;
                    }
                    prevP = curP;
                    prevAppP = curAppP;
                    prevDist = curDist;
                }
#ifdef with_splotch_killer
            }
#endif
        }
    
        if (ndelta < delta + 0.00001) {
            return true;
        } else {
            // nothing better to do
            res.start = 3.0 * (cp1 - start);
            res.end = 3.0 * (end - cp2 );
        }
        
        return true;
    }
  
  return false;
}


bool Path::AttemptSimplify(int off, int N, double treshhold, PathDescrCubicTo &res,int &worstP)
{
    Geom::Point start;
    Geom::Point end;
    
    // pour une coordonnee
    double *Xk;				// la coordonnee traitee (x puis y)
    double *Yk;				// la coordonnee traitee (x puis y)
    double *lk;				// les longueurs de chaque segment
    double *tk;				// les tk
    double *Qk;				// les Qk
    char *fk;       // si point force
  
    Geom::Point cp1;
    Geom::Point cp2;
  
    if (N == 2) {
        worstP = 1;
        return true;
    }
  
    start = pts[off].p;
    cp1 = pts[off + 1].p;
    end = pts[off + N - 1].p;
  
    res.p = end;
    res.start[0] = res.start[1] = 0;
    res.end[0] = res.end[1] = 0;
    if (N == 3) {
        // start -> cp1 -> end
        res.start = cp1 - start;
        res.end = end - cp1;
        worstP = 1;
        return true;
    }
  
    // Totally inefficient, allocates & deallocates all the time.
    tk = (double *) g_malloc(N * sizeof(double));
    Qk = (double *) g_malloc(N * sizeof(double));
    Xk = (double *) g_malloc(N * sizeof(double));
    Yk = (double *) g_malloc(N * sizeof(double));
    lk = (double *) g_malloc(N * sizeof(double));
    fk = (char *) g_malloc(N * sizeof(char));
  
    // chord length method
    tk[0] = 0.0;
    lk[0] = 0.0;
    {
        Geom::Point prevP = start;
        for (int i = 1; i < N; i++) {
            Xk[i] = pts[off + i].p[Geom::X];
            Yk[i] = pts[off + i].p[Geom::Y];
            
            if ( pts[off + i].isMoveTo == polyline_forced ) {
                fk[i] = 0x01;
            } else {
                fk[i] = 0;
            }
            
            Geom::Point diff(Xk[i] - prevP[Geom::X], Yk[i] - prevP[1]);
            prevP[0] = Xk[i];
            prevP[1] = Yk[i];
            lk[i] = Geom::L2(diff);
            tk[i] = tk[i - 1] + lk[i];
        }
    }
    
    if (tk[N - 1] < 0.00001) {
        // longueur nulle 
        res.start[0] = res.start[1] = 0;
        res.end[0] = res.end[1] = 0;
        double worstD = 0;
        worstP = -1;
        for (int i = 1; i < N; i++) {
            Geom::Point nPt;
            bool isForced = fk[i];
            nPt[0] = Xk[i];
            nPt[1] = Yk[i];
 
            double nle = DistanceToCubic(start, res, nPt);
            if ( isForced ) {
                // forced points are favored for splitting the recursion; we do this by increasing their distance
                if ( worstP < 0 || 2 * nle > worstD ) {
                    worstP = i;
                    worstD = 2 * nle;
                }
            } else {
                if ( worstP < 0 || nle > worstD ) {
                    worstP = i;
                    worstD = nle;
                }
            }
        }
        
        g_free(tk);
        g_free(Qk);
        g_free(Xk);
        g_free(Yk);
        g_free(fk);
        g_free(lk);
        
        return false;
    }
    
    double totLen = tk[N - 1];
    for (int i = 1; i < N - 1; i++) {
        tk[i] /= totLen;
    }
  
    res.p = end;
    if ( FitCubic(start, res, Xk, Yk, Qk, tk, N) ) {
        cp1 = start + res.start / 3;
        cp2 = end + res.end / 3;
    } else {
        // aie, non-inversible
        res.start[0] = res.start[1] = 0;
        res.end[0] = res.end[1] = 0;
        double worstD = 0;
        worstP = -1;
        for (int i = 1; i < N; i++) {
            Geom::Point nPt(Xk[i], Yk[i]);
            bool isForced = fk[i];
            double nle = DistanceToCubic(start, res, nPt);
            if ( isForced ) {
                // forced points are favored for splitting the recursion; we do this by increasing their distance
                if ( worstP < 0 || 2 * nle > worstD ) {
                    worstP = i;
                    worstD = 2 * nle;
                }
            } else {
                if ( worstP < 0 || nle > worstD ) {
                    worstP = i;
                    worstD = nle;
                }
            }
        }
        
        g_free(tk);
        g_free(Qk);
        g_free(Xk);
        g_free(Yk);
        g_free(fk);
        g_free(lk);
        return false;
    }
   
    // calcul du delta= pondere par les longueurs des segments
    double delta = 0;
    {
        double worstD = 0;
        worstP = -1;
        Geom::Point prevAppP;
    Geom::Point   prevP;
    double      prevDist;
    prevP[0] = Xk[0];
    prevP[1] = Yk[0];
    prevAppP = prevP; // le premier seulement
    prevDist = 0;
#ifdef with_splotch_killer
    if ( N <= 20 ) {
      for (int i = 1; i < N - 1; i++)
      {
        Geom::Point curAppP;
        Geom::Point curP;
        double    curDist;
        Geom::Point midAppP;
        Geom::Point midP;
        double    midDist;
        
        curAppP[0] = N13 (tk[i]) * cp1[0] + N23 (tk[i]) * cp2[0] + N03 (tk[i]) * Xk[0] + N33 (tk[i]) * Xk[N - 1];
        curAppP[1] = N13 (tk[i]) * cp1[1] + N23 (tk[i]) * cp2[1] + N03 (tk[i]) * Yk[0] + N33 (tk[i]) * Yk[N - 1];
        curP[0] = Xk[i];
        curP[1] = Yk[i];
        midAppP[0] = N13 (0.5*(tk[i]+tk[i-1])) * cp1[0] + N23 (0.5*(tk[i]+tk[i-1])) * cp2[0] + N03 (0.5*(tk[i]+tk[i-1])) * Xk[0] + N33 (0.5*(tk[i]+tk[i-1])) * Xk[N - 1];
        midAppP[1] = N13 (0.5*(tk[i]+tk[i-1])) * cp1[1] + N23 (0.5*(tk[i]+tk[i-1])) * cp2[1] + N03 (0.5*(tk[i]+tk[i-1])) * Yk[0] + N33 (0.5*(tk[i]+tk[i-1])) * Yk[N - 1];
        midP=0.5*(curP+prevP);
        
        Geom::Point diff;
        diff = curAppP-curP;
        curDist = dot(diff,diff);

        diff = midAppP-midP;
        midDist = dot(diff,diff);
        
        delta+=0.3333*(curDist+prevDist+midDist)/**lk[i]*/;

        if ( curDist > worstD ) {
          worstD = curDist;
          worstP = i;
        } else if ( fk[i] && 2*curDist > worstD ) {
          worstD = 2*curDist;
          worstP = i;
        }
        prevP = curP;
        prevAppP = curAppP;
        prevDist = curDist;
      }
      delta/=totLen;
    } else {
#endif
      for (int i = 1; i < N - 1; i++)
      {
        Geom::Point curAppP;
        Geom::Point curP;
        double    curDist;
        
        curAppP[0] = N13 (tk[i]) * cp1[0] + N23 (tk[i]) * cp2[0] + N03 (tk[i]) * Xk[0] + N33 (tk[i]) * Xk[N - 1];
        curAppP[1] = N13 (tk[i]) * cp1[1] + N23 (tk[i]) * cp2[1] + N03 (tk[i]) * Yk[0] + N33 (tk[i]) * Yk[N - 1];
        curP[0] = Xk[i];
        curP[1] = Yk[i];
        
        Geom::Point diff;
        diff = curAppP-curP;
        curDist = dot(diff,diff);
        delta += curDist;
        if ( curDist > worstD ) {
          worstD = curDist;
          worstP = i;
        } else if ( fk[i] && 2*curDist > worstD ) {
          worstD = 2*curDist;
          worstP = i;
        }
        prevP = curP;
        prevAppP = curAppP;
        prevDist = curDist;
      }
#ifdef with_splotch_killer
    }
#endif
  }
  
  if (delta < treshhold * treshhold)
  {
    // premier jet
    res.start = 3.0 * (cp1 - start);
    res.end = -3.0 * (cp2 - end);
    res.p = end;
    
    // Refine a little.
    for (int i = 1; i < N - 1; i++)
    {
      Geom::Point
	    pt;
      pt[0] = Xk[i];
      pt[1] = Yk[i];
      tk[i] = RaffineTk (pt, start, cp1, cp2, end, tk[i]);
      if (tk[i] < tk[i - 1])
	    {
	      // Force tk to be monotonic non-decreasing.
	      tk[i] = tk[i - 1];
	    }
    }
    
    if ( FitCubic(start,res,Xk,Yk,Qk,tk,N) ) {
    } else {
      // ca devrait jamais arriver, mais bon
      res.start = 3.0 * (cp1 - start);
      res.end = -3.0 * (cp2 - end);
      g_free(tk);
      g_free(Qk);
      g_free(Xk);
      g_free(Yk);
      g_free(fk);
      g_free(lk);
      return true;
    }
    double ndelta = 0;
    {
      double worstD = 0;
      worstP = -1;
      Geom::Point   prevAppP;
      Geom::Point   prevP;
      double      prevDist;
      prevP[0] = Xk[0];
      prevP[1] = Yk[0];
      prevAppP = prevP; // le premier seulement
      prevDist = 0;
#ifdef with_splotch_killer
      if ( N <= 20 ) {
        for (int i = 1; i < N - 1; i++)
        {
          Geom::Point curAppP;
          Geom::Point curP;
          double    curDist;
          Geom::Point midAppP;
          Geom::Point midP;
          double    midDist;
          
          curAppP[0] = N13 (tk[i]) * cp1[0] + N23 (tk[i]) * cp2[0] + N03 (tk[i]) * Xk[0] + N33 (tk[i]) * Xk[N - 1];
          curAppP[1] = N13 (tk[i]) * cp1[1] + N23 (tk[i]) * cp2[1] + N03 (tk[i]) * Yk[0] + N33 (tk[i]) * Yk[N - 1];
          curP[0] = Xk[i];
          curP[1] = Yk[i];
          midAppP[0] = N13 (0.5*(tk[i]+tk[i-1])) * cp1[0] + N23 (0.5*(tk[i]+tk[i-1])) * cp2[0] + N03 (0.5*(tk[i]+tk[i-1])) * Xk[0] + N33 (0.5*(tk[i]+tk[i-1])) * Xk[N - 1];
          midAppP[1] = N13 (0.5*(tk[i]+tk[i-1])) * cp1[1] + N23 (0.5*(tk[i]+tk[i-1])) * cp2[1] + N03 (0.5*(tk[i]+tk[i-1])) * Yk[0] + N33 (0.5*(tk[i]+tk[i-1])) * Yk[N - 1];
          midP = 0.5*(curP+prevP);
          
          Geom::Point diff;
          diff = curAppP-curP;
          curDist = dot(diff,diff);
          diff = midAppP-midP;
          midDist = dot(diff,diff);
          
          ndelta+=0.3333*(curDist+prevDist+midDist)/**lk[i]*/;

          if ( curDist > worstD ) {
            worstD = curDist;
            worstP = i;
          } else if ( fk[i] && 2*curDist > worstD ) {
            worstD = 2*curDist;
            worstP = i;
          }
          prevP = curP;
          prevAppP = curAppP;
          prevDist = curDist;
        }
        ndelta /= totLen;
      } else {
#endif
        for (int i = 1; i < N - 1; i++)
        {
          Geom::Point curAppP;
          Geom::Point curP;
          double    curDist;
          
          curAppP[0] = N13 (tk[i]) * cp1[0] + N23 (tk[i]) * cp2[0] + N03 (tk[i]) * Xk[0] + N33 (tk[i]) * Xk[N - 1];
          curAppP[1] = N13 (tk[i]) * cp1[1] + N23 (tk[i]) * cp2[1] + N03 (tk[i]) * Yk[0] + N33 (tk[i]) * Yk[N - 1];
          curP[0]=Xk[i];
          curP[1]=Yk[i];
          
          Geom::Point diff;
          diff=curAppP-curP;
          curDist=dot(diff,diff);
          ndelta+=curDist;

          if ( curDist > worstD ) {
            worstD=curDist;
            worstP=i;
          } else if ( fk[i] && 2*curDist > worstD ) {
            worstD=2*curDist;
            worstP=i;
          }
          prevP=curP;
          prevAppP=curAppP;
          prevDist=curDist;
        }
#ifdef with_splotch_killer
      }
#endif
    }
    
    g_free(tk);
    g_free(Qk);
    g_free(Xk);
    g_free(Yk);
    g_free(fk);
    g_free(lk);
    
    if (ndelta < delta + 0.00001)
    {
      return true;
    } else {
      // nothing better to do
      res.start = 3.0 * (cp1 - start);
      res.end = -3.0 * (cp2 - end);
    }
    return true;
  } else {    
    // nothing better to do
  }
  
  g_free(tk);
  g_free(Qk);
  g_free(Xk);
  g_free(Yk);
  g_free(fk);
  g_free(lk);
  return false;
}

double Path::RaffineTk (Geom::Point pt, Geom::Point p0, Geom::Point p1, Geom::Point p2, Geom::Point p3, double it)
{
    // Refinement of the tk values. 
    // Just one iteration of Newtow Raphson, given that we're approaching the curve anyway.
    // [fr: vu que de toute facon la courbe est approchC)e]
    double const Ax = pt[Geom::X] -
        p0[Geom::X] * N03(it) -
        p1[Geom::X] * N13(it) -
        p2[Geom::X] * N23(it) -
        p3[Geom::X] * N33(it);
    
    double const Bx = (p1[Geom::X] - p0[Geom::X]) * N02(it) +
        (p2[Geom::X] - p1[Geom::X]) * N12(it) +
        (p3[Geom::X] - p2[Geom::X]) * N22(it);
  
    double const Cx = (p0[Geom::X] - 2 * p1[Geom::X] + p2[Geom::X]) * N01(it) +
        (p3[Geom::X] - 2 * p2[Geom::X] + p1[Geom::X]) * N11(it);
    
    double const Ay =  pt[Geom::Y] -
        p0[Geom::Y] * N03(it) -
        p1[Geom::Y] * N13(it) -
        p2[Geom::Y] * N23(it) -
        p3[Geom::Y] * N33(it);
    
    double const By = (p1[Geom::Y] - p0[Geom::Y]) * N02(it) +
        (p2[Geom::Y] - p1[Geom::Y]) * N12(it) +
        (p3[Geom::Y] - p2[Geom::Y]) * N22(it);
    
    double const Cy = (p0[Geom::Y] - 2 * p1[Geom::Y] + p2[Geom::Y]) * N01(it) +
        (p3[Geom::Y] - 2 * p2[Geom::Y] + p1[Geom::Y]) * N11(it);
    
    double const dF = -6 * (Ax * Bx + Ay * By);
    double const ddF = 18 * (Bx * Bx + By * By) - 12 * (Ax * Cx + Ay * Cy);
    if (fabs (ddF) > 0.0000001) {
        return it - dF / ddF;
    }
    
    return it;
}

// Variation on the fitting theme: try to merge path commands into cubic bezier patches.
// The goal is to reduce the number of path commands, especially when operations on path produce
// lots of small path elements; ideally you could get rid of very small segments at reduced visual cost.
void Path::Coalesce(double tresh)
{
    if ( descr_flags & descr_adding_bezier ) {
        CancelBezier();
    }
    
    if ( descr_flags & descr_doing_subpath ) {
        CloseSubpath();
    }
    
    if (descr_cmd.size() <= 2) {
        return;
    }
  
    SetBackData(false);
    Path* tempDest = new Path();
    tempDest->SetBackData(false);
  
    ConvertEvenLines(0.25*tresh);
  
    int lastP = 0;
    int lastAP = -1;
    // As the elements are stored in a separate array, it's no longer worth optimizing
    // the rewriting in the same array.
    // [[comme les elements sont stockes dans un tableau a part, plus la peine d'optimiser
    // la réécriture dans la meme tableau]]
    
    int lastA = descr_cmd[0]->associated;
    int prevA = lastA;
    Geom::Point firstP;

    /* FIXME: the use of this variable probably causes a leak or two.
    ** It's a hack anyway, and probably only needs to be a type rather than
    ** a full PathDescr.
    */
    PathDescr *lastAddition = new PathDescrMoveTo(Geom::Point(0, 0));
    bool containsForced = false;
    PathDescrCubicTo pending_cubic(Geom::Point(0, 0), Geom::Point(0, 0), Geom::Point(0, 0));
  
    for (int curP = 0; curP < int(descr_cmd.size()); curP++) {
        int typ = descr_cmd[curP]->getType();
        int nextA = lastA;

        if (typ == descr_moveto) {

            if (lastAddition->flags != descr_moveto) {
                FlushPendingAddition(tempDest,lastAddition,pending_cubic,lastAP);
            }
            lastAddition = descr_cmd[curP];
            lastAP = curP;
            FlushPendingAddition(tempDest, lastAddition, pending_cubic, lastAP);
            // Added automatically (too bad about multiple moveto's).
            // [fr: (tant pis pour les moveto multiples)]
            containsForced = false;
      
            PathDescrMoveTo *nData = dynamic_cast<PathDescrMoveTo *>(descr_cmd[curP]);
            firstP = nData->p;
            lastA = descr_cmd[curP]->associated;
            prevA = lastA;
            lastP = curP;
            
        } else if (typ == descr_close) {
            nextA = descr_cmd[curP]->associated;
            if (lastAddition->flags != descr_moveto) {
        
                PathDescrCubicTo res(Geom::Point(0, 0), Geom::Point(0, 0), Geom::Point(0, 0));
                int worstP = -1;
                if (AttemptSimplify(lastA, nextA - lastA + 1, (containsForced) ? 0.05 * tresh : tresh, res, worstP)) {
                    lastAddition = new PathDescrCubicTo(Geom::Point(0, 0),
                                                          Geom::Point(0, 0),
                                                          Geom::Point(0, 0));
                    pending_cubic = res;
                    lastAP = -1;
                }

                FlushPendingAddition(tempDest, lastAddition, pending_cubic, lastAP);
                FlushPendingAddition(tempDest, descr_cmd[curP], pending_cubic, curP);
        
	    } else {
                FlushPendingAddition(tempDest,descr_cmd[curP],pending_cubic,curP);
	    }
            
            containsForced = false;
            lastAddition = new PathDescrMoveTo(Geom::Point(0, 0));
            prevA = lastA = nextA;
            lastP = curP;
            lastAP = curP;
            
        } else if (typ == descr_forced) {
            
            nextA = descr_cmd[curP]->associated;
            if (lastAddition->flags != descr_moveto) {
                
                PathDescrCubicTo res(Geom::Point(0, 0), Geom::Point(0, 0), Geom::Point(0, 0));
                int worstP = -1;
                if (AttemptSimplify(lastA, nextA - lastA + 1, 0.05 * tresh, res, worstP)) {
                    // plus sensible parce que point force
                    // ca passe
                    /* (Possible translation: More sensitive because contains a forced point.) */
                    containsForced = true;
                } else  {
                    // Force the addition.
                    FlushPendingAddition(tempDest, lastAddition, pending_cubic, lastAP);
                    lastAddition = new PathDescrMoveTo(Geom::Point(0, 0));
                    prevA = lastA = nextA;
                    lastP = curP;
                    lastAP = curP;
                    containsForced = false;
                }
	    }
            
        } else if (typ == descr_lineto || typ == descr_cubicto || typ == descr_arcto) {
            
            nextA = descr_cmd[curP]->associated;
            if (lastAddition->flags != descr_moveto) {
                
                PathDescrCubicTo res(Geom::Point(0, 0), Geom::Point(0, 0), Geom::Point(0, 0));
                int worstP = -1;
                if (AttemptSimplify(lastA, nextA - lastA + 1, tresh, res, worstP)) {
                    lastAddition = new PathDescrCubicTo(Geom::Point(0, 0),
                                                          Geom::Point(0, 0),
                                                          Geom::Point(0, 0));
                    pending_cubic = res;
                    lastAddition->associated = lastA;
                    lastP = curP;
                    lastAP = -1;
                }  else {
                    lastA = descr_cmd[lastP]->associated;	// pourrait etre surecrit par la ligne suivante
                        /* (possible translation: Could be overwritten by the next line.) */
                    FlushPendingAddition(tempDest, lastAddition, pending_cubic, lastAP);
                    lastAddition = descr_cmd[curP];
                    if ( typ == descr_cubicto ) {
                        pending_cubic = *(dynamic_cast<PathDescrCubicTo*>(descr_cmd[curP]));
                    }
                    lastAP = curP;
                    containsForced = false;
                }
        
	    } else {
                lastA = prevA /*descr_cmd[curP-1]->associated */ ;
                lastAddition = descr_cmd[curP];
                if ( typ == descr_cubicto ) {
                    pending_cubic = *(dynamic_cast<PathDescrCubicTo*>(descr_cmd[curP]));
                }
                lastAP = curP;
                containsForced = false;
	    }
            prevA = nextA;
            
        } else if (typ == descr_bezierto) {

            if (lastAddition->flags != descr_moveto) {
                FlushPendingAddition(tempDest, lastAddition, pending_cubic, lastAP);
                lastAddition = new PathDescrMoveTo(Geom::Point(0, 0));
	    }
            lastAP = -1;
            lastA = descr_cmd[curP]->associated;
            lastP = curP;
            PathDescrBezierTo *nBData = dynamic_cast<PathDescrBezierTo*>(descr_cmd[curP]);
            for (int i = 1; i <= nBData->nb; i++) {
                FlushPendingAddition(tempDest, descr_cmd[curP + i], pending_cubic, curP + i);
            }
            curP += nBData->nb;
            prevA = nextA;
            
        } else if (typ == descr_interm_bezier) {
            continue;
        } else {
            continue;
        }
    }
    
    if (lastAddition->flags != descr_moveto) {
        FlushPendingAddition(tempDest, lastAddition, pending_cubic, lastAP);
    }
  
    Copy(tempDest);
    delete tempDest;
}


void Path::FlushPendingAddition(Path *dest, PathDescr *lastAddition,
                                PathDescrCubicTo &lastCubic, int lastAP)
{
    switch (lastAddition->getType()) {

    case descr_moveto:
        if ( lastAP >= 0 ) {
            PathDescrMoveTo* nData = dynamic_cast<PathDescrMoveTo *>(descr_cmd[lastAP]);
            dest->MoveTo(nData->p);
        }
        break;
        
    case descr_close:
        dest->Close();
        break;

    case descr_cubicto:
        dest->CubicTo(lastCubic.p, lastCubic.start, lastCubic.end);
        break;

    case descr_lineto:
        if ( lastAP >= 0 ) {
            PathDescrLineTo *nData = dynamic_cast<PathDescrLineTo *>(descr_cmd[lastAP]);
            dest->LineTo(nData->p);
        }
        break;

    case descr_arcto:
        if ( lastAP >= 0 ) {
            PathDescrArcTo *nData = dynamic_cast<PathDescrArcTo *>(descr_cmd[lastAP]);
            dest->ArcTo(nData->p, nData->rx, nData->ry, nData->angle, nData->large, nData->clockwise);
        }
        break;

    case descr_bezierto:
        if ( lastAP >= 0 ) {
            PathDescrBezierTo *nData = dynamic_cast<PathDescrBezierTo *>(descr_cmd[lastAP]);
            dest->BezierTo(nData->p);
        }
        break;

    case descr_interm_bezier:
        if ( lastAP >= 0 ) {
            PathDescrIntermBezierTo *nData = dynamic_cast<PathDescrIntermBezierTo*>(descr_cmd[lastAP]);
            dest->IntermBezierTo(nData->p);
        }
        break;
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
