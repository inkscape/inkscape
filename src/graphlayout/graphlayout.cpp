/** \file
 * Interface between Inkscape code (SPItem) and graphlayout functions.
 */
/*
* Authors:
*   Tim Dwyer <tgdwyer@gmail.com>
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
#include <float.h>

#include "util/glib-list-iterators.h"
#include "graphlayout/graphlayout.h"
#include "sp-path.h"
#include "sp-item.h"
#include "sp-item-transform.h"
#include "sp-conn-end-pair.h"
#include "style.h"
#include "conn-avoid-ref.h"
#include "libavoid/connector.h"
#include "libavoid/geomtypes.h"
#include "libcola/cola.h"
#include "libvpsc/generate-constraints.h"
#include "prefs-utils.h"

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

/**
 * Scans the items list and places those items that are 
 * not connectors in filtered
 */
void filterConnectors(GSList const *const items, list<SPItem *> &filtered) {
	for(GSList *i=(GSList *)items; i!=NULL; i=i->next) {
		SPItem *item=SP_ITEM(i->data);
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
void graphlayout(GSList const *const items) {
	if(!items) {
		return;
	}

	using Inkscape::Util::GSListConstIterator;
	list<SPItem *> selected;
	filterConnectors(items,selected);
	if (selected.empty()) return;

	const unsigned n=selected.size();
	//Check 2 or more selected objects
	if (n < 2) return;

	double minX=DBL_MAX, minY=DBL_MAX, maxX=-DBL_MAX, maxY=-DBL_MAX;

	map<string,unsigned> nodelookup;
	vector<Rectangle*> rs;
	vector<Edge> es;
	for (list<SPItem *>::iterator i(selected.begin());
		i != selected.end();
		++i)
	{
		SPItem *u=*i;
		NR::Rect const item_box(sp_item_bbox_desktop(u));
		NR::Point ll(item_box.min());
		NR::Point ur(item_box.max());
		minX=min(ll[0],minX); minY=min(ll[1],minY);
	        maxX=max(ur[0],maxX); maxY=max(ur[1],maxY);
		nodelookup[u->id]=rs.size();
		cout << "Node " << rs.size() << endl;
		rs.push_back(new Rectangle(ll[0],ur[0],ll[1],ur[1]));
	}

	SimpleConstraints scy;
	double ideal_connector_length = prefs_get_double_attribute("tools.connector","length",100);
	double directed_edge_height_modifier = 1.0;
	gchar const *directed_str = NULL, *overlaps_str = NULL;
	directed_str = prefs_get_string_attribute("tools.connector",
		       	"directedlayout");
	overlaps_str = prefs_get_string_attribute("tools.connector",
		       	"avoidoverlaplayout");
	bool avoid_overlaps = false;
	bool directed = false;
        if (directed_str && !strcmp(directed_str, "true")) {
            cout << "Directed layout requested.\n";
	    directed = true;
	}
        if (overlaps_str && !strcmp(overlaps_str, "true")) {
            cout << "Avoid overlaps requested.\n";
	    avoid_overlaps = true;
	}

	for (list<SPItem *>::iterator i(selected.begin());
		i != selected.end();
		++i)
	{
		SPItem *iu=*i;
		unsigned u=nodelookup[iu->id];
		GSList *nlist=iu->avoidRef->getAttachedConnectors(Avoid::runningFrom);
		list<SPItem *> connectors;
		
		connectors.insert<GSListConstIterator<SPItem *> >(connectors.end(),nlist,NULL);
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
		
			// What do we do if iv not in nodelookup?!?!
			map<string,unsigned>::iterator v_pair=nodelookup.find(iv->id);
			if(v_pair!=nodelookup.end()) {
				unsigned v=v_pair->second;
				cout << "Edge: (" << u <<","<<v<<")"<<endl;
				es.push_back(make_pair(u,v));
				if(conn->style->marker[SP_MARKER_LOC_END].set) {
					if(directed && strcmp(conn->style->marker[SP_MARKER_LOC_END].value,"none")) {
						cout << conn->style->marker[SP_MARKER_LOC_END].value << endl;
						scy.push_back(new SimpleConstraint(v, u, 
                                    (ideal_connector_length * directed_edge_height_modifier)));
					}
				}
			}
		}
		if(nlist) {
			g_slist_free(nlist);
		}
	}
	double width=maxX-minX;
	double height=maxY-minY;
	const unsigned E = es.size();
	double eweights[E];
	fill(eweights,eweights+E,1);

	ConstrainedMajorizationLayout alg(rs,es,eweights,ideal_connector_length);
	alg.setupConstraints(NULL,NULL,avoid_overlaps,
			NULL,NULL,NULL,&scy,NULL,NULL);
	alg.run();
	
	for (list<SPItem *>::iterator it(selected.begin());
		it != selected.end();
		++it)
	{
		SPItem *u=*it;
		if(!isConnector(u)) {
			Rectangle* r=rs[nodelookup[u->id]];
			NR::Rect const item_box(sp_item_bbox_desktop(u));
			NR::Point const curr(item_box.midpoint());
			NR::Point const dest(r->getCentreX(),r->getCentreY());
			sp_item_move_rel(u, NR::translate(dest - curr));
		}
	}
}
// vim: set cindent 
// vim: ts=4 sw=4 et tw=0 wm=0
