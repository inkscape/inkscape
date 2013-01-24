#ifndef SEEN_SP_DEFS_H
#define SEEN_SP_DEFS_H

/*
 * SVG <defs> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Abhishek Sharma
 *
 * Copyright (C) 2000-2002 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-object.h"

#define SP_TYPE_DEFS            (sp_defs_get_type())
#define SP_DEFS(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_DEFS, SPDefs))
#define SP_DEFS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_DEFS, SPDefsClass))
#define SP_IS_DEFS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_DEFS))
#define SP_IS_DEFS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_DEFS))

GType sp_defs_get_type(void) G_GNUC_CONST;

class SPDefs : public SPObject {
private:
    friend class SPDefsClass;	
};

class SPDefsClass {
public:
    SPObjectClass parent_class;

private:
    friend class SPDefs;	
};


#endif // !SEEN_SP_DEFS_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
