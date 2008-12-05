#ifndef __SP_ELLIPSE_H__
#define __SP_ELLIPSE_H__

/*
 * SVG <ellipse> and related implementations
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Mitsuru Oka
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "svg/svg-length.h"
#include "sp-shape.h"

/* Common parent class */

#define SP_TYPE_GENERICELLIPSE (sp_genericellipse_get_type ())
#define SP_GENERICELLIPSE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_GENERICELLIPSE, SPGenericEllipse))
#define SP_GENERICELLIPSE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_GENERICELLIPSE, SPGenericEllipseClass))
#define SP_IS_GENERICELLIPSE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_GENERICELLIPSE))
#define SP_IS_GENERICELLIPSE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_GENERICELLIPSE))

class SPGenericEllipse;
class SPGenericEllipseClass;

struct SPGenericEllipse : public SPShape {
	SVGLength cx;
	SVGLength cy;
	SVGLength rx;
	SVGLength ry;

	unsigned int closed : 1;
	double start, end;
};

struct SPGenericEllipseClass {
	SPShapeClass parent_class;
};

GType sp_genericellipse_get_type (void);

/* This is technically priate by we need this in object edit (Lauris) */
void sp_genericellipse_normalize (SPGenericEllipse *ellipse);

/* SVG <ellipse> element */

#define SP_TYPE_ELLIPSE (sp_ellipse_get_type ())
#define SP_ELLIPSE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_ELLIPSE, SPEllipse))
#define SP_ELLIPSE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_ELLIPSE, SPEllipseClass))
#define SP_IS_ELLIPSE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_ELLIPSE))
#define SP_IS_ELLIPSE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_ELLIPSE))

struct SPEllipse : public SPGenericEllipse {
};

struct SPEllipseClass {
	SPGenericEllipseClass parent_class;
};

GType sp_ellipse_get_type (void);

void sp_ellipse_position_set (SPEllipse * ellipse, gdouble x, gdouble y, gdouble rx, gdouble ry);

/* SVG <circle> element */

#define SP_TYPE_CIRCLE (sp_circle_get_type ())
#define SP_CIRCLE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_CIRCLE, SPCircle))
#define SP_CIRCLE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_CIRCLE, SPCircleClass))
#define SP_IS_CIRCLE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_CIRCLE))
#define SP_IS_CIRCLE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_CIRCLE))

struct SPCircle : public SPGenericEllipse {
};

struct SPCircleClass {
	SPGenericEllipseClass parent_class;
};

GType sp_circle_get_type (void);

/* <path sodipodi:type="arc"> element */

#define SP_TYPE_ARC (sp_arc_get_type ())
#define SP_ARC(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_ARC, SPArc))
#define SP_ARC_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_ARC, SPArcClass))
#define SP_IS_ARC(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_ARC))
#define SP_IS_ARC_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_ARC))

struct SPArc : public SPGenericEllipse {
};

struct SPArcClass {
	SPGenericEllipseClass parent_class;
};

GType sp_arc_get_type (void);
void sp_arc_position_set (SPArc * arc, gdouble x, gdouble y, gdouble rx, gdouble ry);
Geom::Point sp_arc_get_xy (SPArc *ge, gdouble arg);

#endif
