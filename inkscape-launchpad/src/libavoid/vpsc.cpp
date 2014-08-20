/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 *
 * Copyright (C) 2005-2009  Monash University
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * See the file LICENSE.LGPL distributed with the library.
 *
 * Licensees holding a valid commercial license may use this file in
 * accordance with the commercial license agreement provided with the
 * library.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Author(s):   Tim Dwyer  <Tim.Dwyer@csse.monash.edu.au>
 *
 * --------------
 *
 * This file contains a slightly modified version of Solver() from libvpsc:
 * A solver for the problem of Variable Placement with Separation Constraints.
 * It has the following changes from the Adaptagrams VPSC version:
 *  -  The required VPSC code has been consolidated into a single file.
 *  -  Unnecessary code (like Solver) has been removed.
 *  -  The PairingHeap code has been replaced by a STL priority_queue.
 *
 * Modifications:  Michael Wybrow  <mjwybrow@users.sourceforge.net>
 *
*/

#include <iostream>
#include <math.h>
#include <sstream>
#include <map>
#include <cfloat>
#include <cstdio>

#include "libavoid/vpsc.h"
#include "libavoid/assertions.h"


using namespace std;

namespace Avoid {

static const double ZERO_UPPERBOUND=-1e-10;
static const double LAGRANGIAN_TOLERANCE=-1e-4;

IncSolver::IncSolver(vector<Variable*> const &vs, vector<Constraint *> const &cs)
    : m(cs.size()),
      cs(cs),
      n(vs.size()),
      vs(vs)
{
    for(unsigned i=0;i<n;++i) {
        vs[i]->in.clear();
        vs[i]->out.clear();
    }
    for(unsigned i=0;i<m;++i) {
        Constraint *c=cs[i];
        c->left->out.push_back(c);
        c->right->in.push_back(c);
    }
    bs=new Blocks(vs);
#ifdef LIBVPSC_LOGGING
    printBlocks();
    //COLA_ASSERT(!constraintGraphIsCyclic(n,vs));
#endif

    inactive=cs;
    for(Constraints::iterator i=inactive.begin();i!=inactive.end();++i) {
        (*i)->active=false;
    }
}
IncSolver::~IncSolver() {
    delete bs;
}

// useful in debugging
void IncSolver::printBlocks() {
#ifdef LIBVPSC_LOGGING
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

/*
 * Stores the relative positions of the variables in their finalPosition
 * field.
 */
void IncSolver::copyResult() {
    for(Variables::const_iterator i=vs.begin();i!=vs.end();++i) {
        Variable* v=*i;
        v->finalPosition=v->position();
        COLA_ASSERT(v->finalPosition==v->finalPosition);/// TODO: check! Possibly some error in this line...
    }
}

struct node {
    set<node*> in;
    set<node*> out;
};
// useful in debugging - cycles would be BAD
bool IncSolver::constraintGraphIsCyclic(const unsigned n, Variable* const vs[]) {
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
bool IncSolver::blockGraphIsCyclic() {
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

bool IncSolver::solve() {
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f<<"solve_inc()..."<<endl;
#endif
    satisfy();
    double lastcost = DBL_MAX, cost = bs->cost();
    while(fabs(lastcost-cost)>0.0001) {
        satisfy();
        lastcost=cost;
        cost = bs->cost();
#ifdef LIBVPSC_LOGGING
        f<<"  bs->size="<<bs->size()<<", cost="<<cost<<endl;
#endif
    }
    copyResult();
    return bs->size()!=n;
}
/*
 * incremental version of satisfy that allows refinement after blocks are
 * moved.
 *
 *  - move blocks to new positions
 *  - repeatedly merge across most violated constraint until no more
 *    violated constraints exist
 *
 * Note: there is a special case to handle when the most violated constraint
 * is between two variables in the same block.  Then, we must split the block
 * over an active constraint between the two variables.  We choose the
 * constraint with the most negative lagrangian multiplier.
 */
bool IncSolver::satisfy() {
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f<<"satisfy_inc()..."<<endl;
#endif
    splitBlocks();
    //long splitCtr = 0;
    Constraint* v = NULL;
    //CBuffer buffer(inactive);
    while((v = mostViolated(inactive))
          && (v->equality || ((v->slack() < ZERO_UPPERBOUND) && !v->active)))
    {
        COLA_ASSERT(!v->active);
        Block *lb = v->left->block, *rb = v->right->block;
        if(lb != rb) {
            lb->merge(rb,v);
        } else {
            if(lb->isActiveDirectedPathBetween(v->right,v->left)) {
                // cycle found, relax the violated, cyclic constraint
                v->unsatisfiable=true;
                continue;
                //UnsatisfiableException e;
                //lb->getActiveDirectedPathBetween(e.path,v->right,v->left);
                //e.path.push_back(v);
                //throw e;
            }
            //if(splitCtr++>10000) {
                //throw "Cycle Error!";
            //}
            // constraint is within block, need to split first
            try {
                Constraint* splitConstraint
                    =lb->splitBetween(v->left,v->right,lb,rb);
                if(splitConstraint!=NULL) {
                    COLA_ASSERT(!splitConstraint->active);
                    inactive.push_back(splitConstraint);
                } else {
                    v->unsatisfiable=true;
                    continue;
                }
            } catch(UnsatisfiableException& e) {
                e.path.push_back(v);
                std::cerr << "Unsatisfiable:" << std::endl;
                for(std::vector<Constraint*>::iterator r=e.path.begin();
                        r!=e.path.end();++r)
                {
                    std::cerr << **r <<std::endl;
                }
                v->unsatisfiable=true;
                continue;
            }
            if(v->slack()>=0) {
                COLA_ASSERT(!v->active);
                // v was satisfied by the above split!
                inactive.push_back(v);
                bs->insert(lb);
                bs->insert(rb);
            } else {
                bs->insert(lb->merge(rb,v));
            }
        }
        bs->cleanup();
#ifdef LIBVPSC_LOGGING
        f<<"...remaining blocks="<<bs->size()<<", cost="<<bs->cost()<<endl;
#endif
    }
#ifdef LIBVPSC_LOGGING
    f<<"  finished merges."<<endl;
#endif
    bs->cleanup();
    bool activeConstraints=false;
    for(unsigned i=0;i<m;i++) {
        v=cs[i];
        if(v->active) activeConstraints=true;
        if(v->slack() < ZERO_UPPERBOUND) {
            ostringstream s;
            s<<"Unsatisfied constraint: "<<*v;
#ifdef LIBVPSC_LOGGING
            ofstream f(LOGFILE,ios::app);
            f<<s.str()<<endl;
#endif
            throw s.str().c_str();
        }
    }
#ifdef LIBVPSC_LOGGING
    f<<"  finished cleanup."<<endl;
    printBlocks();
#endif
    copyResult();
    return activeConstraints;
}
void IncSolver::moveBlocks() {
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f<<"moveBlocks()..."<<endl;
#endif
    for(set<Block*>::const_iterator i(bs->begin());i!=bs->end();++i) {
        Block *b = *i;
        b->updateWeightedPosition();
        //b->posn = b->wposn / b->weight;
    }
#ifdef LIBVPSC_LOGGING
    f<<"  moved blocks."<<endl;
#endif
}
void IncSolver::splitBlocks() {
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
#endif
    moveBlocks();
    splitCnt=0;
    // Split each block if necessary on min LM
    for(set<Block*>::const_iterator i(bs->begin());i!=bs->end();++i) {
        Block* b = *i;
        Constraint* v=b->findMinLM();
        if(v!=NULL && v->lm < LAGRANGIAN_TOLERANCE) {
            COLA_ASSERT(!v->equality);
#ifdef LIBVPSC_LOGGING
            f<<"    found split point: "<<*v<<" lm="<<v->lm<<endl;
#endif
            splitCnt++;
            Block *b = v->left->block, *l=NULL, *r=NULL;
            COLA_ASSERT(v->left->block == v->right->block);
            //double pos = b->posn;
            b->split(l,r,v);
            //l->posn=r->posn=pos;
            //l->wposn = l->posn * l->weight;
            //r->wposn = r->posn * r->weight;
            l->updateWeightedPosition();
            r->updateWeightedPosition();
            bs->insert(l);
            bs->insert(r);
            b->deleted=true;
            COLA_ASSERT(!v->active);
            inactive.push_back(v);
#ifdef LIBVPSC_LOGGING
            f<<"  new blocks: "<<*l<<" and "<<*r<<endl;
#endif
        }
    }
    //if(splitCnt>0) { std::cout<<"  splits: "<<splitCnt<<endl; }
#ifdef LIBVPSC_LOGGING
    f<<"  finished splits."<<endl;
#endif
    bs->cleanup();
}

/*
 * Scan constraint list for the most violated constraint, or the first equality
 * constraint
 */
Constraint* IncSolver::mostViolated(Constraints &l) {
    double minSlack = DBL_MAX;
    Constraint* v=NULL;
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f<<"Looking for most violated..."<<endl;
#endif
    Constraints::iterator end = l.end();
    Constraints::iterator deletePoint = end;
    for(Constraints::iterator i=l.begin();i!=end;++i) {
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
    // TODO check this logic and add parens:
    if((deletePoint != end) && (((minSlack < ZERO_UPPERBOUND) && !v->active) || v->equality)) {
        *deletePoint = l[l.size()-1];
        l.resize(l.size()-1);
    }
#ifdef LIBVPSC_LOGGING
    f<<"  most violated is: "<<*v<<endl;
#endif
    return v;
}


using std::set;
using std::vector;
using std::iterator;
using std::list;
using std::copy;
#define __NOTNAN(p) (p)==(p)

long blockTimeCtr;

Blocks::Blocks(vector<Variable*> const &vs) : vs(vs),nvs(vs.size()) {
    blockTimeCtr=0;
    for(int i=0;i<nvs;i++) {
        insert(new Block(vs[i]));
    }
}
Blocks::~Blocks(void)
{
    blockTimeCtr=0;
    for(set<Block*>::iterator i=begin();i!=end();++i) {
        delete *i;
    }
    clear();
}

/*
 * returns a list of variables with total ordering determined by the constraint
 * DAG
 */
list<Variable*> *Blocks::totalOrder() {
    list<Variable*> *order = new list<Variable*>;
    for(int i=0;i<nvs;i++) {
        vs[i]->visited=false;
    }
    for(int i=0;i<nvs;i++) {
        if(vs[i]->in.size()==0) {
            dfsVisit(vs[i],order);
        }
    }
    return order;
}
// Recursive depth first search giving total order by pushing nodes in the DAG
// onto the front of the list when we finish searching them
void Blocks::dfsVisit(Variable *v, list<Variable*> *order) {
    v->visited=true;
    vector<Constraint*>::iterator it=v->out.begin();
    for(;it!=v->out.end();++it) {
        Constraint *c=*it;
        if(!c->right->visited) {
            dfsVisit(c->right, order);
        }
    }
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f<<"  order="<<*v<<endl;
#endif
    order->push_front(v);
}
/*
 * Processes incoming constraints, most violated to least, merging with the
 * neighbouring (left) block until no more violated constraints are found
 */
void Blocks::mergeLeft(Block *r) {
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f<<"mergeLeft called on "<<*r<<endl;
#endif
    r->timeStamp=++blockTimeCtr;
    r->setUpInConstraints();
    Constraint *c=r->findMinInConstraint();
    while (c != NULL && c->slack()<0) {
#ifdef LIBVPSC_LOGGING
        f<<"mergeLeft on constraint: "<<*c<<endl;
#endif
        r->deleteMinInConstraint();
        Block *l = c->left->block;
        if (l->in==NULL) l->setUpInConstraints();
        double dist = c->right->offset - c->left->offset - c->gap;
        if (r->vars->size() < l->vars->size()) {
            dist=-dist;
            std::swap(l, r);
        }
        blockTimeCtr++;
        r->merge(l, c, dist);
        r->mergeIn(l);
        r->timeStamp=blockTimeCtr;
        removeBlock(l);
        c=r->findMinInConstraint();
    }
#ifdef LIBVPSC_LOGGING
    f<<"merged "<<*r<<endl;
#endif
}
/*
 * Symmetrical to mergeLeft
 */
void Blocks::mergeRight(Block *l) {
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f<<"mergeRight called on "<<*l<<endl;
#endif
    l->setUpOutConstraints();
    Constraint *c = l->findMinOutConstraint();
    while (c != NULL && c->slack()<0) {
#ifdef LIBVPSC_LOGGING
        f<<"mergeRight on constraint: "<<*c<<endl;
#endif
        l->deleteMinOutConstraint();
        Block *r = c->right->block;
        r->setUpOutConstraints();
        double dist = c->left->offset + c->gap - c->right->offset;
        if (l->vars->size() > r->vars->size()) {
            dist=-dist;
            std::swap(l, r);
        }
        l->merge(r, c, dist);
        l->mergeOut(r);
        removeBlock(r);
        c=l->findMinOutConstraint();
    }
#ifdef LIBVPSC_LOGGING
    f<<"merged "<<*l<<endl;
#endif
}
void Blocks::removeBlock(Block *doomed) {
    doomed->deleted=true;
    //erase(doomed);
}
void Blocks::cleanup() {
    vector<Block*> bcopy(begin(),end());
    for(vector<Block*>::iterator i=bcopy.begin();i!=bcopy.end();++i) {
        Block *b=*i;
        if(b->deleted) {
            erase(b);
            delete b;
        }
    }
}
/*
 * Splits block b across constraint c into two new blocks, l and r (c's left
 * and right sides respectively)
 */
void Blocks::split(Block *b, Block *&l, Block *&r, Constraint *c) {
    b->split(l,r,c);
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f<<"Split left: "<<*l<<endl;
    f<<"Split right: "<<*r<<endl;
#endif
    r->posn = b->posn;
    //COLA_ASSERT(r->weight!=0);
    //r->wposn = r->posn * r->weight;
    mergeLeft(l);
    // r may have been merged!
    r = c->right->block;
    r->updateWeightedPosition();
    //r->posn = r->wposn / r->weight;
    mergeRight(r);
    removeBlock(b);

    insert(l);
    insert(r);
    COLA_ASSERT(__NOTNAN(l->posn));
    COLA_ASSERT(__NOTNAN(r->posn));
}
/*
 * returns the cost total squared distance of variables from their desired
 * positions
 */
double Blocks::cost() {
    double c = 0;
    for(set<Block*>::iterator i=begin();i!=end();++i) {
        c += (*i)->cost();
    }
    return c;
}

void PositionStats::addVariable(Variable* v) {
    double ai=scale/v->scale;
    double bi=v->offset/v->scale;
    double wi=v->weight;
    AB+=wi*ai*bi;
    AD+=wi*ai*v->desiredPosition;
    A2+=wi*ai*ai;
    /*
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f << "adding v[" << v->id << "], blockscale=" << scale << ", despos="
      << v->desiredPosition << ", ai=" << ai << ", bi=" << bi
      << ", AB=" << AB << ", AD=" << AD << ", A2=" << A2;
#endif
*/
}
void Block::addVariable(Variable* v) {
    v->block=this;
    vars->push_back(v);
    if(ps.A2==0) ps.scale=v->scale;
    //weight+= v->weight;
    //wposn += v->weight * (v->desiredPosition - v->offset);
    //posn=wposn/weight;
    ps.addVariable(v);
    posn=(ps.AD - ps.AB) / ps.A2;
    COLA_ASSERT(__NOTNAN(posn));
    /*
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f << ", posn=" << posn << endl;
#endif
*/
}
Block::Block(Variable* const v)
    : vars(new vector<Variable*>)
    , posn(0)
    //, weight(0)
    //, wposn(0)
    , deleted(false)
    , timeStamp(0)
    , in(NULL)
    , out(NULL)
{
    if(v!=NULL) {
        v->offset=0;
        addVariable(v);
    }
}

void Block::updateWeightedPosition() {
    //wposn=0;
    ps.AB=ps.AD=ps.A2=0;
    for (Vit v=vars->begin();v!=vars->end();++v) {
        //wposn += ((*v)->desiredPosition - (*v)->offset) * (*v)->weight;
        ps.addVariable(*v);
    }
    posn=(ps.AD - ps.AB) / ps.A2;
    COLA_ASSERT(__NOTNAN(posn));
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f << ", posn=" << posn << endl;
#endif
}
Block::~Block(void)
{
    delete vars;
    delete in;
    delete out;
}
void Block::setUpInConstraints() {
    setUpConstraintHeap(in,true);
}
void Block::setUpOutConstraints() {
    setUpConstraintHeap(out,false);
}
void Block::setUpConstraintHeap(Heap* &h,bool in) {
    delete h;
    h = new Heap();
    for (Vit i=vars->begin();i!=vars->end();++i) {
        Variable *v=*i;
        vector<Constraint*> *cs=in?&(v->in):&(v->out);
        for (Cit j=cs->begin();j!=cs->end();++j) {
            Constraint *c=*j;
            c->timeStamp=blockTimeCtr;
            if (((c->left->block != this) && in) || ((c->right->block != this) && !in)) {
                h->push(c);
            }
        }
    }
}
Block* Block::merge(Block* b, Constraint* c) {
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f<<"  merging on: "<<*c<<",c->left->offset="<<c->left->offset<<",c->right->offset="<<c->right->offset<<endl;
#endif
    double dist = c->right->offset - c->left->offset - c->gap;
    Block *l=c->left->block;
    Block *r=c->right->block;
    if (l->vars->size() < r->vars->size()) {
        r->merge(l,c,dist);
    } else {
               l->merge(r,c,-dist);
    }
    Block* mergeBlock=b->deleted?this:b;
#ifdef LIBVPSC_LOGGING
    f<<"  merged block="<<*mergeBlock<<endl;
#endif
    return mergeBlock;
}
/*
 * Merges b into this block across c.  Can be either a
 * right merge or a left merge
 * @param b block to merge into this
 * @param c constraint being merged
 * @param distance separation required to satisfy c
 */
void Block::merge(Block *b, Constraint *c, double dist) {
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f<<"    merging: "<<*b<<"dist="<<dist<<endl;
#endif
    c->active=true;
    //wposn+=b->wposn-dist*b->weight;
    //weight+=b->weight;
    for(Vit i=b->vars->begin();i!=b->vars->end();++i) {
        Variable *v=*i;
        //v->block=this;
        //vars->push_back(v);
        v->offset+=dist;
        addVariable(v);
    }
#ifdef LIBVPSC_LOGGING
    for(Vit i=vars->begin();i!=vars->end();++i) {
        Variable *v=*i;
        f<<"    v["<<v->id<<"]: d="<<v->desiredPosition
            <<" a="<<v->scale<<" o="<<v->offset
            <<endl;
    }
    f<<"  AD="<<ps.AD<<" AB="<<ps.AB<<" A2="<<ps.A2<<endl;
#endif
    //posn=wposn/weight;
    //COLA_ASSERT(wposn==ps.AD - ps.AB);
    posn=(ps.AD - ps.AB) / ps.A2;
    COLA_ASSERT(__NOTNAN(posn));
    b->deleted=true;
}

void Block::mergeIn(Block *b) {
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f<<"  merging constraint heaps... "<<endl;
#endif
    // We check the top of the heaps to remove possible internal constraints
    findMinInConstraint();
    b->findMinInConstraint();
    while (!b->in->empty())
    {
        in->push(b->in->top());
        b->in->pop();
    }
#ifdef LIBVPSC_LOGGING
    f<<"  merged heap: "<<*in<<endl;
#endif
}
void Block::mergeOut(Block *b) {
    findMinOutConstraint();
    b->findMinOutConstraint();
    while (!b->out->empty())
    {
        out->push(b->out->top());
        b->out->pop();
    }
}
Constraint *Block::findMinInConstraint() {
    Constraint *v = NULL;
    vector<Constraint*> outOfDate;
    while (!in->empty()) {
        v = in->top();
        Block *lb=v->left->block;
        Block *rb=v->right->block;
        // rb may not be this if called between merge and mergeIn
#ifdef LIBVPSC_LOGGING
        ofstream f(LOGFILE,ios::app);
        f<<"  checking constraint ... "<<*v;
        f<<"    timestamps: left="<<lb->timeStamp<<" right="<<rb->timeStamp<<" constraint="<<v->timeStamp<<endl;
#endif
        if(lb == rb) {
            // constraint has been merged into the same block
#ifdef LIBVPSC_LOGGING
            if(v->slack()<0) {
                f<<"  violated internal constraint found! "<<*v<<endl;
                f<<"     lb="<<*lb<<endl;
                f<<"     rb="<<*rb<<endl;
            }
#endif
            in->pop();
#ifdef LIBVPSC_LOGGING
            f<<" ... skipping internal constraint"<<endl;
#endif
        } else if(v->timeStamp < lb->timeStamp) {
            // block at other end of constraint has been moved since this
            in->pop();
            outOfDate.push_back(v);
#ifdef LIBVPSC_LOGGING
            f<<"    reinserting out of date (reinsert later)"<<endl;
#endif
        } else {
            break;
        }
    }
    for(Cit i=outOfDate.begin();i!=outOfDate.end();++i) {
        v=*i;
        v->timeStamp=blockTimeCtr;
        in->push(v);
    }
    if(in->empty()) {
        v=NULL;
    } else {
        v=in->top();
    }
    return v;
}
Constraint *Block::findMinOutConstraint() {
    if(out->empty()) return NULL;
    Constraint *v = out->top();
    while (v->left->block == v->right->block) {
        out->pop();
        if(out->empty()) return NULL;
        v = out->top();
    }
    return v;
}
void Block::deleteMinInConstraint() {
    in->pop();
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f<<"deleteMinInConstraint... "<<endl;
    f<<"  result: "<<*in<<endl;
#endif
}
void Block::deleteMinOutConstraint() {
    out->pop();
}
inline bool Block::canFollowLeft(Constraint const* c, Variable const* last) const {
    return c->left->block==this && c->active && last!=c->left;
}
inline bool Block::canFollowRight(Constraint const* c, Variable const* last) const {
    return c->right->block==this && c->active && last!=c->right;
}

// computes the derivative of v and the lagrange multipliers
// of v's out constraints (as the recursive sum of those below.
// Does not backtrack over u.
// also records the constraint with minimum lagrange multiplier
// in min_lm
double Block::compute_dfdv(Variable* const v, Variable* const u,
               Constraint *&min_lm) {
    double dfdv=v->dfdv();
    for(Cit it=v->out.begin();it!=v->out.end();++it) {
        Constraint *c=*it;
        if(canFollowRight(c,u)) {
            c->lm=compute_dfdv(c->right,v,min_lm);
            dfdv+=c->lm*c->left->scale;
            if(!c->equality&&(min_lm==NULL||c->lm<min_lm->lm)) min_lm=c;
        }
    }
    for(Cit it=v->in.begin();it!=v->in.end();++it) {
        Constraint *c=*it;
        if(canFollowLeft(c,u)) {
            c->lm=-compute_dfdv(c->left,v,min_lm);
            dfdv-=c->lm*c->right->scale;
            if(!c->equality&&(min_lm==NULL||c->lm<min_lm->lm)) min_lm=c;
        }
    }
    return dfdv/v->scale;
}
double Block::compute_dfdv(Variable* const v, Variable* const u) {
    double dfdv = v->dfdv();
    for(Cit it = v->out.begin(); it != v->out.end(); ++it) {
        Constraint *c = *it;
        if(canFollowRight(c,u)) {
            c->lm =   compute_dfdv(c->right,v);
            dfdv += c->lm * c->left->scale;
        }
    }
    for(Cit it=v->in.begin();it!=v->in.end();++it) {
        Constraint *c = *it;
        if(canFollowLeft(c,u)) {
            c->lm = - compute_dfdv(c->left,v);
            dfdv -= c->lm * c->right->scale;
        }
    }
    return dfdv/v->scale;
}

// The top level v and r are variables between which we want to find the
// constraint with the smallest lm.
// Similarly, m is initially NULL and is only assigned a value if the next
// variable to be visited is r or if a possible min constraint is returned from
// a nested call (rather than NULL).
// Then, the search for the m with minimum lm occurs as we return from
// the recursion (checking only constraints traversed left-to-right
// in order to avoid creating any new violations).
// We also do not consider equality constraints as potential split points
bool Block::split_path(
    Variable* r,
    Variable* const v,
    Variable* const u,
    Constraint* &m,
    bool desperation=false
    )
{
    for(Cit it(v->in.begin());it!=v->in.end();++it) {
        Constraint *c=*it;
        if(canFollowLeft(c,u)) {
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f<<"  left split path: "<<*c<<endl;
#endif
            if(c->left==r) {
                if(desperation&&!c->equality) m=c;
                return true;
            } else {
                if(split_path(r,c->left,v,m)) {
                    if(desperation && !c->equality && (!m||c->lm<m->lm)) {
                               m=c;
                    }
                    return true;
                }
            }
        }
    }
    for(Cit it(v->out.begin());it!=v->out.end();++it) {
        Constraint *c=*it;
        if(canFollowRight(c,u)) {
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f<<"  right split path: "<<*c<<endl;
#endif
            if(c->right==r) {
                if(!c->equality) m=c;
                return true;
            } else {
                if(split_path(r,c->right,v,m)) {
                    if(!c->equality && (!m||c->lm<m->lm))
                               m=c;
                    return true;
                }
            }
        }
    }
    return false;
}
/*
Block::Pair Block::compute_dfdv_between(
        Variable* r, Variable* const v, Variable* const u,
        const Direction dir = NONE, bool changedDirection = false) {
    double dfdv=v->weight*(v->position() - v->desiredPosition);
    Constraint *m=NULL;
    for(Cit it(v->in.begin());it!=v->in.end();++it) {
        Constraint *c=*it;
        if(canFollowLeft(c,u)) {
            if(dir==RIGHT) {
                changedDirection = true;
            }
            if(c->left==r) {
                       r=NULL;
                    if(!c->equality) m=c;
            }
            Pair p=compute_dfdv_between(r,c->left,v,
                    LEFT,changedDirection);
            dfdv -= c->lm = -p.first;
            if(r && p.second)
                m = p.second;
        }
    }
    for(Cit it(v->out.begin());it!=v->out.end();++it) {
        Constraint *c=*it;
        if(canFollowRight(c,u)) {
            if(dir==LEFT) {
                changedDirection = true;
            }
            if(c->right==r) {
                       r=NULL;
                    if(!c->equality) m=c;
            }
            Pair p=compute_dfdv_between(r,c->right,v,
                    RIGHT,changedDirection);
            dfdv += c->lm = p.first;
            if(r && p.second)
                m = changedDirection && !c->equality && c->lm < p.second->lm
                    ? c
                    : p.second;
        }
    }
    return Pair(dfdv,m);
}
*/

// resets LMs for all active constraints to 0 by
// traversing active constraint tree starting from v,
// not back tracking over u
void Block::reset_active_lm(Variable* const v, Variable* const u) {
    for(Cit it=v->out.begin();it!=v->out.end();++it) {
        Constraint *c=*it;
        if(canFollowRight(c,u)) {
            c->lm=0;
            reset_active_lm(c->right,v);
        }
    }
    for(Cit it=v->in.begin();it!=v->in.end();++it) {
        Constraint *c=*it;
        if(canFollowLeft(c,u)) {
            c->lm=0;
            reset_active_lm(c->left,v);
        }
    }
}
void Block::list_active(Variable* const v, Variable* const u) {
    for(Cit it=v->out.begin();it!=v->out.end();++it) {
        Constraint *c=*it;
        if(canFollowRight(c,u)) {
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f<<"  "<<*c<<endl;
#endif
            list_active(c->right,v);
        }
    }
    for(Cit it=v->in.begin();it!=v->in.end();++it) {
        Constraint *c=*it;
        if(canFollowLeft(c,u)) {
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f<<"  "<<*c<<endl;
#endif
            list_active(c->left,v);
        }
    }
}
/*
 * finds the constraint with the minimum lagrange multiplier, that is, the constraint
 * that most wants to split
 */
Constraint *Block::findMinLM() {
    Constraint *min_lm=NULL;
    reset_active_lm(vars->front(),NULL);
    compute_dfdv(vars->front(),NULL,min_lm);
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f<<"  langrangians: "<<endl;
    list_active(vars->front(),NULL);
#endif
    return min_lm;
}
Constraint *Block::findMinLMBetween(Variable* const lv, Variable* const rv) {
    reset_active_lm(vars->front(),NULL);
    compute_dfdv(vars->front(),NULL);
    Constraint *min_lm=NULL;
    split_path(rv,lv,NULL,min_lm);
#if 0
    if(min_lm==NULL) {
        split_path(rv,lv,NULL,min_lm,true);
    }
#else
    if(min_lm==NULL) {
        fprintf(stderr,"Couldn't find split point!\n");
        UnsatisfiableException e;
        getActivePathBetween(e.path,lv,rv,NULL);
        throw e;
    }
    COLA_ASSERT(min_lm!=NULL);
#endif
    return min_lm;
}

// populates block b by traversing the active constraint tree adding variables as they're
// visited.  Starts from variable v and does not backtrack over variable u.
void Block::populateSplitBlock(Block *b, Variable* v, Variable const* u) {
    b->addVariable(v);
    for (Cit c=v->in.begin();c!=v->in.end();++c) {
        if (canFollowLeft(*c,u))
            populateSplitBlock(b, (*c)->left, v);
    }
    for (Cit c=v->out.begin();c!=v->out.end();++c) {
        if (canFollowRight(*c,u))
            populateSplitBlock(b, (*c)->right, v);
    }
}
/*
 * Returns the active path between variables u and v... not back tracking over w
 */
bool Block::getActivePathBetween(Constraints& path, Variable const* u,
               Variable const* v, Variable const *w) const {
    if(u==v) return true;
    for (Cit_const c=u->in.begin();c!=u->in.end();++c) {
        if (canFollowLeft(*c,w)) {
            if(getActivePathBetween(path, (*c)->left, v, u)) {
                path.push_back(*c);
                return true;
            }
        }
    }
    for (Cit_const c=u->out.begin();c!=u->out.end();++c) {
        if (canFollowRight(*c,w)) {
            if(getActivePathBetween(path, (*c)->right, v, u)) {
                path.push_back(*c);
                return true;
            }
        }
    }
    return false;
}
// Search active constraint tree from u to see if there is a directed path to v.
// Returns true if path is found with all constraints in path having their visited flag
// set true.
bool Block::isActiveDirectedPathBetween(Variable const* u, Variable const* v) const {
    if(u==v) return true;
    for (Cit_const c=u->out.begin();c!=u->out.end();++c) {
        if(canFollowRight(*c,NULL)) {
            if(isActiveDirectedPathBetween((*c)->right,v)) {
                return true;
            }
        }
    }
    return false;
}
bool Block::getActiveDirectedPathBetween(
        Constraints& path, Variable const* u, Variable const* v) const {
    if(u==v) return true;
    for (Cit_const c=u->out.begin();c!=u->out.end();++c) {
        if(canFollowRight(*c,NULL)) {
            if(getActiveDirectedPathBetween(path,(*c)->right,v)) {
                path.push_back(*c);
                return true;
            }
        }
    }
    return false;
}
/*
 * Block needs to be split because of a violated constraint between vl and vr.
 * We need to search the active constraint tree between l and r and find the constraint
 * with min lagrangrian multiplier and split at that point.
 * Returns the split constraint
 */
Constraint* Block::splitBetween(Variable* const vl, Variable* const vr,
               Block* &lb, Block* &rb) {
#ifdef LIBVPSC_LOGGING
    ofstream f(LOGFILE,ios::app);
    f<<"  need to split between: "<<*vl<<" and "<<*vr<<endl;
#endif
    Constraint *c=findMinLMBetween(vl, vr);
    if(c!=NULL) {
#ifdef LIBVPSC_LOGGING
    f<<"  going to split on: "<<*c<<endl;
#endif
        split(lb,rb,c);
        deleted = true;
    }
    return c;
}

/*
 * Creates two new blocks, l and r, and splits this block across constraint c,
 * placing the left subtree of constraints (and associated variables) into l
 * and the right into r.
 */
void Block::split(Block* &l, Block* &r, Constraint* c) {
    c->active=false;
    l=new Block();
    populateSplitBlock(l,c->left,c->right);
    //COLA_ASSERT(l->weight>0);
    r=new Block();
    populateSplitBlock(r,c->right,c->left);
    //COLA_ASSERT(r->weight>0);
}

/*
 * Computes the cost (squared euclidean distance from desired positions) of the
 * current positions for variables in this block
 */
double Block::cost() {
    double c = 0;
    for (Vit v=vars->begin();v!=vars->end();++v) {
        double diff = (*v)->position() - (*v)->desiredPosition;
        c += (*v)->weight * diff * diff;
    }
    return c;
}
ostream& operator <<(ostream &os, const Block& b)
{
    os<<"Block(posn="<<b.posn<<"):";
    for(Block::Vit v=b.vars->begin();v!=b.vars->end();++v) {
        os<<" "<<**v;
    }
    if(b.deleted) {
        os<<" Deleted!";
    }
    return os;
}

Constraint::Constraint(Variable *left, Variable *right, double gap, bool equality)
: left(left),
  right(right),
  gap(gap),
  timeStamp(0),
  active(false),
  equality(equality),
  unsatisfiable(false)
{
    // In hindsight I think it's probably better to build the constraint DAG
    // (by creating variable in/out lists) when needed, rather than in advance
    //left->out.push_back(this);
    //right->in.push_back(this);
}
Constraint::~Constraint() {
    // see constructor: the following is just way too slow.
    // Better to create a
    // new DAG on demand than maintain the lists dynamically.
    //Constraints::iterator i;
    //for(i=left->out.begin(); i!=left->out.end(); i++) {
        //if(*i==this) break;
    //}
    //left->out.erase(i);
    //for(i=right->in.begin(); i!=right->in.end(); i++) {
        //if(*i==this) break;
    //}
    //right->in.erase(i);
}
double Constraint::slack() const {
    return unsatisfiable ? DBL_MAX
           : right->scale * right->position()
         - gap - left->scale * left->position();
}
std::ostream& operator <<(std::ostream &os, const Constraint &c)
{
    if(&c==NULL) {
        os<<"NULL";
    } else {
        const char *type=c.equality?"=":"<=";
        std::ostringstream lscale, rscale;
        if(c.left->scale!=1) {
            lscale << c.left->scale << "*";
        }
        if(c.right->scale!=1) {
            rscale << c.right->scale << "*";
        }
        os<<lscale.str()<<*c.left<<"+"<<c.gap<<type<<rscale.str()<<*c.right;
        if(c.left->block&&c.right->block)
            os<<"("<<c.slack()<<")"<<(c.active?"-active":"")
                <<"(lm="<<c.lm<<")";
        else
            os<<"(vars have no position)";
    }
    return os;
}

bool CompareConstraints::operator() (
    Constraint *const &l, Constraint *const &r
) const {
    double const sl =
        l->left->block->timeStamp > l->timeStamp
        ||l->left->block==l->right->block
        ?-DBL_MAX:l->slack();
    double const sr =
        r->left->block->timeStamp > r->timeStamp
        ||r->left->block==r->right->block
        ?-DBL_MAX:r->slack();
    if(sl==sr) {
        // arbitrary choice based on id
        if(l->left->id==r->left->id) {
            if(l->right->id<r->right->id) return true;
            return false;
        }
        if(l->left->id<r->left->id) return true;
        return false;
    }
    return sl > sr;
}

std::ostream& operator <<(std::ostream &os, const Variable &v) {
    if(v.block)
        os << "(" << v.id << "=" << v.position() << ")";
    else
        os << "(" << v.id << "=" << v.desiredPosition << ")";
    return os;
}

}
