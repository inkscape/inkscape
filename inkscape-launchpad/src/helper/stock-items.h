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

#include <glib.h>

class SPObject;

SPObject *get_stock_item(gchar const *urn, gboolean stock=FALSE);

#endif // SEEN_INK_STOCK_ITEMS_H
