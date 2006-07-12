/** \file
 * Interface between Inkscape code (SPItem) and remove-overlaps function.
 */
/*
* Authors:
*   Tim Dwyer <tgdwyer@gmail.com>
*
* Copyright (C) 2005 Authors
*
* Released under GNU LGPL.  Read the file 'COPYING' for more information.
*/
#include "util/glib-list-iterators.h"
#include "sp-item.h"
#include "sp-item-transform.h"
#include "libvpsc/generate-constraints.h"
#include "libvpsc/remove_rectangle_overlap.h"

/**
* Takes a list of inkscape items and moves them as little as possible
* such that rectangular bounding boxes are separated by at least xGap
* horizontally and yGap vertically
*/
void removeoverlap(GSList const *const items, double const xGap, double const yGap) {
	if(!items) {
		return;
	}

	using Inkscape::Util::GSListConstIterator;
	std::list<SPItem *> selected;
	selected.insert<GSListConstIterator<SPItem *> >(selected.end(), items, NULL);
	if (selected.empty()) return;
	int n=selected.size();

	//Check 2 or more selected objects
	if (n < 2) return;

	Rectangle **rs = new Rectangle*[n];
	int i=0;

	NR::Point const gap(xGap, yGap);
	for (std::list<SPItem *>::iterator it(selected.begin());
		it != selected.end();
		++it)
	{
		using NR::X; using NR::Y;
		NR::Rect const item_box(sp_item_bbox_desktop(*it));
		
		/* The current algorithm requires widths & heights to be strictly positive. */
		NR::Point min(item_box.min());
		NR::Point max(item_box.max());
		for (unsigned d = 0; d < 2; ++d) {
			double const minsize = 1; // arbitrary positive number
			if (max[d] - min[d] + gap[d] < minsize) {
				double const mid = .5 * (min[d] + max[d]);
				min[d] = mid - .5*minsize;
				max[d] = mid + .5*minsize;
			} else {
				min[d] -= .5*gap[d];
				max[d] += .5*gap[d];
			}
		}
		rs[i++] = new Rectangle(min[X], max[X],
					min[Y], max[Y]);
	}
	removeRectangleOverlap(n, rs, 0.0, 0.0);
	i=0;
	for (std::list<SPItem *>::iterator it(selected.begin());
		it != selected.end();
		++it)
	{
		NR::Rect const item_box(sp_item_bbox_desktop(*it));
		Rectangle *r = rs[i++];
		NR::Point const curr(item_box.midpoint());
		NR::Point const dest(r->getCentreX(),
				     r->getCentreY());
		sp_item_move_rel(*it, NR::translate(dest - curr));
		delete r;
	}
	delete [] rs;
}
