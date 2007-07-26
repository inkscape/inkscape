#ifndef SP_FEMERGENODE_H_SEEN
#define SP_FEMERGENODE_H_SEEN

/** \file
 * feMergeNode implementation. A feMergeNode stores information about one
 * input image for feMerge filter primitive.
 */
/*
 * Authors:
 *   Kees Cook <kees@outflux.net>
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2004,2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-object.h"

#define SP_TYPE_FEMERGENODE (sp_feMergeNode_get_type())
#define SP_FEMERGENODE(o) (G_TYPE_CHECK_INSTANCE_CAST((o), SP_TYPE_FEMERGENODE, SPFeMergeNode))
#define SP_IS_FEMERGENODE(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), SP_TYPE_FEMERGENODE))

class SPFeMergeNode;
class SPFeMergeNodeClass;

struct SPFeMergeNode : public SPObject {
    int input;
};

struct SPFeMergeNodeClass {
    SPObjectClass parent_class;
};

GType sp_feMergeNode_get_type();


#endif /* !SP_FEMERGENODE_H_SEEN */

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
