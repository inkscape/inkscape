#ifndef _GRADIENT_PROJECTION_H
#define _GRADIENT_PROJECTION_H

#include <libvpsc/solve_VPSC.h>
#include <libvpsc/variable.h>
#include <libvpsc/constraint.h>
#include <libvpsc/generate-constraints.h>
#include <vector>
#include <iostream>
#include <math.h>

typedef std::vector<vpsc::Constraint*> Constraints;
typedef std::vector<vpsc::Variable*> Variables;
typedef std::vector<std::pair<unsigned, double> > OffsetList;

class SimpleConstraint {
public:
    SimpleConstraint(unsigned l, unsigned r, double g) 
        : left(l), right(r), gap(g)  {}
    unsigned left;
    unsigned right;
    double gap;
};
typedef std::vector<SimpleConstraint*> SimpleConstraints;
class AlignmentConstraint {
friend class GradientProjection;
public:
    AlignmentConstraint(double pos) :
        offsets(),
        guide(NULL),
        position(pos),
        variable(NULL)
        {}
    void updatePosition() {
        position = variable->position();
    }
    OffsetList offsets;
    void* guide;
    double position;
private:
    vpsc::Variable* variable;
};
typedef std::vector<AlignmentConstraint*> AlignmentConstraints;

class PageBoundaryConstraints {
public:
    PageBoundaryConstraints(double lm, double rm, double w)
        : leftMargin(lm), rightMargin(rm), weight(w) { }
    void createVarsAndConstraints(Variables &vs, Constraints &cs) {
        vpsc::Variable* vl, * vr;
        // create 2 dummy vars, based on the dimension we are in
        vs.push_back(vl=new vpsc::Variable(vs.size(), leftMargin, weight));
        vs.push_back(vr=new vpsc::Variable(vs.size(), rightMargin, weight));

        // for each of the "real" variables, create a constraint that puts that var
        // between our two new dummy vars, depending on the dimension.
        for(OffsetList::iterator o=offsets.begin(); o!=offsets.end(); ++o)  {
            cs.push_back(new vpsc::Constraint(vl, vs[o->first], o->second));
            cs.push_back(new vpsc::Constraint(vs[o->first], vr, o->second));
        }
    }
    OffsetList offsets;
private:
    double leftMargin;
    double rightMargin;
    double weight;
};

typedef std::vector<std::pair<unsigned, double> > CList;
/**
 * A DummyVarPair is a pair of variables with an ideal distance between them and which have no
 * other interaction with other variables apart from through constraints.  This means that
 * the entries in the laplacian matrix for dummy vars and other vars would be 0 - thus, sparse
 * matrix techniques can be used in laplacian operations.
 * The constraints are specified by a two lists of pairs of variable indexes and required separation.
 * The two lists are:
 *   leftof: variables to which left must be to the left of, 
 *   rightof: variables to which right must be to the right of. 
 */
class DummyVarPair {
public:
    DummyVarPair(double desiredDist) :
        leftof(),
        rightof(),
        place_l(0),
        place_r(0),
        dist(desiredDist),
        b(0),
        left(NULL),
        right(NULL),
        lap2(1.0/(desiredDist*desiredDist)),
        g(0),
        old_place_l(0),
        old_place_r(0)
        {}
    
