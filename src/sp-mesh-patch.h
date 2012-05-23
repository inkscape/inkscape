#ifndef SEEN_SP_MESHPATCH_H
#define SEEN_SP_MESHPATCH_H

/** \file
 * SPMeshPatch: SVG <meshpatch> implementation.
 */
/*
 * Authors: Tavmjong Bah
 *
 * Copyright (C) 2012 Tavmjong Bah
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>
#include <glibmm/ustring.h>
//#include "svg/svg-length.h"
#include "sp-object.h"

class SPObjectClass;

struct SPMeshPatch;
struct SPMeshPatchClass;

#define SP_TYPE_MESHPATCH (sp_meshpatch_get_type())
#define SP_MESHPATCH(o) (G_TYPE_CHECK_INSTANCE_CAST((o), SP_TYPE_MESHPATCH, SPMeshPatch))
#define SP_MESHPATCH_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), SP_TYPE_MESHPATCH, SPMeshPatchClass))
#define SP_IS_MESHPATCH(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), SP_TYPE_MESHPATCH))
#define SP_IS_MESHPATCH_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE((k), SP_TYPE_MESHPATCH))

GType sp_meshpatch_get_type();

/** Gradient MeshPatch. */
struct SPMeshPatch : public SPObject {

    SPMeshPatch* getNextMeshPatch();
    SPMeshPatch* getPrevMeshPatch();
    Glib::ustring * tensor_string;
    //SVGLength tx[4];  // Tensor points
    //SVGLength ty[4];  // Tensor points
};

/// The SPMeshPatch vtable.
struct SPMeshPatchClass {
    SPObjectClass parent_class;
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
