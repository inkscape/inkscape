#ifndef SEEN_INK_STOCK_ITEMS_H
#define SEEN_INK_STOCK_ITEMS_H

/*
 * Stock-items
 *
 * Stock Item management code
 *
 * Authors:
 *  John Cliff <simarilius@yahoo.com>
 *
 * Copyright 2004 John Cliff
 *
 */

#include <glib/gtypes.h>

class SPObject;

SPObject *get_stock_item(gchar const *urn);

#endif // SEEN_INK_STOCK_ITEMS_H
