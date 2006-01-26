/**
 * \brief Remove overlaps function
 *
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#include <cassert>
#include "constraint.h"
#include "block.h"
#include "blocks.h"
#include "solve_VPSC.h"
#ifdef RECTANGLE_OVERLAP_LOGGING
#include <fstream>
using std::ios;
using std::ofstream;
using std::endl;
#endif

using std::list;
using std::set;

VPSC::VPSC(Variable *vs[], const int n, Constraint *cs[], const int m) : cs(cs), m(m) {
	//assert(!constraintGraphIsCyclic(vs,n));
	bs=new Blocks(vs,n);
#ifdef RECTANGLE_OVERLAP_LOGGING
	printBlocks();
#endif
}
VPSC::~VPSC() {
	delete bs;
}

// useful in debugging
void VPSC::printBlocks() {
#ifdef RECTANGLE_OVERLAP_LOGGING
	ofstream f(LOGFILE,ios::app);
	for(set<Block*>::iterator i=bs->begin();i!=bs->end();i++) {
		Block *b=*i;
		f<<"  "<<*b<<endl;
	}
	for(int i=0;i<m;i++) {
		f<<"  "<<*cs[i]<<endl;
	}
#endif
}
/**
* Produces a feasible - though not necessarily optimal - solution by
* examining blocks in the partial order defined by the directed acyclic
* graph of constraints. For each block (when processing left to right) we
* maintain the invariant that all constraints to the left of the block
* (incoming constraints) are satisfied. This is done by repeatedly merging
* blocks into bigger blocks across violated constraints (most violated
* first) fixing the position of variables inside blocks relative to one
* another so that constraints internal to the block are satisfied.
*/
void VPSC::satisfy() {
	list<Variable*> *vs=bs->totalOrder();
	for(list<Variable*>::iterator i=vs->begin();i!=vs->end();i++) {
		Variable *v=*i;
		if(!v->block->deleted) {
			bs->mergeLeft(v->block);
		}
	}
	bs->cleanup();
	for(int i=0;i<m;i++) {
		if(cs[i]->slack()<-0.0000001) {
#ifdef RECTANGLE_OVERLAP_LOGGING
			ofstream f(LOGFILE,ios::app);
			f<<"Error: Unsatisfied constraint: "<<*cs[i]<<endl;
#endif
			//assert(cs[i]->slack()>-0.0000001);
			throw "Unsatisfied constraint";
		}
	}
	delete vs;
}

void VPSC::refine() {
	bool solved=false;
	// Solve shouldn't loop indefinately
	// ... but just to make sure we limit the number of iterations
	int maxtries=100;
	while(!solved&&maxtries>=0) {
		solved=true;
		maxtries--;
		for(set<Block*>::const_iterator i=bs->begin();i!=bs->end();i++) {
			Block *b=*i;
			b->setUpInConstraints();
			b->setUpOutConstraints();
		}
		for(set<Block*>::const_iterator i=bs->begin();i!=bs->end();i++) {
			Block *b=*i;
			Constraint *c=b->findMinLM();
			if(c!=NULL && c->lm<0) {
#ifdef RECTANGLE_OVERLAP_LOGGING
				ofstream f(LOGFILE,ios::app);
				f<<"Split on constraint: "<<*c<<endl;
#endif
				// Split on c
				Block *l=NULL, *r=NULL;
				bs->split(b,l,r,c);
				bs->cleanup();
				// split alters the block set so we have to restart
				solved=false;
				break;
			}
		}
	}
	for(int i=0;i<m;i++) {
		if(cs[i]->slack()<-0.0000001) {
			assert(cs[i]->slack()>-0.0000001);
			throw "Unsatisfied constraint";
		}
	}
}
/**
 * Calculate the optimal solution. After using satisfy() to produce a
 * feasible solution, refine() examines each block to see if further
 * refinement is possible by splitting the block. This is done repeatedly
 * until no further improvement is possible.
 */
void VPSC::solve() {
	satisfy();
	refine();
}

