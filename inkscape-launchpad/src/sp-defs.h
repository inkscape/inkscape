#ifndef SEEN_SP_DEFS_H
#define SEEN_SP_DEFS_H

/*
 * SVG <defs> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Abhishek Sharma
 *
 * Copyright (C) 2000-2002 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-object.h"

#define SP_DEFS(obj) (dynamic_cast<SPDefs*>((SPObject*)obj))
#define SP_IS_DEFS(obj) (dynamic_cast<const SPDefs*>((SPObject*)obj) != NULL)

class SPDefs : public SPObject {
public:
	SPDefs();
	virtual ~SPDefs();

protected:
	virtual void release();
	virtual void update(SPCtx* ctx, unsigned int flags);
	virtual void modified(unsigned int flags);
	virtual Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, unsigned int flags);
};

#endif // !SEEN_SP_DEFS_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
