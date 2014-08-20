/*
 * Solve an instance of the "Variable Placement with Separation
 * Constraints" problem.
 *
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU LGPL.  Read the file 'COPYING' for more information.
 */

#include <cassert>
#include "constraint.h"
#include "block.h"
#include "blocks.h"
#include "solve_VPSC.h"
#include <math.h>
#include <sstream>
#ifdef RECTANGLE_OVERLAP_LOGGING
#include <fstream>
#endif
#include <map>

using namespace std;

namespace vpsc {

static const double ZERO_UPPERBOUND=-0.0000001;

IncSolver::IncSolver(const unsigned n, Variable* const vs[], const unsigned m, Constraint *cs[]) 
	: Solver(n,vs,m,cs) {
	inactive.assign(cs,cs+m);
	for(ConstraintList::iterator i=inactive.begin();i!=inactive.end();++i) {
		(*i)->active=false;
	}
}
Solver::Solver(const unsigned n, Variable* const vs[], const unsigned m, Constraint *cs[]) : m(m), cs(cs), n(n), vs(vs) {
	bs=new Blocks(n, vs);
#ifdef RECTANGLE_OVERLAP_LOGGING
	printBlocks();
	//assert(!constraintGraphIsCyclic(n,vs));
#endif
}
Solver::~Solver() {
	delete bs;
}

// useful in debugging
void Solver::printBlocks() {
#ifdef RECTANGLE_OVERLAP_LOGGING
	ofstream f(LOGFILE,ios::app);
	for(set<Block*>::iterator i=bs->begin();i!=bs->end();++i) {
		Block *b=*i;
		f<<"  "<<*b<<endl;
	}
	for(unsigned i=0;i<m;i++) {
		f<<"  "<<*cs[i]<<endl;
	}
#endif
}

void Solver::satisfy() {
	list<Variable*> *vs=bs->totalOrder();
	for(list<Variable*>::iterator i=vs->begin();i!=vs->end();++i) {
		Variable *v=*i;
		if(!v->block->deleted) {
			bs->mergeLeft(v->block);
		}
	}
	bs->cleanup();
	for(unsigned i=0;i<m;i++) {
		if(cs[i]->slack() < ZERO_UPPERBOUND) {
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

void Solver::refine() {
	bool solved=false;
	// Solve shouldn't loop indefinately
	// ... but just to make sure we limit the number of iterations
	unsigned maxtries=100;
	while(!solved&&maxtries>0) {
		solved=true;
		maxtries--;
		for(set<Block*>::const_iterator i=bs->begin();i!=bs->end();++i) {
			Block *b=*i;
			b->setUpInConstraints();
			b->setUpOutConstraints();
		}
		for(set<Block*>::const_iterator i=bs->begin();i!=bs->end();++i) {
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
	for(unsigned i=0;i<m;i++) {
		if(cs[i]->slack() < ZERO_UPPERBOUND) {
			assert(cs[i]->slack()>ZERO_UPPERBOUND);
			throw "Unsatisfied constraint";
		}
	}
}

void Solver::solve() {
	satisfy();
	refine();
}

void IncSolver::solve() {
#ifdef RECTANGLE_OVERLAP_LOGGING
	ofstream f(LOGFILE,ios::app);
	f<<"solve_inc()..."<<endl;
#endif
	double lastcost,cost = bs->cost();
	do {
		lastcost=cost;
		satisfy();
		splitBlocks();
		cost = bs->cost();
#ifdef RECTANGLE_OVERLAP_LOGGING
	f<<"  cost="<<cost<<endl;
#endif
	} while(fabs(lastcost-cost)>0.0001);
}

void IncSolver::satisfy() {
#ifdef RECTANGLE_OVERLAP_LOGGING
	ofstream f(LOGFILE,ios::app);
	f<<"satisfy_inc()..."<<endl;
#endif
	splitBlocks();
	long splitCtr = 0;
	Constraint* v = NULL;
	while((v=mostViolated(inactive))&&(v->equality || v->slack() < ZERO_UPPERBOUND)) {
		assert(!v->active);
		Block *lb = v->left->block, *rb = v->right->block;
		if(lb != rb) {
			lb->merge(rb,v);
		} else {
			if(lb->isActiveDirectedPathBetween(v->right,v->left)) {
				// cycle found, relax the violated, cyclic constraint
				v->gap = v->slack();
				continue;
			}
			if(splitCtr++>10000) {
				throw "Cycle Error!";
			}
			// constraint is within block, need to split first
			inactive.push_back(lb->splitBetween(v->left,v->right,lb,rb));
			lb->merge(rb,v);
			bs->insert(lb);
		}
	}
#ifdef RECTANGLE_OVERLAP_LOGGING
	f<<"  finished merges."<<endl;
#endif
	bs->cleanup();
	for(unsigned i=0;i<m;i++) {
		v=cs[i];
		if(v->slack() < ZERO_UPPERBOUND) {
			ostringstream s;
			s<<"Unsatisfied constraint: "<<*v;
#ifdef RECTANGLE_OVERLAP_LOGGING
			ofstream f(LOGFILE,ios::app);
			f<<s.str()<<endl;
#endif
			throw s.str().c_str();
		}
	}
#ifdef RECTANGLE_OVERLAP_LOGGING
	f<<"  finished cleanup."<<endl;
	printBlocks();
#endif
}
void IncSolver::moveBlocks() {
#ifdef RECTANGLE_OVERLAP_LOGGING
	ofstream f(LOGFILE,ios::app);
	f<<"moveBlocks()..."<<endl;
#endif
	for(set<Block*>::const_iterator i(bs->begin());i!=bs->end();++i) {
		Block *b = *i;
		b->wposn = b->desiredWeightedPosition();
		b->posn = b->wposn / b->weight;
	}
#ifdef RECTANGLE_OVERLAP_LOGGING
	f<<"  moved blocks."<<endl;
#endif
}
void IncSolver::splitBlocks() {
#ifdef RECTANGLE_OVERLAP_LOGGING
	ofstream f(LOGFILE,ios::app);
#endif
	moveBlocks();
	splitCnt=0;
	// Split each block if necessary on min LM
	for(set<Block*>::const_iterator i(bs->begin());i!=bs->end();++i) {
		Block* b = *i;
		Constraint* v=b->findMinLM();
		if(v!=NULL && v->lm < ZERO_UPPERBOUND) {
			assert(!v->equality);
#ifdef RECTANGLE_OVERLAP_LOGGING
			f<<"    found split point: "<<*v<<" lm="<<v->lm<<endl;
#endif
			splitCnt++;
			Block *b = v->left->block, *l=NULL, *r=NULL;
			assert(v->left->block == v->right->block);
			double pos = b->posn;
			b->split(l,r,v);
			l->posn=r->posn=pos;
			l->wposn = l->posn * l->weight;
			r->wposn = r->posn * r->weight;
			bs->insert(l);
			bs->insert(r);
			b->deleted=true;
			inactive.push_back(v);
#ifdef RECTANGLE_OVERLAP_LOGGING
			f<<"  new blocks: "<<*l<<" and "<<*r<<endl;
#endif
		}
	}
#ifdef RECTANGLE_OVERLAP_LOGGING
	f<<"  finished splits."<<endl;
#endif
	bs->cleanup();
}

Constraint* IncSolver::mostViolated(ConstraintList &l) {
	double minSlack = DBL_MAX;
	Constraint* v=NULL;
#ifdef RECTANGLE_OVERLAP_LOGGING
	ofstream f(LOGFILE,ios::app);
	f<<"Looking for most violated..."<<endl;
#endif
	ConstraintList::iterator end = l.end();
	ConstraintList::iterator deletePoint = end;
	for(ConstraintList::iterator i=l.begin();i!=end;++i) {
		Constraint *c=*i;
		double slack = c->slack();
		if(c->equality || slack < minSlack) {
			minSlack=slack;	
			v=c;
			deletePoint=i;
			if(c->equality) break;
		}
	}
	// Because the constraint list is not order dependent we just
	// move the last element over the deletePoint and resize
	// downwards.  There is always at least 1 element in the
	// vector because of search.
	if(deletePoint != end && (minSlack<ZERO_UPPERBOUND||v->equality)) {
		*deletePoint = l[l.size()-1];
		l.resize(l.size()-1);
	}
#ifdef RECTANGLE_OVERLAP_LOGGING
	f<<"  most violated is: "<<*v<<endl;
#endif
	return v;
}

struct node {
	set<node*> in;
	set<node*> out;
};
// useful in debugging - cycles would be BAD
bool Solver::constraintGraphIsCyclic(const unsigned n, Variable* const vs[]) {
	map<Variable*, node*> varmap;
	vector<node*> graph;
	for(unsigned i=0;i<n;i++) {
		node *u=new node;
		graph.push_back(u);
		varmap[vs[i]]=u;
	}
	for(unsigned i=0;i<n;i++) {
		for(vector<Constraint*>::iterator c=vs[i]->in.begin();c!=vs[i]->in.end();++c) {
			Variable *l=(*c)->left;
			varmap[vs[i]]->in.insert(varmap[l]);
		}

		for(vector<Constraint*>::iterator c=vs[i]->out.begin();c!=vs[i]->out.end();++c) {
			Variable *r=(*c)->right;
			varmap[vs[i]]->out.insert(varmap[r]);
		}
	}
	while(!graph.empty()) {
		node *u=NULL;
		vector<node*>::iterator i=graph.begin();
		for(;i!=graph.end();++i) {
			u=*i;
			if(u->in.empty()) {
				break;
			}
		}
		if(i==graph.end() && !graph.empty()) {
			//cycle found!
			return true;
		} else {
			graph.erase(i);
			for(set<node*>::iterator j=u->out.begin();j!=u->out.end();++j) {
				node *v=*j;
				v->in.erase(u);
			}
			delete u;
		}
	}
	for(unsigned i=0; i<graph.size(); ++i) {
		delete graph[i];
	}
	return false;
}

// useful in debugging - cycles would be BAD
bool Solver::blockGraphIsCyclic() {
	map<Block*, node*> bmap;
	vector<node*> graph;
	for(set<Block*>::const_iterator i=bs->begin();i!=bs->end();++i) {
		Block *b=*i;
		node *u=new node;
		graph.push_back(u);
		bmap[b]=u;
	}
	for(set<Block*>::const_iterator i=bs->begin();i!=bs->end();++i) {
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
	while(!graph.empty()) {
		node *u=NULL;
		vector<node*>::iterator i=graph.begin();
		for(;i!=graph.end();++i) {
			u=*i;
			if(u->in.empty()) {
				break;
			}
		}
		if(i==graph.end() && !graph.empty()) {
			//cycle found!
			return true;
		} else {
			graph.erase(i);
			for(set<node*>::iterator j=u->out.begin();j!=u->out.end();++j) {
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
}
