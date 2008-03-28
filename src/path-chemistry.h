#ifndef __PATH_CHEMISTRY_H__
#define __PATH_CHEMISTRY_H__

/*
 * Here are handlers for modifying selections, specific to paths
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "forward.h"

void sp_selected_path_combine (void);
void sp_selected_path_break_apart (void);
void sp_selected_path_to_curves (bool interactive = true);
Inkscape::XML::Node *sp_selected_item_to_curved_repr(SPItem *item, guint32 text_grouping_policy);
void sp_selected_path_reverse ();

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
