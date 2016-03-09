#ifndef SEEN_PATH_CHEMISTRY_H
#define SEEN_PATH_CHEMISTRY_H

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

class SPDesktop;
class SPItem;

namespace Inkscape {
class Selection;
namespace XML {
class Node;
} // namespace XML
} // namespace Inkscape

typedef unsigned int guint32;

void sp_selected_path_combine (SPDesktop *desktop, bool skip_undo = false);
void sp_selected_path_break_apart (SPDesktop *desktop, bool skip_undo = false);
// interactive=true only has an effect if desktop != NULL, i.e. if a GUI is available
void sp_selected_path_to_curves (Inkscape::Selection *selection, SPDesktop *desktop, bool interactive = true);
void sp_selected_to_lpeitems(SPDesktop *desktop);
Inkscape::XML::Node *sp_selected_item_to_curved_repr(SPItem *item, guint32 text_grouping_policy);
void sp_selected_path_reverse (SPDesktop *desktop);
bool sp_item_list_to_curves(const std::vector<SPItem*> &items, std::vector<SPItem*> &selected, std::vector<Inkscape::XML::Node*> &to_select, bool skip_all_lpeitems = false);

#endif // SEEN_PATH_CHEMISTRY_H

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
