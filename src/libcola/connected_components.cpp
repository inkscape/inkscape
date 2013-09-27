#include <map>
#include <list>
#include <libvpsc/remove_rectangle_overlap.h>
#include "cola.h"
using namespace std;

namespace cola {
    Component::~Component() {
        for(unsigned i=0;i<scx.size();i++) {
            delete scx[i];
        }
        for(unsigned i=0;i<scy.size();i++) {
            delete scy[i];
        }
    }
    void Component::moveRectangles(double x, double y) {
        for(unsigned i=0;i<rects.size();i++) {
            rects[i]->moveCentreX(rects[i]->getCentreX()+x);
            rects[i]->moveCentreY(rects[i]->getCentreY()+y);
        }
    }
    Rectangle* Component::getBoundingBox() {
        double llx=DBL_MAX, lly=DBL_MAX, urx=-DBL_MAX, ury=-DBL_MAX;
        for(unsigned i=0;i<rects.size();i++) {
            llx=min(llx,rects[i]->getMinX());
            lly=min(lly,rects[i]->getMinY());
            urx=max(urx,rects[i]->getMaxX());
            ury=max(ury,rects[i]->getMaxY());
        }
        return new Rectangle(llx,urx,lly,ury);
    }

    namespace ccomponents {
        struct Node {
            unsigned id;
            bool visited;
            vector<Node*> neighbours;
            list<Node*>::iterator listPos;
            Rectangle* r;
        };
        // Depth first search traversal of graph to find connected component
        static void dfs(Node* v,
                list<Node*>& remaining,
                Component* component,
                map<unsigned,pair<Component*,unsigned> > &cmap) {
            v->visited=true;
            remaining.erase(v->listPos);
            cmap[v->id]=make_pair(component,component->node_ids.size());
            component->node_ids.push_back(v->id);
            component->rects.push_back(v->r);
            for(unsigned i=0;i<v->neighbours.size();i++) {
                Node* u=v->neighbours[i];
                if(!u->visited) {
                    dfs(u,remaining,component,cmap);
                }
            }
        }
    }

    using namespace ccomponents;

    // for a graph of n nodes, return connected components
    void connectedComponents(
            const vector<Rectangle*> &rs,
            const vector<Edge> &es, 
            const SimpleConstraints &scx,
            const SimpleConstraints &scy,
            vector<Component*> &components) {
        unsigned n=rs.size();
        vector<Node> vs(n);
        list<Node*> remaining;
        for(unsigned i=0;i<n;i++) {
            vs[i].id=i;
            vs[i].visited=false;
            vs[i].r=rs[i];
            vs[i].listPos = remaining.insert(remaining.end(),&vs[i]);
        }
        vector<Edge>::const_iterator ei;
        SimpleConstraints::const_iterator ci;
        for(ei=es.begin();ei!=es.end();++ei) {
            vs[ei->first].neighbours.push_back(&vs[ei->second]);
            vs[ei->second].neighbours.push_back(&vs[ei->first]);
        }
        map<unsigned,pair<Component*,unsigned> > cmap;
        while(!remaining.empty()) {
            Component* component=new Component;
            Node* v=*remaining.begin();
            dfs(v,remaining,component,cmap);
            components.push_back(component);
        }
        for(ei=es.begin();ei!=es.end();++ei) {
            pair<Component*,unsigned> u=cmap[ei->first],
                                      v=cmap[ei->second];
            assert(u.first==v.first);
            u.first->edges.push_back(make_pair(u.second,v.second));
        }
        for(ci=scx.begin();ci!=scx.end();++ci) {
            SimpleConstraint *c=*ci;
            pair<Component*,unsigned> u=cmap[c->left],
                                      v=cmap[c->right];
            assert(u.first==v.first);
            u.first->scx.push_back(
                    new SimpleConstraint(u.second,v.second,c->gap));
        }
        for(ci=scy.begin();ci!=scy.end();++ci) {
            SimpleConstraint *c=*ci;
            pair<Component*,unsigned> u=cmap[c->left],
                                      v=cmap[c->right];
            assert(u.first==v.first);
            u.first->scy.push_back(
                    new SimpleConstraint(u.second,v.second,c->gap));
        }
    }
    void separateComponents(const vector<Component*> &components) {
        unsigned n=components.size();
        Rectangle* bbs[n];
        double origX[n], origY[n];
        for(unsigned i=0;i<n;i++) {
            bbs[i]=components[i]->getBoundingBox();
            origX[i]=bbs[i]->getCentreX();
            origY[i]=bbs[i]->getCentreY();
        }
        removeRectangleOverlap(n,bbs,0,0);
        for(unsigned i=0;i<n;i++) {
            components[i]->moveRectangles(
                    bbs[i]->getCentreX()-origX[i],
                    bbs[i]->getCentreY()-origY[i]);
            delete bbs[i];
        }
    }
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4
