#ifndef __SP_DESC_H__
#define __SP_DESC_H__

/*
 * SVG <desc> implementation
 *
 * Authors:
 *   Jeff Schiller <codedread@gmail.com>
 *
 * Copyright (C) 2008 Jeff Schiller
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-object.h"

#define SP_TYPE_DESC            (sp_desc_get_type ())
#define SP_IS_DESC(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_DESC))

class SPDesc;
class SPDescClass;

struct SPDesc : public SPObject {
};

struct SPDescClass {
	SPObjectClass parent_class;
};

GType sp_desc_get_type (void);

#endif
