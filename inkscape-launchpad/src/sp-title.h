#ifndef __SP_TITLE_H__
#define __SP_TITLE_H__

/*
 * SVG <title> implementation
 *
 * Authors:
 *   Jeff Schiller <codedread@gmail.com>
 *
 * Copyright (C) 2008 Jeff Schiller
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-object.h"

#define SP_TITLE(obj) (dynamic_cast<SPTitle*>((SPObject*)obj))
#define SP_IS_TITLE(obj) (dynamic_cast<const SPTitle*>((SPObject*)obj) != NULL)

class SPTitle : public SPObject {
public:
	SPTitle();
	virtual ~SPTitle();

	virtual Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags);
};

#endif
