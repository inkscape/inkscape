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
#define SP_GENERICELLIPSE(obj) (dynamic_cast<SPGenericEllipse*>((SPObject*)obj))
#define SP_IS_GENERICELLIPSE(obj) (dynamic_cast<const SPGenericEllipse*>((SPObject*)obj) != NULL)

class SPGenericEllipse : public SPShape {
public:
	SPGenericEllipse();
	virtual ~SPGenericEllipse();

	SVGLength cx;
	SVGLength cy;
	SVGLength rx;
	SVGLength ry;

	unsigned int closed : 1;
	double start, end;

	virtual void update(SPCtx* ctx, unsigned int flags);
	virtual Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags);

	virtual void snappoints(std::vector<Inkscape::SnapCandidatePoint> &p, Inkscape::SnapPreferences const *snapprefs);
	virtual void set_shape();

	virtual void update_patheffect(bool write);
};

/* This is technically priate by we need this in object edit (Lauris) */
void sp_genericellipse_normalize (SPGenericEllipse *ellipse);

/* SVG <ellipse> element */
#define SP_ELLIPSE(obj) (dynamic_cast<SPEllipse*>((SPObject*)obj))
#define SP_IS_ELLIPSE(obj) (dynamic_cast<const SPEllipse*>((SPObject*)obj) != NULL)

class SPEllipse : public SPGenericEllipse {
public:
	SPEllipse();
	virtual ~SPEllipse();

	virtual void build(SPDocument *document, Inkscape::XML::Node *repr);
	virtual Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags);
	virtual void set(unsigned int key, gchar const* value);
	virtual gchar* description();
};

void sp_ellipse_position_set (SPEllipse * ellipse, gdouble x, gdouble y, gdouble rx, gdouble ry);

/* SVG <circle> element */
#define SP_CIRCLE(obj) (dynamic_cast<SPCircle*>((SPObject*)obj))
#define SP_IS_CIRCLE(obj) (dynamic_cast<const SPCircle*>((SPObject*)obj) != NULL)

class SPCircle : public SPGenericEllipse {
public:
	SPCircle();
	virtual ~SPCircle();

	virtual void build(SPDocument *document, Inkscape::XML::Node *repr);
	virtual Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags);
	virtual void set(unsigned int key, gchar const* value);
	virtual gchar* description();
};

/* <path sodipodi:type="arc"> element */
#define SP_ARC(obj) (dynamic_cast<SPArc*>((SPObject*)obj))
#define SP_IS_ARC(obj) (dynamic_cast<const SPArc*>((SPObject*)obj) != NULL)

class SPArc : public SPGenericEllipse {
public:
	SPArc();
	virtual ~SPArc();

	virtual void build(SPDocument *document, Inkscape::XML::Node *repr);
	virtual Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags);
	virtual void set(unsigned int key, gchar const* value);
	virtual gchar* description();
	virtual void modified(unsigned int flags);
};

void sp_arc_position_set (SPArc * arc, gdouble x, gdouble y, gdouble rx, gdouble ry);
Geom::Point sp_arc_get_xy (SPArc *ge, gdouble arg);

#endif
