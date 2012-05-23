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

#include <glib.h>
#include "sp-object.h"

class SPObjectClass;

struct SPMeshRow;
struct SPMeshRowClass;

#define SP_TYPE_MESHROW (sp_meshrow_get_type())
#define SP_MESHROW(o) (G_TYPE_CHECK_INSTANCE_CAST((o), SP_TYPE_MESHROW, SPMeshRow))
#define SP_MESHROW_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), SP_TYPE_MESHROW, SPMeshRowClass))
#define SP_IS_MESHROW(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), SP_TYPE_MESHROW))
#define SP_IS_MESHROW_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE((k), SP_TYPE_MESHROW))

GType sp_meshrow_get_type();

/** Gradient MeshRow. */
struct SPMeshRow : public SPObject {

    SPMeshRow* getNextMeshRow();
    SPMeshRow* getPrevMeshRow();
};

/// The SPMeshRow vtable.
struct SPMeshRowClass {
    SPObjectClass parent_class;
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
