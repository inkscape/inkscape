#ifndef COLA_H
#define COLA_H

#include <utility>
#include <iterator>
#include <vector>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <cassert>
#include "gradient_projection.h"
#include "straightener.h"


typedef std::vector<unsigned> Cluster;
typedef std::vector<Cluster*> Clusters;
namespace vpsc { class Rectangle; }

namespace cola {
    using vpsc::Rectangle;
    typedef std::pair<unsigned, unsigned> Edge;

    // a graph component with a list of node_ids giving indices for some larger list of nodes
    // for the nodes in this component, and a list of edges - node indices relative to this component
    class Component {
    public:
        std::vector<unsigned> node_ids;
        std::vector<Rectangle*> rects;
        std::vector<Edge> edges;
        SimpleConstraints scx, scy;
        virtual ~Component();
        void moveRectangles(double x, double y);
        Rectangle* getBoundingBox();
    };

    // for a graph of n nodes, return connected components
    void connectedComponents(
            const std::vector<Rectangle*> &rs,
            const std::vector<Edge> &es,
            const SimpleConstraints &scx,
            const SimpleConstraints &scy,
            std::vector<Component*> &components);

    // move the contents of each component so that the components do not
    // overlap.
    void separateComponents(const std::vector<Component*> &components);

    // defines references to three variables for which the goal function
    // will be altered to prefer points u-b-v are in a linear arrangement
    // such that b is placed at u+t(v-u).
    struct LinearConstraint {
        LinearConstraint(unsigned u, unsigned v, unsigned b, double w,
                double frac_ub, double frac_bv,
                double* X, double* Y)
            : u(u),v(v),b(b),w(w),frac_ub(frac_ub),frac_bv(frac_bv),
              tAtProjection(true)
        {
            assert(frac_ub<=1.0);
            assert(frac_bv<=1.0);
            assert(frac_ub>=0);
            assert(frac_bv>=0);
            if(tAtProjection) {
                double uvx = X[v] - X[u],
                       uvy = Y[v] - Y[u],
                       vbx = X[b] - X[u],
                       vby = Y[b] - Y[u];
                t = uvx * vbx + uvy * vby;
                t/= uvx * uvx + uvy * uvy;
                // p is the projection point of b on line uv
                //double px = scalarProj * uvx + X[u];
                //double py = scalarProj * uvy + Y[u];
                // take t=|up|/|uv|
            } else {
                double numerator=X[b]-X[u];
                double denominator=X[v]-X[u];
                if(fabs(denominator)<0.001) {
                    // if line is close to vertical then use Y coords to compute T
                    numerator=Y[b]-Y[u];
                    denominator=Y[v]-Y[u];
                }
                if(fabs(denominator)<0.0001) {
                    denominator=1;
                }
                t=numerator/denominator;
            }
            duu=(1-t)*(1-t);
            duv=t*(1-t);
            dub=t-1;
            dvv=t*t;
            dvb=-t;
            dbb=1;
             //printf("New LC: t=%f\n",t);
        }
        unsigned u;
        unsigned v;
        unsigned b;
        double w; // weight
        double t;
        // 2nd partial derivatives of the goal function
        //   (X[b] - (1-t) X[u] - t X[v])^2
        double duu;
        double duv;
        double dub;
        double dvv;
        double dvb;
        double dbb;
        // Length of each segment as a fraction of the total edge length
        double frac_ub;
        double frac_bv;
        bool tAtProjection;
    };

    typedef std::vector<LinearConstraint*> LinearConstraints;

class TestConvergence {
public:
    double old_stress;
    TestConvergence(const double& tolerance = 0.001, const unsigned maxiterations = 1000)
        : tolerance(tolerance),
          maxiterations(maxiterations) { reset(); }
    virtual ~TestConvergence() {}

    virtual bool operator()(double new_stress, double* /*X*/, double* /*Y*/) {
        //std::cout<<"iteration="<<iterations<<", new_stress="<<new_stress<<std::endl;
        if (old_stress == DBL_MAX) {
            old_stress = new_stress;
            if(++iterations>=maxiterations) {;
                return true;
            } else {
                return false;
            }
        }
        bool converged =
            fabs(new_stress - old_stress) / (new_stress + 1e-10) < tolerance
            || ++iterations > maxiterations;
        old_stress = new_stress;
        return converged;
    }
    void reset() {
        old_stress = DBL_MAX;
        iterations = 0;
    }
private:
    const double tolerance;
    const unsigned maxiterations;
    unsigned iterations;
};

static TestConvergence defaultTest(0.0001,100);
class ConstrainedMajorizationLayout {
public:
    ConstrainedMajorizationLayout(
        std::vector<Rectangle*>& rs,
        std::vector<Edge>& es,
        double* eweights,
        double idealLength,
        TestConvergence& done=defaultTest);

    void moveBoundingBoxes() {
        for(unsigned i=0;i<lapSize;i++) {
            boundingBoxes[i]->moveCentreX(X[i]);
            boundingBoxes[i]->moveCentreY(Y[i]);
        }
    }

        void setupConstraints(
                AlignmentConstraints* acsx, AlignmentConstraints* acsy,
                bool avoidOverlaps,
                PageBoundaryConstraints* pbcx = NULL,
                PageBoundaryConstraints* pbcy = NULL,
                SimpleConstraints* scx = NULL,
                SimpleConstraints* scy = NULL,
                Clusters* cs = NULL,
                std::vector<straightener::Edge*>* straightenEdges = NULL);

        void addLinearConstraints(LinearConstraints* linearConstraints);

        void setupDummyVars();

        virtual ~ConstrainedMajorizationLayout() {
            if(boundingBoxes) {
                delete [] boundingBoxes;
            }
            if(constrainedLayout) {
                delete gpX;
                delete gpY;
            }
            for(unsigned i=0;i<lapSize;i++) {
                delete [] lap2[i];
                delete [] Dij[i];
            }
            delete [] lap2;
            delete [] Dij;
            delete [] X;
            delete [] Y;
        }
    bool run();
        void straighten(std::vector<straightener::Edge*>&, Dim);
        bool avoidOverlaps;
        bool constrainedLayout;
    private:
        double euclidean_distance(unsigned i, unsigned j) {
            return sqrt(
                (X[i] - X[j]) * (X[i] - X[j]) +
                (Y[i] - Y[j]) * (Y[i] - Y[j]));
        }
        double compute_stress(double **Dij);
        void majlayout(double** Dij,GradientProjection* gp, double* coords);
        void majlayout(double** Dij,GradientProjection* gp, double* coords,
                double* b);
        unsigned n; // is lapSize + dummyVars
        unsigned lapSize; // lapSize is the number of variables for actual nodes
        double** lap2; // graph laplacian
        double** Q; // quadratic terms matrix used in computations
        double** Dij;
        double tol;
        TestConvergence& done;
        Rectangle** boundingBoxes;
        double *X, *Y;
        Clusters* clusters;
        double edge_length;
        LinearConstraints *linearConstraints;
        GradientProjection *gpX, *gpY;
        std::vector<straightener::Edge*>* straightenEdges;
};

}
#endif                          // COLA_H
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4
