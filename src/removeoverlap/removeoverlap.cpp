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
#include <utility>

using vpsc::Rectangle;

namespace {
	struct Record {
		SPItem *item;
		NR::Point midpoint;
		Rectangle *vspc_rect;

		Record() {}
		Record(SPItem *i, NR::Point m, Rectangle *r)
		: item(i), midpoint(m), vspc_rect(r) {}
	};
}

/**
* Takes a list of inkscape items and moves them as little as possible
* such that rectangular bounding boxes are separated by at least xGap
* horizontally and yGap vertically
*/
void removeoverlap(GSList const *const items, double const xGap, double const yGap) {
	using Inkscape::Util::GSListConstIterator;
	std::list<SPItem *> selected;
	selected.insert<GSListConstIterator<SPItem *> >(selected.end(), items, NULL);
	std::vector<Record> records;
	std::vector<Rectangle *> rs;

	NR::Point const gap(xGap, yGap);
	for (std::list<SPItem *>::iterator it(selected.begin());
		it != selected.end();
		++it)
	{
		using NR::X; using NR::Y;
		boost::optional<NR::Rect> item_box(sp_item_bbox_desktop(*it));
		if (item_box) {
			NR::Point min(item_box->min() - .5*gap);
			NR::Point max(item_box->max() + .5*gap);
			Rectangle *vspc_rect = new Rectangle(min[X], max[X], min[Y], max[Y]);
			records.push_back(Record(*it, item_box->midpoint(), vspc_rect));
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
		NR::Point const curr = it->midpoint;
		NR::Point const dest(it->vspc_rect->getCentreX(),
				     it->vspc_rect->getCentreY());
		sp_item_move_rel(it->item, Geom::Translate(dest - curr));
		delete it->vspc_rect;
	}
}
