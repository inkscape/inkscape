#ifndef __SP_LINE_H__
#define __SP_LINE_H__

/*
 * SVG <line> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "svg/svg-length.h"
#include "sp-shape.h"



#define SP_TYPE_LINE            (sp_line_get_type ())
#define SP_LINE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_LINE, SPLine))
#define SP_LINE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_LINE, SPLineClass))
#define SP_IS_LINE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_LINE))
#define SP_IS_LINE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_LINE))

class SPLine;
class SPLineClass;

struct SPLine : public SPShape {
	SVGLength x1;
	SVGLength y1;
	SVGLength x2;
	SVGLength y2;
};

struct SPLineClass {
	SPShapeClass parent_class;
};

GType sp_line_get_type (void);



#endif
