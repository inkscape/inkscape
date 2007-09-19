/*
 * \brief  Tree Editor - Abstract base class for dialogs that allow
 *         editing properties of tree-organized data.
 *
 * Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 2004, 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "tree-editor.h"
#include "verbs.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

TreeEditor::TreeEditor(Behavior::BehaviorFactory behavior_factory)
    : Dialog (behavior_factory, "dialogs.treeeditor", SP_VERB_NONE /*FIXME*/)
{
    get_vbox()->pack_start(_hbox);

    _hbox.add(_leftWin);
    _leftWin.add(_leftTree);

    // Only show the scrollbars when they are necessary
    _leftWin.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

    // TODO:  Create the tree model
    
    show_all_children();
}

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

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