/**
 * incremental version of solve that should allow refinement after blocks are
 * moved.  Work in progress.
 */
void VPSC::move_and_split() {
	//assert(!blockGraphIsCyclic());
	for(set<Block*>::const_iterator i=bs->begin();i!=bs->end();i++) {
		Block *b=*i;
		if(!b->deleted) {
			b->wposn = b->desiredWeightedPosition();
			b->posn = b->wposn / b->weight;
			Variable *v=b->vars->front();
			bs->mergeLeft(b);
			// b may be merged away, so get any new block from one of its members
			bs->mergeRight(v->block);
		}
	}
	bs->cleanup();
	// assert(!blockGraphIsCyclic());
	refine();
}

#include <map>
using std::map;
using std::vector;
struct node {
	set<node*> in;
	set<node*> out;
};
/*
// useful in debugging - cycles would be BAD
bool VPSC::constraintGraphIsCyclic(Variable *vs[], const int n) {
	map<Variable*, node*> varmap;
	vector<node*> graph;
	for(int i=0;i<n;i++) {
		node *u=new node;
		graph.push_back(u);
		varmap[vs[i]]=u;
	}
	for(int i=0;i<n;i++) {
		for(vector<Constraint*>::iterator c=vs[i]->in.begin();c!=vs[i]->in.end();c++) {
			Variable *l=(*c)->left;
			varmap[vs[i]]->in.insert(varmap[l]);
		}

		for(vector<Constraint*>::iterator c=vs[i]->out.begin();c!=vs[i]->out.end();c++) {
			Variable *r=(*c)->right;
			varmap[vs[i]]->out.insert(varmap[r]);
		}
	}
	while(graph.size()>0) {
		node *u=NULL;
		vector<node*>::iterator i=graph.begin();
		for(;i!=graph.end();i++) {
			u=*i;
			if(u->in.size()==0) {
				break;
			}
		}
		if(i==graph.end() && graph.size()>0) {
			//cycle found!
			return true;
		} else {
			graph.erase(i);
			for(set<node*>::iterator j=u->out.begin();j!=u->out.end();j++) {
				node *v=*j;
				v->in.erase(u);
			}
			delete u;
		}
	}
	for(unsigned i=0; i<graph.size(); i++) {
		delete graph[i];
	}
	return false;
}

// useful in debugging - cycles would be BAD
bool VPSC::blockGraphIsCyclic() {
	map<Block*, node*> bmap;
	vector<node*> graph;
	for(set<Block*>::const_iterator i=bs->begin();i!=bs->end();i++) {
		Block *b=*i;
		node *u=new node;
		graph.push_back(u);
		bmap[b]=u;
	}
	for(set<Block*>::const_iterator i=bs->begin();i!=bs->end();i++) {
		Block *b=*i;
		b->setUpInConstraints();
		Constraint *c=b->findMinInConstraint();
		while(c!=NULL) {
			Block *l=c->left->block;
			bmap[b]->in.insert(bmap[l]);
			b->deleteMinInConstraint();
			c=b->findMinInConstraint();
		}

		b->setUpOutConstraints();
		c=b->findMinOutConstraint();
		while(c!=NULL) {
			Block *r=c->right->block;
			bmap[b]->out.insert(bmap[r]);
			b->deleteMinOutConstraint();
			c=b->findMinOutConstraint();
		}
	}
	while(graph.size()>0) {
		node *u=NULL;
		vector<node*>::iterator i=graph.begin();
		for(;i!=graph.end();i++) {
			u=*i;
			if(u->in.size()==0) {
				break;
			}
		}
		if(i==graph.end() && graph.size()>0) {
			//cycle found!
			return true;
		} else {
			graph.erase(i);
			for(set<node*>::iterator j=u->out.begin();j!=u->out.end();j++) {
				node *v=*j;
				v->in.erase(u);
			}
			delete u;
		}
	}
	for(unsigned i=0; i<graph.size(); i++) {
		delete graph[i];
	}
	return false;
}
*/

