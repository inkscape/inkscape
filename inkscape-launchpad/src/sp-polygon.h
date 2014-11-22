#ifndef SEEN_SP_POLYGON_H
#define SEEN_SP_POLYGON_H

/*
 * SVG <polygon> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-shape.h"

#define SP_POLYGON(obj) (dynamic_cast<SPPolygon*>((SPObject*)obj))
#define SP_IS_POLYGON(obj) (dynamic_cast<const SPPolygon*>((SPObject*)obj) != NULL)

class SPPolygon : public SPShape {
public:
	SPPolygon();
	virtual ~SPPolygon();

	virtual void build(SPDocument *document, Inkscape::XML::Node *repr);
	virtual Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, unsigned int flags);
	virtual void set(unsigned int key, char const* value);
	virtual char* description() const;
};

// made 'public' so that SPCurve can set it as friend:
void sp_polygon_set(SPObject *object, unsigned int key, char const*value);

#endif
