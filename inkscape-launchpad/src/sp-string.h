#ifndef SEEN_SP_STRING_H
#define SEEN_SP_STRING_H

/*
 * string elements
 * extracted from sp-text
 */

#include <glibmm/ustring.h>

#include "sp-object.h"

#define SP_STRING(obj) (dynamic_cast<SPString*>((SPObject*)obj))
#define SP_IS_STRING(obj) (dynamic_cast<const SPString*>((SPObject*)obj) != NULL)

class SPString : public SPObject {
public:
	SPString();
	virtual ~SPString();

    Glib::ustring  string;

	virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);
	virtual void release();

	virtual void read_content();

	virtual void update(SPCtx* ctx, unsigned int flags);
};

#endif
