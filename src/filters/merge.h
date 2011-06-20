/** \file
 * SVG merge filter effect
 *//*
 * Authors:
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *
 * Copyright (C) 2006 Hugo Rodrigues
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifndef SP_FEMERGE_H_SEEN
#define SP_FEMERGE_H_SEEN

#include "sp-filter-primitive.h"

#define SP_TYPE_FEMERGE (sp_feMerge_get_type())
#define SP_FEMERGE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_FEMERGE, SPFeMerge))
#define SP_FEMERGE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_FEMERGE, SPFeMergeClass))
#define SP_IS_FEMERGE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_FEMERGE))
#define SP_IS_FEMERGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_FEMERGE))

class SPFeMergeClass;

struct SPFeMerge : public SPFilterPrimitive {
    
};

struct SPFeMergeClass {
    SPFilterPrimitiveClass parent_class;
};

GType sp_feMerge_get_type();


#endif /* !SP_FEMERGE_H_SEEN */

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
