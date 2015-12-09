/** \file
 * Interface between Inkscape code (SPItem) and remove-overlaps function.
 */
/*
* Authors:
*   Tim Dwyer <tgdwyer@gmail.com>
 *   Abhishek Sharma
*
* Copyright (C) 2005 Authors
*
* Released under GNU LGPL.  Read the file 'COPYING' for more information.
*/
#include <utility>
#include <2geom/transforms.h>
#include "sp-item.h"
#include "sp-item-transform.h"
#include "libvpsc/generate-constraints.h"
#include "libvpsc/remove_rectangle_overlap.h"
#include "removeoverlap.h"

using vpsc::Rectangle;

namespace {
	struct Record {
		SPItem *item;
		Geom::Point midpoint;
		Rectangle *vspc_rect;

		Record() : item(0), vspc_rect(0) {}
		Record(SPItem *i, Geom::Point m, Rectangle *r)
		: item(i), midpoint(m), vspc_rect(r) {}
	};
}

/**
* Takes a list of inkscape items and moves them as little as possible
* such that rectangular bounding boxes are separated by at least xGap
* horizontally and yGap vertically
*/
void removeoverlap(std::vector<SPItem*> const &items, double const xGap, double const yGap) {
	std::vector<SPItem*> selected(items);
	std::vector<Record> records;
	std::vector<Rectangle *> rs;

	Geom::Point const gap(xGap, yGap);
	for (std::vector<SPItem*>::iterator it(selected.begin());
		it != selected.end();
		++it)
	{
    	SPItem* item = *it;
		using Geom::X; using Geom::Y;
		Geom::OptRect item_box((item)->desktopVisualBounds());
		if (item_box) {
			Geom::Point min(item_box->min() - .5*gap);
			Geom::Point max(item_box->max() + .5*gap);
			// A negative gap is allowed, but will lead to problems when the gap is larger than
			// the bounding box (in either X or Y direction, or both); min will have become max
			// now, which cannot be handled by Rectangle() which is called below. And how will
			// removeRectangleOverlap handle such a case?
			// That's why we will enforce some boundaries on min and max here:
			if (max[X] < min[X]) {
				min[X] = max[X] = (min[X] + max[X])/2;
			}
			if (max[Y] < min[Y]) {
				min[Y] = max[Y] = (min[Y] + max[Y])/2;
			}
			Rectangle *vspc_rect = new Rectangle(min[X], max[X], min[Y], max[Y]);
			records.push_back(Record(item, item_box->midpoint(), vspc_rect));
			rs.push_back(vspc_rect);
		}
	}
	if (!rs.empty()) {
		removeRectangleOverlap(rs.size(), &rs[0], 0.0, 0.0);
	}
	for ( std::vector<Record>::iterator it = records.begin();
	      it != records.end();
	      ++it )
	{
		Geom::Point const curr = it->midpoint;
		Geom::Point const dest(it->vspc_rect->getCentreX(),
				     it->vspc_rect->getCentreY());
		sp_item_move_rel(it->item, Geom::Translate(dest - curr));
		delete it->vspc_rect;
	}
}