    CList leftof; // variables to which left dummy var must be to the left of
    CList rightof; // variables to which right dummy var must be to the right of
    double place_l;
    double place_r;
    void computeLinearTerm(double euclideanDistance) {   
        if(euclideanDistance > 1e-30) {
            b = place_r - place_l;
            b /= euclideanDistance * dist;
        } else { b=0; }
    }
    double stress(double euclideanDistance) {
        double diff = dist - euclideanDistance;
        return diff*diff / (dist*dist);
    }
private:
friend class GradientProjection; 
    /**
     * Setup vars and constraints for an instance of the VPSC problem.
     * Adds generated vars and constraints to the argument vectors.
     */
    void setupVPSC(Variables &vars, Constraints &cs) {
        double weight=1;
        left = new vpsc::Variable(vars.size(),place_l,weight);
        vars.push_back(left);
        right = new vpsc::Variable(vars.size(),place_r,weight);
        vars.push_back(right);
        for(CList::iterator cit=leftof.begin();
                cit!=leftof.end(); ++cit) {
            vpsc::Variable* v = vars[(*cit).first];
            cs.push_back(new vpsc::Constraint(left,v,(*cit).second)); 
        }
        for(CList::iterator cit=rightof.begin();
                cit!=rightof.end(); ++cit) {
            vpsc::Variable* v = vars[(*cit).first];
            cs.push_back(new vpsc::Constraint(v,right,(*cit).second)); 
        }
    }
    /**
     * Extract the result of a VPSC solution to the variable positions
     */
    void updatePosition() {
        place_l=left->position();
        place_r=right->position();
    }
    /**
     * Compute the descent vector, also copying the current position to old_place for
     * future reference
     */
    void computeDescentVector() {
        old_place_l=place_l;
        old_place_r=place_r;
        g = 2.0 * ( b + lap2 * ( place_l - place_r ) );
    }
    /** 
     * move in the direction of steepest descent (based on g computed by computeGradient)
     * alpha is the step size.
     */
    void steepestDescent(double alpha) {
        place_l -= alpha*g;
        place_r += alpha*g;
        left->desiredPosition=place_l;
        right->desiredPosition=place_r;
    }
    /**
     * add dummy vars' contribution to numerator and denominator for 
     * beta step size calculation
     */
    void betaCalc(double &numerator, double &denominator) {
        double dl = place_l-old_place_l,
               dr = place_r-old_place_r,
               r = 2.0 * lap2 * ( dr - dl );
        numerator += g * ( dl - dr );
        denominator += r*dl - r * dr;
    }
    /**
     * move by stepsize beta from old_place to place
     */
    void feasibleDescent(double beta) {
        left->desiredPosition = place_l = old_place_l + beta*(place_l - old_place_l);
        right->desiredPosition = place_r = old_place_r + beta*(place_r - old_place_r);
    }
    double absoluteDisplacement() {
        return fabs(place_l - old_place_l) + fabs(place_r - old_place_r);
    }
    double dist; // ideal distance between vars
    double b; // linear coefficient in quad form for left (b_right = -b)
    vpsc::Variable* left; // Variables used in constraints
    vpsc::Variable* right;
    double lap2; // laplacian entry
    double g; // descent vec for quad form for left (g_right = -g)
    double old_place_l; // old_place is where the descent vec g was computed
    double old_place_r;
};
typedef std::vector<DummyVarPair*> DummyVars;

enum Dim { HORIZONTAL, VERTICAL };

class GradientProjection {
public:
    GradientProjection(
        const Dim k,
        unsigned n, 
        double** A,
        double* x,
        double tol,
        unsigned max_iterations,
        AlignmentConstraints* acs=NULL,
        bool nonOverlapConstraints=false,
        vpsc::Rectangle** rs=NULL,
        PageBoundaryConstraints *pbc = NULL,
        SimpleConstraints *sc = NULL)
            : k(k), n(n), A(A), place(x), rs(rs),
              nonOverlapConstraints(nonOverlapConstraints),
              tolerance(tol), acs(acs), max_iterations(max_iterations),
              g(new double[n]), d(new double[n]), old_place(new double[n]),
              constrained(false)
    {
        for(unsigned i=0;i<n;i++) {
            vars.push_back(new vpsc::Variable(i,1,1));
        }
        if(acs) {
            for(AlignmentConstraints::iterator iac=acs->begin();
                    iac!=acs->end();++iac) {
                AlignmentConstraint* ac=*iac;
                vpsc::Variable *v=ac->variable=new vpsc::Variable(vars.size(),ac->position,0.0001);
                vars.push_back(v);
                for(OffsetList::iterator o=ac->offsets.begin();
                        o!=ac->offsets.end();
                        ++o) {
                    gcs.push_back(new vpsc::Constraint(v,vars[o->first],o->second,true));
                }
            }
        }
        if (pbc)  {          
            pbc->createVarsAndConstraints(vars,gcs);
        }
        if (sc) {
            for(SimpleConstraints::iterator c=sc->begin(); c!=sc->end();++c) {
                gcs.push_back(new vpsc::Constraint(
                        vars[(*c)->left],vars[(*c)->right],(*c)->gap));
            }
        }
        if(!gcs.empty() || nonOverlapConstraints) {
            constrained=true;
        }
    }
    virtual ~GradientProjection() {
        delete [] g;
        delete [] d;
        delete [] old_place;
        for(Constraints::iterator i(gcs.begin()); i!=gcs.end(); ++i) {
            delete *i;
        }
        gcs.clear();
        for(unsigned i=0;i<vars.size();i++) {
            delete vars[i];
        }
    }
    void clearDummyVars();
    unsigned solve(double* b);
    DummyVars dummy_vars; // special vars that must be considered in Lapl.
private:
    vpsc::IncSolver* setupVPSC();
    void destroyVPSC(vpsc::IncSolver *vpsc);
    Dim k;
    unsigned n; // number of actual vars
    double** A; // Graph laplacian matrix
    double* place;
    Variables vars; // all variables
                          // computations
    Constraints gcs; /* global constraints - persist throughout all
                                iterations */
    Constraints lcs; /* local constraints - only for current iteration */
    vpsc::Rectangle** rs;
    bool nonOverlapConstraints;
    double tolerance;
    AlignmentConstraints* acs;
    unsigned max_iterations;
    double* g; /* gradient */
    double* d;
    double* old_place;
    bool constrained;
};

#endif /* _GRADIENT_PROJECTION_H */

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
