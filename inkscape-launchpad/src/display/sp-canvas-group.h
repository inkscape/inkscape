#ifndef SEEN_SP_CANVAS_GROUP_H
#define SEEN_SP_CANVAS_GROUP_H

/**
 * @file
 * SPCanvasGroup.
 */
/*
 * Authors:
 *   Federico Mena <federico@nuclecu.unam.mx>
 *   Raph Levien <raph@gimp.org>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 1998 The Free Software Foundation
 * Copyright (C) 2002 Lauris Kaplinski
 * Copyright (C) 2010 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glib-object.h>

#define SP_TYPE_CANVAS_GROUP (sp_canvas_group_get_type())
#define SP_CANVAS_GROUP(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_CANVAS_GROUP, SPCanvasGroup))
#define SP_IS_CANVAS_GROUP(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_CANVAS_GROUP))

GType sp_canvas_group_get_type();



#endif // SEEN_SP_CANVAS_GROUP_H

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
