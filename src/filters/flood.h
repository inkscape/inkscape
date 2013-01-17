/** @file
 * @brief SVG flood filter effect
 *//*
 * Authors:
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *
 * Copyright (C) 2006 Hugo Rodrigues
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SP_FEFLOOD_H_SEEN
#define SP_FEFLOOD_H_SEEN

#include "sp-filter-primitive.h"
#include "svg/svg-icc-color.h"

G_BEGIN_DECLS

#define SP_TYPE_FEFLOOD            (sp_feFlood_get_type())
#define SP_FEFLOOD(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_FEFLOOD, SPFeFlood))
#define SP_FEFLOOD_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_FEFLOOD, SPFeFloodClass))
#define SP_IS_FEFLOOD(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_FEFLOOD))
#define SP_IS_FEFLOOD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_FEFLOOD))

class SPFeFloodClass;

struct SPFeFlood : public SPFilterPrimitive {
    guint32 color;
    SVGICCColor *icc;
    double opacity;
};

struct SPFeFloodClass {
    SPFilterPrimitiveClass parent_class;
};

GType sp_feFlood_get_type() G_GNUC_CONST;

G_END_DECLS
#endif /* !SP_FEFLOOD_H_SEEN */

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
