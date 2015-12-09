/**
 * @file
 * Interface between Inkscape code (SPItem) and graphlayout functions.
 */
/*
 * Authors:
 *   Tim Dwyer <Tim.Dwyer@infotech.monash.edu.au>
 *   Abhishek Sharma
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#include <iostream>
#include <config.h>
#include <map>
#include <vector>
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <float.h>
#include <2geom/transforms.h>

#include "desktop.h"
#include "inkscape.h"
#include "sp-namedview.h"
#include "graphlayout.h"
#include "sp-path.h"
#include "sp-item.h"
#include "sp-item-transform.h"
#include "sp-conn-end-pair.h"
#include "style.h"
#include "conn-avoid-ref.h"
#include "libavoid/connector.h"
#include "libavoid/router.h"
#include "libavoid/geomtypes.h"
#include "libcola/cola.h"
#include "libvpsc/generate-constraints.h"
#include "preferences.h"

using namespace std;
using namespace cola;
using namespace vpsc;
/**
 * Returns true if item is a connector
 */
bool isConnector(SPItem const *const i) {
    SPPath *path = NULL;
    if(SP_IS_PATH(i)) {
        path = SP_PATH(i);
    }
    return path && path->connEndPair.isAutoRoutingConn();
}

struct CheckProgress : TestConvergence {
    CheckProgress(double d,unsigned i,list<SPItem *>&
                  selected,vector<Rectangle*>& rs,map<string,unsigned>& nodelookup) :
        TestConvergence(d,i), selected(selected), rs(rs), nodelookup(nodelookup) {}
    bool operator()(double new_stress, double* X, double* Y) {
        /* This is where, if we wanted to animate the layout, we would need to update
         * the positions of all objects and redraw the canvas and maybe sleep a bit
         cout << "stress="<<new_stress<<endl;
        cout << "x[0]="<<rs[0]->getMinX()<<endl;
        for (list<SPItem *>::iterator it(selected.begin());
            it != selected.end();
            ++it)
        {
            SPItem *u=*it;
            if(!isConnector(u)) {
                Rectangle* r=rs[nodelookup[u->id]];
                Geom::Rect const item_box(sp_item_bbox_desktop(u));
                Geom::Point const curr(item_box.midpoint());
                Geom::Point const dest(r->getCentreX(),r->getCentreY());
                sp_item_move_rel(u, Geom::Translate(dest - curr));
            }
        }
        */
        return TestConvergence::operator()(new_stress,X,Y);
    }
    list<SPItem *>& selected;
    vector<Rectangle*>& rs;
    map<string,unsigned>& nodelookup;
};

/**
 * Scans the items list and places those items that are
 * not connectors in filtered
 */
