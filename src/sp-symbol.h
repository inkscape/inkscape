#ifndef __SP_SYMBOL_H__
#define __SP_SYMBOL_H__

/*
 * SVG <symbol> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2003 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/*
 * This is quite similar in logic to <svg>
 * Maybe we should merge them somehow (Lauris)
 */

#define SP_TYPE_SYMBOL (sp_symbol_get_type ())
#define SP_SYMBOL(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_SYMBOL, SPSymbol))
#define SP_IS_SYMBOL(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_SYMBOL))

class SPSymbol;
class SPSymbolClass;

#include <libnr/nr-matrix.h>
#include <libnr/nr-rect.h>
#include "svg/svg-length.h"
#include "enums.h"
#include "sp-item-group.h"

struct SPSymbol : public SPGroup {
	/* viewBox; */
	unsigned int viewBox_set : 1;
	NRRect viewBox;

	/* preserveAspectRatio */
	unsigned int aspect_set : 1;
	unsigned int aspect_align : 4;
	unsigned int aspect_clip : 1;

	/* Child to parent additional transform */
	NR::Matrix c2p;
};

struct SPSymbolClass {
	SPGroupClass parent_class;
};

GType sp_symbol_get_type (void);

#endif
