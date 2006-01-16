#ifndef SP_SKELETON_H_SEEN
#define SP_SKELETON_H_SEEN

/** \file
 * SVG <skeleton> implementation, see sp-skeleton.cpp.
 */
/*
 * Authors:
 *   Kees Cook <kees@outflux.net>
 *
 * Copyright (C) 2004 Kees Cook
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-object.h"

/* Skeleton base class */

#define SP_TYPE_SKELETON (sp_skeleton_get_type())
#define SP_SKELETON(o) (G_TYPE_CHECK_INSTANCE_CAST((o), SP_TYPE_SKELETON, SPSkeleton))
#define SP_IS_SKELETON(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), SP_TYPE_SKELETON))

class SPSkeleton;
class SPSkeletonClass;

struct SPSkeleton : public SPObject {
};

struct SPSkeletonClass {
    SPObjectClass parent_class;
};

GType sp_skeleton_get_type();


#endif /* !SP_SKELETON_H_SEEN */

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
