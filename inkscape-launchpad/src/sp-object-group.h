#ifndef SEEN_SP_OBJECTGROUP_H
#define SEEN_SP_OBJECTGROUP_H

/*
 * Abstract base class for non-item groups
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Abhishek Sharma
 *
 * Copyright (C) 1999-2003 Authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-object.h"

#define SP_OBJECTGROUP(obj) (dynamic_cast<SPObjectGroup*>((SPObject*)obj))
#define SP_IS_OBJECTGROUP(obj) (dynamic_cast<const SPObjectGroup*>((SPObject*)obj) != NULL)

class SPObjectGroup : public SPObject {
public:
	SPObjectGroup();
	virtual ~SPObjectGroup();

protected:
	virtual void child_added(Inkscape::XML::Node* child, Inkscape::XML::Node* ref);
	virtual void remove_child(Inkscape::XML::Node* child);

	virtual void order_changed(Inkscape::XML::Node* child, Inkscape::XML::Node* old, Inkscape::XML::Node* new_repr);

	virtual Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, unsigned int flags);
};

#endif // SEEN_SP_OBJECTGROUP_H
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
