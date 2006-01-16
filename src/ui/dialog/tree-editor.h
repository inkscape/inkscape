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

#ifndef INKSCAPE_DIALOG_TREE_EDITOR_H
#define INKSCAPE_DIALOG_TREE_EDITOR_H

#include <gtkmm/box.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>
#include <glibmm/i18n.h>

#include "dialog.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

class TreeEditor : public Dialog {
public:
    TreeEditor();
    virtual ~TreeEditor();

protected:

    Gtk::HBox             _hbox;
    Gtk::ScrolledWindow   _leftWin;
    Gtk::TreeView         _leftTree;

    // TODO:  Add the tree model
    // Glib::RefPtr<ExampleTreeModel> _refTreeModel;

private:
    TreeEditor(TreeEditor const &d);
    TreeEditor& operator=(TreeEditor const &d);
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_DIALOG_TREE_EDITOR_H

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
