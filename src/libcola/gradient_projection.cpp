/**********************************************************
 *
 * Solve a quadratic function f(X) = X' A X + b X
 * subject to a set of separation constraints cs
 *
 * Tim Dwyer, 2006
 **********************************************************/

#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <float.h>
#include <cassert>
#include <libvpsc/solve_VPSC.h>
#include <libvpsc/variable.h>
#include <libvpsc/constraint.h>
#include "gradient_projection.h"
#include <iostream>
#include <2geom/math-utils.h>
#include "isinf.h"
#include <math.h>

using namespace std;
using namespace vpsc;
//#define CONMAJ_LOGGING 1

static void dumpVPSCException(char const *str, IncSolver* solver) {
    cerr<<str<<endl;
    unsigned m;
    Constraint** cs = solver->getConstraints(m);
    for(unsigned i=0;i<m;i++) {
        cerr << *cs[i] << endl;
    }
}
/*
 * Use gradient-projection to solve an instance of
 * the Variable Placement with Separation Constraints problem.
 * Uses sparse matrix techniques to handle pairs of dummy
 * vars.
 */
unsigned GradientProjection::solve(double * b) {
	unsigned i,j,counter;
	if(max_iterations==0) return 0;

	bool converged=false;

    IncSolver* solver=NULL;

    solver = setupVPSC();
    //cerr << "in gradient projection: n=" << n << endl;
    for (i=0;i<n;i++) {
        assert(!IS_NAN(place[i]));
        assert(!isinf(place[i]));
        vars[i]->desiredPosition=place[i];
    }
    try {
        solver->satisfy();
    } catch (char const *str) {
        dumpVPSCException(str,solver);
	}

    for (i=0;i<n;i++) {
        place[i]=vars[i]->position();
    }
    for (DummyVars::iterator it=dummy_vars.begin();it!=dummy_vars.end();++it){
        (*it)->updatePosition();
    }
    	
	for (counter=0; counter<max_iterations&&!converged; counter++) {
		converged=true;		
		// find steepest descent direction
        //  g = 2 ( b - Ax )
		for (i=0; i<n; i++) {
			old_place[i]=place[i];
			g[i] = b[i];
			for (j=0; j<n; j++) {
				g[i] -= A[i][j]*place[j];
			}
            g[i] *= 2.0;
		}		
        for (DummyVars::iterator it=dummy_vars.begin();it!=dummy_vars.end();++it){
            (*it)->computeDescentVector();
        }
        // compute step size: alpha = ( g' g ) / ( 2 g' A g )
        //   g terms for dummy vars cancel out so don't consider
		double numerator = 0, denominator = 0, r;
		for (i=0; i<n; i++) {
			numerator += g[i]*g[i];
			r=0;
			for (j=0; j<n; j++) {
				r += A[i][j]*g[j];
			}
			denominator -= 2.0 * r*g[i];
		}
		double alpha = numerator/denominator;

        // move to new unconstrained position
		for (i=0; i<n; i++) {
			place[i]-=alpha*g[i];
            assert(!IS_NAN(place[i]));
            assert(!isinf(place[i]));
            vars[i]->desiredPosition=place[i];
		}
        for (DummyVars::iterator it=dummy_vars.begin();it!=dummy_vars.end();++it){
            (*it)->steepestDescent(alpha);
        }

        //project to constraint boundary
        try {
            solver->satisfy();
        } catch (char const *str) {
            dumpVPSCException(str,solver);
        }
        for (i=0;i<n;i++) {
            place[i]=vars[i]->position();
        }
        for (DummyVars::iterator it=dummy_vars.begin();it!=dummy_vars.end();++it){
            (*it)->updatePosition();
        }
        // compute d, the vector from last pnt to projection pnt
		for (i=0; i<n; i++) {
			d[i]=place[i]-old_place[i];
		}	
		// now compute beta, optimal step size from last pnt to projection pnt
        //   beta = ( g' d ) / ( 2 d' A d )
		numerator = 0, denominator = 0;
		for (i=0; i<n; i++) {
			numerator += g[i] * d[i];
			r=0;
			for (j=0; j<n; j++) {
				r += A[i][j] * d[j];
			}
			denominator += 2.0 * r * d[i];
		}
        for (DummyVars::iterator it=dummy_vars.begin();it!=dummy_vars.end();++it){
            (*it)->betaCalc(numerator,denominator);
        }
		double beta = numerator/denominator;

        // beta > 1.0 takes us back outside the feasible region
        // beta < 0 clearly not useful and may happen due to numerical imp.
        if(beta>0&&beta<1.0) {
            for (i=0; i<n; i++) {
                place[i]=old_place[i]+beta*d[i];
            }
            for (DummyVars::iterator it=dummy_vars.begin();it!=dummy_vars.end();++it){
                (*it)->feasibleDescent(beta);
            }
        }
		double test=0;
		for (i=0; i<n; i++) {
			test += fabs(place[i]-old_place[i]);
		}
        for (DummyVars::iterator it=dummy_vars.begin();it!=dummy_vars.end();++it){
            test += (*it)->absoluteDisplacement();
        }
		if(test>tolerance) {
			converged=false;
		}
	}
    destroyVPSC(solver);
	return counter;
}
// Setup an instance of the Variable Placement with Separation Constraints
// for one iteration.
// Generate transient local constraints --- such as non-overlap constraints 
// --- that are only relevant to one iteration, and merge these with the
// global constraint list (including alignment constraints,
// dir-edge constraints, containment constraints, etc).
IncSolver* GradientProjection::setupVPSC() {
    Constraint **cs;
    //assert(lcs.size()==0);
    
    for(DummyVars::iterator dit=dummy_vars.begin();
            dit!=dummy_vars.end(); ++dit) {
        (*dit)->setupVPSC(vars,lcs);
    }
    Variable** vs = new Variable*[vars.size()];
    for(unsigned i=0;i<vars.size();i++) {
        vs[i]=vars[i];
    }
    if(nonOverlapConstraints) {
        Constraint** tmp_cs=NULL;
        unsigned m=0;
        if(k==HORIZONTAL) {
            Rectangle::setXBorder(0.0001);
            m=generateXConstraints(n,rs,vs,tmp_cs,true); 
            Rectangle::setXBorder(0);
        } else {
            m=generateYConstraints(n,rs,vs,tmp_cs); 
        }
        for(unsigned i=0;i<m;i++) {
            lcs.push_back(tmp_cs[i]);
        }
    }
    cs = new Constraint*[lcs.size() + gcs.size()];
    unsigned m = 0 ;
    for(vector<Constraint*>::iterator ci = lcs.begin();ci!=lcs.end();++ci) {
        cs[m++] = *ci;
    }
    for(vector<Constraint*>::iterator ci = gcs.begin();ci!=gcs.end();++ci) {
        cs[m++] = *ci;
    }
    return new IncSolver(vars.size(),vs,m,cs);
}
void GradientProjection::clearDummyVars() {
    for(DummyVars::iterator i=dummy_vars.begin();i!=dummy_vars.end();++i) {
        delete *i;
    }
    dummy_vars.clear();
}
void GradientProjection::destroyVPSC(IncSolver *vpsc) {
    if(acs) {
        for(AlignmentConstraints::iterator ac=acs->begin(); ac!=acs->end();++ac) {
            (*ac)->updatePosition();
        }
    }
    unsigned m,n;
    Constraint** cs = vpsc->getConstraints(m);
    const Variable* const* vs = vpsc->getVariables(n);
    delete vpsc;
    delete [] cs;
    delete [] vs;
    for(vector<Constraint*>::iterator i=lcs.begin();i!=lcs.end();++i) {
            delete *i;
    }
    lcs.clear();
    //cout << " Vars count = " << vars.size() << " Dummy vars cnt=" << dummy_vars.size() << endl;
    vars.resize(vars.size()-dummy_vars.size()*2);
    for(DummyVars::iterator i=dummy_vars.begin();i!=dummy_vars.end();++i) {
        DummyVarPair* p = *i;
        delete p->left;
        delete p->right;
    }
}

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