void filterConnectors(std::vector<SPItem*> const &items, list<SPItem *> &filtered) {
    for(std::vector<SPItem*>::const_iterator i = items.begin();i !=items.end(); ++i){
        SPItem *item = *i;
        if(!isConnector(item)) {
            filtered.push_back(item);
        }
    }
}
/**
* Takes a list of inkscape items, extracts the graph defined by
* connectors between them, and uses graph layout techniques to find
* a nice layout
*/
void graphlayout(std::vector<SPItem*> const &items) {
    if(items.empty()) {
        return;
    }

    list<SPItem *> selected;
    filterConnectors(items,selected);
    if (selected.empty()) return;

    const unsigned n=selected.size();
    //Check 2 or more selected objects
    if (n < 2) return;

    // add the connector spacing to the size of node bounding boxes
    // so that connectors can always be routed between shapes
    SPDesktop* desktop = SP_ACTIVE_DESKTOP;
    double spacing = 0;
    if(desktop) spacing = desktop->namedview->connector_spacing+0.1;

    map<string,unsigned> nodelookup;
    vector<Rectangle*> rs;
    vector<Edge> es;
    for (list<SPItem *>::iterator i(selected.begin());
         i != selected.end();
         ++i)
    {
        SPItem *u=*i;
        Geom::OptRect const item_box = u->desktopVisualBounds();
        if(item_box) {
            Geom::Point ll(item_box->min());
            Geom::Point ur(item_box->max());
            nodelookup[u->getId()]=rs.size();
            rs.push_back(new Rectangle(ll[0]-spacing,ur[0]+spacing,
                        ll[1]-spacing,ur[1]+spacing));
        } else {
            // I'm not actually sure if it's possible for something with a
            // NULL item-box to be attached to a connector in which case we
            // should never get to here... but if such a null box can occur it's
            // probably pretty safe to simply ignore
            //fprintf(stderr,"NULL item_box found in graphlayout, ignoring!\n");
        }
    }

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    SimpleConstraints scx,scy;
    double ideal_connector_length = prefs->getDouble("/tools/connector/length", 100.0);
    double directed_edge_height_modifier = 1.0;

    bool directed =       prefs->getBool("/tools/connector/directedlayout");
    bool avoid_overlaps = prefs->getBool("/tools/connector/avoidoverlaplayout");

    for (list<SPItem *>::iterator i(selected.begin());
         i != selected.end();
         ++i)
    {
        SPItem *iu=*i;
        map<string,unsigned>::iterator i_iter=nodelookup.find(iu->getId());
        map<string,unsigned>::iterator i_iter_end=nodelookup.end();
        if(i_iter==i_iter_end) {
            continue;
        }
        unsigned u=i_iter->second;
        std::vector<SPItem *> nlist=iu->avoidRef->getAttachedConnectors(Avoid::runningFrom);
        list<SPItem *> connectors;

        connectors.insert(connectors.end(), nlist.begin(), nlist.end());

        for (list<SPItem *>::iterator j(connectors.begin());
                j != connectors.end();
                ++j) {
            SPItem *conn=*j;
            SPItem *iv;
            SPItem *items[2];
            assert(isConnector(conn));
            SP_PATH(conn)->connEndPair.getAttachedItems(items);
            if(items[0]==iu) {
                iv=items[1];
            } else {
                iv=items[0];
            }

            if (iv == NULL) {
                // The connector is not attached to anything at the
                // other end so we should just ignore it.
                continue;
            }

            // If iv not in nodelookup we again treat the connector
            // as disconnected and continue
            map<string,unsigned>::iterator v_pair=nodelookup.find(iv->getId());
            if(v_pair!=nodelookup.end()) {
                unsigned v=v_pair->second;
                //cout << "Edge: (" << u <<","<<v<<")"<<endl;
                es.push_back(make_pair(u,v));
                if(conn->style->marker_end.set) {
                    if(directed && strcmp(conn->style->marker_end.value,"none")) {
                        scy.push_back(new SimpleConstraint(v, u,
                                    (ideal_connector_length * directed_edge_height_modifier)));
                    }
                }
            }
        }
    }
    const unsigned E = es.size();
    double eweights[E];
    fill(eweights,eweights+E,1);
    vector<Component*> cs;
    connectedComponents(rs,es,scx,scy,cs);
    for(unsigned i=0;i<cs.size();i++) {
        Component* c=cs[i];
        if(c->edges.size()<2) continue;
        CheckProgress test(0.0001,100,selected,rs,nodelookup);
        ConstrainedMajorizationLayout alg(c->rects,c->edges,eweights,ideal_connector_length,test);
        alg.setupConstraints(NULL,NULL,avoid_overlaps,
                NULL,NULL,&c->scx,&c->scy,NULL,NULL);
        alg.run();
    }
    separateComponents(cs);

    for (list<SPItem *>::iterator it(selected.begin());
         it != selected.end();
         ++it)
    {
        SPItem *u=*it;
        if(!isConnector(u)) {
            map<string,unsigned>::iterator i=nodelookup.find(u->getId());
            if(i!=nodelookup.end()) {
                Rectangle* r=rs[i->second];
                Geom::OptRect item_box = u->desktopVisualBounds();
                if (item_box) {
                    Geom::Point const curr(item_box->midpoint());
                    Geom::Point const dest(r->getCentreX(),r->getCentreY());
                    sp_item_move_rel(u, Geom::Translate(dest - curr));
                }
            }
        }
    }
    for(unsigned i=0;i<scx.size();i++) {
        delete scx[i];
    }
    for(unsigned i=0;i<scy.size();i++) {
        delete scy[i];
    }
    for(unsigned i=0;i<rs.size();i++) {
        delete rs[i];
    }
}
// vim: set cindent
// vim: ts=4 sw=4 et tw=0 wm=0

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
