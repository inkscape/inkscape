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

typedef struct _GSList GSList;
class SPItem;

void graphlayout(GSList const *const items);

bool isConnector(SPItem const *const item);

void filterConnectors(GSList const *const items, std::list<SPItem *> &filtered);

#endif // SEEN_GRAPHLAYOUT_H
