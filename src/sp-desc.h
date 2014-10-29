#ifndef SEEN_SP_DESC_H
#define SEEN_SP_DESC_H

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

#define SP_DESC(obj) (dynamic_cast<SPDesc*>((SPObject*)obj))
#define SP_IS_DESC(obj) (dynamic_cast<const SPDesc*>((SPObject*)obj) != NULL)

class SPDesc : public SPObject {
public:
	SPDesc();
	virtual ~SPDesc();

protected:
	virtual Inkscape::XML::Node* write(Inkscape::XML::Document* doc, Inkscape::XML::Node* repr, unsigned int flags);
};

#endif
