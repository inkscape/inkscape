/**
 * \brief graph layout functions
 *
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef SEEN_GRAPHLAYOUT_H
#define SEEN_GRAPHLAYOUT_H

struct _GSList;
void graphlayout(_GSList const *const items);
class SPItem;
bool isConnector(SPItem const *const item);
#include <list>
void filterConnectors(_GSList const *const items, std::list<SPItem *> &filtered);
#endif // SEEN_GRAPHLAYOUT_H
