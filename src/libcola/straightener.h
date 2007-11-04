/*
** vim: set cindent 
** vim: ts=4 sw=4 et tw=0 wm=0
*/
#ifndef STRAIGHTENER_H
#define STRAIGHTENER_H
#include <set>
#include <libvpsc/generate-constraints.h>
#include "gradient_projection.h"
namespace straightener {
    struct Route {
        Route(unsigned n) : n(n), xs(new double[n]), ys(new double[n]) {}
        virtual ~Route() {
            delete [] xs;
            delete [] ys;
        }
        void boundingBox(double &xmin,double &ymin,double &xmax,double &ymax) {
            xmin=ymin=DBL_MAX;
            xmax=ymax=-DBL_MAX;
            for(unsigned i=0;i<n;i++) {
                xmin=min(xmin,xs[i]);
                xmax=max(xmax,xs[i]);
                ymin=min(ymin,ys[i]);
                ymax=max(ymax,ys[i]);
            } 
        }
        unsigned n;
        double *xs;
        double *ys;
    };
    class Node;
    struct Edge {
        unsigned id;
        unsigned openInd; // position in openEdges
        unsigned startNode, endNode;
        Route* route;
        double xmin, xmax, ymin, ymax;
        vector<unsigned> dummyNodes;
        vector<unsigned> path;
        Edge(unsigned id, unsigned start, unsigned end, Route* route)
        : id(id), startNode(start), endNode(end), route(route)
        {
            route->boundingBox(xmin,ymin,xmax,ymax);
        }
        virtual ~Edge() {
            delete route;
        }
        void setRoute(Route* r) {
            delete route;
            route=r;
            route->boundingBox(xmin,ymin,xmax,ymax);
        }
        bool isEnd(unsigned n) {
            if(startNode==n||endNode==n) return true;
            return false;
        }
        void nodePath(vector<Node*>& nodes);
        void createRouteFromPath(double* X, double* Y) {
            Route* r=new Route(path.size());
            for(unsigned i=0;i<path.size();i++) {
                r->xs[i]=X[path[i]];
                r->ys[i]=Y[path[i]];
            }
            setRoute(r);
        }
        void xpos(double y, vector<double>& xs) {
            // search line segments for intersection points with y pos
            for(unsigned i=1;i<route->n;i++) {
                double ax=route->xs[i-1], bx=route->xs[i], ay=route->ys[i-1], by=route->ys[i];
                double r=(y-ay)/(by-ay);
                // as long as y is between ay and by then r>0
                if(r>0&&r<=1) {
                    xs.push_back(ax+(bx-ax)*r);
                }
            }
        }
        void ypos(double x, vector<double>& ys) {
            // search line segments for intersection points with x pos
            for(unsigned i=1;i<route->n;i++) {
                double ax=route->xs[i-1], bx=route->xs[i], ay=route->ys[i-1], by=route->ys[i];
                double r=(x-ax)/(bx-ax);
                // as long as y is between ax and bx then r>0
                if(r>0&&r<=1) {
                    ys.push_back(ay+(by-ay)*r);
                }
            }
        }
    };
    class Node {
    public:
        unsigned id;
        double x,y;
        double scanpos;
        double width, height;
        double xmin, xmax, ymin, ymax;
        Edge *edge;
        bool dummy;
        double weight;
        bool open;
        Node(unsigned id, vpsc::Rectangle* r) :
            id(id),x(r->getCentreX()),y(r->getCentreY()), width(r->width()), height(r->height()),
            xmin(x-width/2),xmax(x+width/2),
            ymin(y-height/2),ymax(y+height/2),
            edge(NULL),dummy(false),weight(-0.1),open(false) { }
    private:
        friend void sortNeighbours(Node* v, Node* l, Node* r, 
            double conjpos, vector<Edge*>& openEdges, 
            vector<Node*>& L,vector<Node*>& nodes, Dim dim);
        Node(unsigned id, double x, double y, Edge* e) : 
            id(id),x(x),y(y), width(4), height(width),
            xmin(x-width/2),xmax(x+width/2),
            ymin(y-height/2),ymax(y+height/2),
            edge(e),dummy(true),weight(-0.1)  {
                e->dummyNodes.push_back(id);
            }
    };
    struct CmpNodePos {
        bool operator() (const Node* u, const Node* v) const {
            if (u->scanpos < v->scanpos) {
                return true;
            }
            if (v->scanpos < u->scanpos) {
                return false;
            }
            return u < v;
        }
    };
    typedef std::set<Node*,CmpNodePos> NodeSet;
    void generateConstraints(vector<Node*>& nodes, vector<Edge*>& edges,vector<SimpleConstraint*>& cs, Dim dim);
    void nodePath(Edge& e,vector<Node*>& nodes, vector<unsigned>& path);
}

#endif
