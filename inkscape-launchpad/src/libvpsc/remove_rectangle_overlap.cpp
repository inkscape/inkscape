/*
 * remove overlaps between a set of rectangles.
 *
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU LGPL.  Read the file 'COPYING' for more information.
 */

#include <iostream>
#include <cassert>
#include "generate-constraints.h"
#include "solve_VPSC.h"
#include "variable.h"
#include "constraint.h"
#include "remove_rectangle_overlap.h"  /* own include */
#ifdef RECTANGLE_OVERLAP_LOGGING
#include <fstream>
#include "blocks.h"
using std::ios;
using std::ofstream;
using std::endl;
#endif

#define EXTRA_GAP 0.0001
using namespace vpsc;

double Rectangle::xBorder=0;
double Rectangle::yBorder=0;

void removeRectangleOverlap(unsigned n, Rectangle *rs[], double xBorder, double yBorder) {
	try {
	// The extra gap avoids numerical imprecision problems
	Rectangle::setXBorder(xBorder+EXTRA_GAP);
	Rectangle::setYBorder(yBorder+EXTRA_GAP);
	Variable **vs=new Variable*[n];
	for(unsigned i=0;i<n;i++) {
		vs[i]=new Variable(i,0,1);
	}
	Constraint **cs;
	double *oldX = new double[n];
	unsigned m=generateXConstraints(n,rs,vs,cs,true);
	for(unsigned i=0;i<n;i++) {
		oldX[i]=vs[i]->desiredPosition;
	}
	Solver vpsc_x(n,vs,m,cs);
#ifdef RECTANGLE_OVERLAP_LOGGING
	ofstream f(LOGFILE,ios::app);
	f<<"Calling VPSC: Horizontal pass 1"<<endl;
	f.close();
#endif
	vpsc_x.solve();
	for(unsigned i=0;i<n;i++) {
		rs[i]->moveCentreX(vs[i]->position());
	}
	for(unsigned i = 0; i < m; ++i) {
		delete cs[i];
	}
	delete [] cs;
	// Removing the extra gap here ensures things that were moved to be adjacent to
	// one another above are not considered overlapping
	Rectangle::setXBorder(Rectangle::xBorder-EXTRA_GAP);
	m=generateYConstraints(n,rs,vs,cs);
	Solver vpsc_y(n,vs,m,cs);
#ifdef RECTANGLE_OVERLAP_LOGGING
	f.open(LOGFILE,ios::app);
	f<<"Calling VPSC: Vertical pass"<<endl;
	f.close();
#endif
	vpsc_y.solve();
	for(unsigned i=0;i<n;i++) {
		rs[i]->moveCentreY(vs[i]->position());
		rs[i]->moveCentreX(oldX[i]);
	}
	delete [] oldX;
	for(unsigned i = 0; i < m; ++i) {
		delete cs[i];
	}
	delete [] cs;
	Rectangle::setYBorder(Rectangle::yBorder-EXTRA_GAP);
	m=generateXConstraints(n,rs,vs,cs,false);
	Solver vpsc_x2(n,vs,m,cs);
#ifdef RECTANGLE_OVERLAP_LOGGING
	f.open(LOGFILE,ios::app);
	f<<"Calling VPSC: Horizontal pass 2"<<endl;
	f.close();
#endif
	vpsc_x2.solve();
	for(unsigned i = 0; i < m; ++i) {
		delete cs[i];
	}
	delete [] cs;
	for(unsigned i=0;i<n;i++) {
		rs[i]->moveCentreX(vs[i]->position());
		delete vs[i];
	}
	delete [] vs;
	} catch (char const *str) {
		std::cerr<<str<<std::endl;
		for(unsigned i=0;i<n;i++) {
			std::cerr << *rs[i]<<std::endl;
		}
	}
}
