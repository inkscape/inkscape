/**
 * @file
 * graph layout functions.
 */
/*
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef SEEN_GRAPHLAYOUT_H
#define SEEN_GRAPHLAYOUT_H

#include <list>

class SPItem;

void graphlayout(std::vector<SPItem*> const &items);

bool isConnector(SPItem const *const item);

void filterConnectors(std::vector<SPItem*> const &items, std::list<SPItem *> &filtered);

#endif // SEEN_GRAPHLAYOUT_H
