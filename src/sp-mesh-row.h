#ifndef SEEN_SP_MESHROW_H
#define SEEN_SP_MESHROW_H

/** \file
 * SPMeshRow: SVG <meshRow> implementation.
 */
/*
 * Authors: Tavmjong Bah
 * Copyright (C) 2012 Tavmjong Bah
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-object.h"

#define SP_MESHROW(obj) (dynamic_cast<SPMeshRow*>((SPObject*)obj))
#define SP_IS_MESHROW(obj) (dynamic_cast<const SPMeshRow*>((SPObject*)obj) != NULL)

/** Gradient MeshRow. */
class SPMeshRow : public SPObject {
public:
	SPMeshRow();
	virtual ~SPMeshRow();

    SPMeshRow* getNextMeshRow();
    SPMeshRow* getPrevMeshRow();

protected:
	virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);
	virtual void set(unsigned int key, const char* value);
	virtual Inkscape::XML::Node* write(Inkscape::XML::Document* doc, Inkscape::XML::Node* repr, unsigned int flags);
};

#endif /* !SEEN_SP_MESHROW_H */

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
