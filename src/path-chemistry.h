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

void sp_selected_path_combine (SPDesktop *desktop);
void sp_selected_path_break_apart (SPDesktop *desktop);
void sp_selected_path_to_curves (SPDesktop *desktop, bool interactive = true);
void sp_selected_to_lpeitems(SPDesktop *desktop);
Inkscape::XML::Node *sp_selected_item_to_curved_repr(SPItem *item, guint32 text_grouping_policy);
void sp_selected_path_reverse (SPDesktop *desktop);
bool sp_item_list_to_curves(const GSList *items, GSList **selected, GSList **to_select, bool skip_all_lpeitems = false);

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
