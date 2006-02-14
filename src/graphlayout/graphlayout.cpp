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
#include "graphlayout/graphlayout.h"
#include <iostream>
#include <config.h>

#ifdef HAVE_BOOST_GRAPH_LIB
#include "util/glib-list-iterators.h"
#include "sp-path.h"
#include "sp-item.h"
#include "sp-item-transform.h"
#include "sp-conn-end-pair.h"
#include "conn-avoid-ref.h"
#include "libavoid/connector.h"
#include "libavoid/geomtypes.h"
#include <boost/graph/kamada_kawai_spring_layout.hpp>
#include <boost/graph/circle_layout.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <map>
#include <vector>
#include <algorithm>
#include <float.h>

using namespace boost;

// create a typedef for the Graph type
typedef adjacency_list<vecS, vecS, undirectedS, no_property, 
	property<edge_weight_t, double> > Graph;
typedef property_map<Graph, edge_weight_t>::type WeightMap;
typedef graph_traits<Graph>::vertex_descriptor Vertex;
typedef std::vector<Avoid::Point> PositionVec;
typedef iterator_property_map<PositionVec::iterator, property_map<Graph, vertex_index_t>::type> PositionMap;

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
void filterConnectors(GSList const *const items, std::list<SPItem *> &filtered) {
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
	std::list<SPItem *> selected;
	filterConnectors(items,selected);
	if (selected.empty()) return;

	int n=selected.size();
	std::cout<<"|V|="<<n<<std::endl;

	Graph g;

	double minX=DBL_MAX, minY=DBL_MAX, maxX=-DBL_MAX, maxY=-DBL_MAX;

	std::map<std::string,Vertex> nodelookup;
	for (std::list<SPItem *>::iterator i(selected.begin());
		i != selected.end();
		++i)
	{
		SPItem *u=*i;
		std::cout<<"Creating node for id: "<<u->id<<std::endl;
		nodelookup[u->id]=add_vertex(g);
	}

	//Check 2 or more selected objects
	if (n < 2) return;

	WeightMap weightmap=get(edge_weight, g);
	for (std::list<SPItem *>::iterator i(selected.begin());
		i != selected.end();
		++i)
	{
		using NR::X; using NR::Y;
		SPItem *iu=*i;
		std::cout<<"Getting neighbours for id: "<<iu->id<<std::endl;
		Vertex u=nodelookup[iu->id];
		GSList *nlist=iu->avoidRef->getAttachedShapes(Avoid::ConnRef::runningFrom);
		std::list<SPItem *> neighbours;
		neighbours.insert<GSListConstIterator<SPItem *> >(neighbours.end(),nlist,NULL);
		for (std::list<SPItem *>::iterator j(neighbours.begin());
				j != neighbours.end();
				++j) {
			
			SPItem *iv=*j;
			Vertex v=nodelookup[iv->id];
			Graph::edge_descriptor e; bool inserted;
			tie(e, inserted)=add_edge(u,v,g);
			weightmap[e]=1.0;
		}
		if(nlist) {
			g_slist_free(nlist);
		}
		NR::Rect const item_box(sp_item_bbox_desktop(*i));
			
		NR::Point ll(item_box.min());
		minX=std::min(ll[0],minX);
		minY=std::min(ll[1],minY);
		NR::Point ur(item_box.max());
		maxX=std::max(ur[0],maxX);
		maxY=std::max(ur[1],maxY);
	}
	double width=maxX-minX;
	double height=maxY-minY;
	std::cout<<"Graph has |V|="<<num_vertices(g)<<" Width="<<width<<" Height="<<height<<std::endl;
  	PositionVec position_vec(num_vertices(g));
  	PositionMap position(position_vec.begin(), get(vertex_index, g));
  	write_graphviz(std::cout, g);
	circle_graph_layout(g, position, width/2.0);
	kamada_kawai_spring_layout(g, position, weightmap, side_length(width));

	graph_traits<Graph>::vertex_iterator vi, vi_end;
	for (std::list<SPItem *>::iterator it(selected.begin());
		it != selected.end();
		++it)
	{
		SPItem *u=*it;
		if(!isConnector(u)) {
			NR::Rect const item_box(sp_item_bbox_desktop(u));
			NR::Point const curr(item_box.midpoint());
			NR::Point const dest(minX+width/2.0+position[nodelookup[u->id]].x,
				     	minY+height/2.0+position[nodelookup[u->id]].y);
			sp_item_move_rel(u, NR::translate(dest - curr));
		}
	}
}
#else
void graphlayout(GSList const *const items) {
	std::cout<<"Connector network layout not available!  Install boost graph library and recompile to enable."<<std::endl;
}
#endif // HAVE_BOOST_GRAPH_LIB
