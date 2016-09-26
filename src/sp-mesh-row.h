#ifndef SEEN_SP_MESHROW_H
#define SEEN_SP_MESHROW_H

/** \file
 * SPMeshrow: SVG <meshrow> implementation.
 */
/*
 * Authors: Tavmjong Bah
 * Copyright (C) 2012 Tavmjong Bah
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-object.h"

#define SP_MESHROW(obj) (dynamic_cast<SPMeshrow*>((SPObject*)obj))
#define SP_IS_MESHROW(obj) (dynamic_cast<const SPMeshrow*>((SPObject*)obj) != NULL)

/** Gradient Meshrow. */
class SPMeshrow : public SPObject {
public:
    SPMeshrow();
    virtual ~SPMeshrow();

    SPMeshrow* getNextMeshrow();
    SPMeshrow* getPrevMeshrow();

protected:
    virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);
    virtual void set(unsigned int key, const char* value);
    virtual void modified(unsigned int flags);
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
