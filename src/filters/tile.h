/** @file
 * @brief SVG tile filter effect
 *//*
 * Authors:
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *
 * Copyright (C) 2006 Hugo Rodrigues
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SP_FETILE_H_SEEN
#define SP_FETILE_H_SEEN

#include "sp-filter-primitive.h"

#define SP_TYPE_FETILE (sp_feTile_get_type())
#define SP_FETILE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_FETILE, SPFeTile))
#define SP_FETILE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_FETILE, SPFeTileClass))
#define SP_IS_FETILE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_FETILE))
#define SP_IS_FETILE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_FETILE))

/* FeTile base class */
class SPFeTileClass;

struct SPFeTile : public SPFilterPrimitive {
    
};

struct SPFeTileClass {
    SPFilterPrimitiveClass parent_class;
};

GType sp_feTile_get_type();

#endif /* !SP_FETILE_H_SEEN */

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
