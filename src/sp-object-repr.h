#ifndef __SP_OBJECT_REPR_H__
#define __SP_OBJECT_REPR_H__

/*
 * Object type dictionary and build frontend
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2003 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "forward.h"

namespace Inkscape {
namespace XML {
class Node;
}
}


SPObject *sp_object_repr_build_tree (Document *document, Inkscape::XML::Node *repr);

GType sp_repr_type_lookup (Inkscape::XML::Node *repr);

void sp_object_type_register(gchar const *name, GType type);

#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
