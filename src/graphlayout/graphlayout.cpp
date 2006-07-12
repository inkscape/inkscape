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
#include "util/glib-list-iterators.h"
#include "graphlayout/graphlayout.h"
#include <iostream>
#include <config.h>

#include "sp-path.h"
#include "sp-item.h"
#include "sp-item-transform.h"
#include "sp-conn-end-pair.h"
#include "conn-avoid-ref.h"
#include "libavoid/connector.h"
#include "libavoid/geomtypes.h"
#include "libcola/cola.h"
#include "libvpsc/generate-constraints.h"
#include <map>
#include <vector>
#include <algorithm>
#include <float.h>

using namespace std;
using namespace cola;

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
	cout<<"|V|="<<n<<endl;
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
		cout<<"Creating node for id: "<<u->id<<endl;
		nodelookup[u->id]=rs.size();
		rs.push_back(new Rectangle(ll[0],ur[0],ll[1],ur[1]));
	}


	for (list<SPItem *>::iterator i(selected.begin());
		i != selected.end();
		++i)
	{
		SPItem *iu=*i;
		cout<<"Getting neighbours for id: "<<iu->id<<endl;
		unsigned u=nodelookup[iu->id];
		GSList *nlist=iu->avoidRef->getAttachedShapes(Avoid::runningFrom);
		list<SPItem *> neighbours;
		neighbours.insert<GSListConstIterator<SPItem *> >(neighbours.end(),nlist,NULL);
		for (list<SPItem *>::iterator j(neighbours.begin());
				j != neighbours.end();
				++j) {
			
			SPItem *iv=*j;
			// What do we do if iv not in nodelookup?!?!
			unsigned v=nodelookup[iv->id];
			es.push_back(make_pair(u,v));
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

	ConstrainedMajorizationLayout alg(rs,es,eweights,width/n);
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
