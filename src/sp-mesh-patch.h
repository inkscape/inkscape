#ifndef SEEN_SP_MESHPATCH_H
#define SEEN_SP_MESHPATCH_H

/** \file
 * SPMeshpatch: SVG <meshpatch> implementation.
 */
/*
 * Authors: Tavmjong Bah
 *
 * Copyright (C) 2012 Tavmjong Bah
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/ustring.h>
#include "sp-object.h"

#define SP_MESHPATCH(obj) (dynamic_cast<SPMeshpatch*>((SPObject*)obj))
#define SP_IS_MESHPATCH(obj) (dynamic_cast<const SPMeshpatch*>((SPObject*)obj) != NULL)

/** Gradient Meshpatch. */
class SPMeshpatch : public SPObject {
public:
    SPMeshpatch();
    virtual ~SPMeshpatch();

    SPMeshpatch* getNextMeshpatch();
    SPMeshpatch* getPrevMeshpatch();
    Glib::ustring * tensor_string;
    //SVGLength tx[4];  // Tensor points
    //SVGLength ty[4];  // Tensor points

protected:
    virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);
    virtual void set(unsigned int key, const char* value);
    virtual void modified(unsigned int flags);
    virtual Inkscape::XML::Node* write(Inkscape::XML::Document* doc, Inkscape::XML::Node* repr, unsigned int flags);
};

#endif /* !SEEN_SP_MESHPATCH_H */

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
