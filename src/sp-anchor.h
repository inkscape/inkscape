#ifndef SEEN_SP_ANCHOR_H
#define SEEN_SP_ANCHOR_H

/*
 * SVG <a> element implementation
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-item-group.h"

#define SP_ANCHOR(obj) (dynamic_cast<SPAnchor*>((SPObject*)obj))
#define SP_IS_ANCHOR(obj) (dynamic_cast<const SPAnchor*>((SPObject*)obj) != NULL)

class SPAnchor : public SPGroup {
public:
	SPAnchor();
	virtual ~SPAnchor();

	char *href;

	virtual void build(SPDocument *document, Inkscape::XML::Node *repr);
	virtual void release();
	virtual void set(unsigned int key, char const* value);
	virtual Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, unsigned int flags);

    virtual const char* displayName() const;
	virtual char* description() const;
	virtual int event(SPEvent *event);
};

#endif
