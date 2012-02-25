/** @file
 * A simple dialog with information about extensions
 */
/* Authors:
 *   Jon A. Cruz
 *
 * Copyright (C) 2005 The Inkscape Organization
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
 
#ifndef SEEN_EXTENSIONS_H
#define SEEN_EXTENSIONS_H

#include "ui/widget/panel.h"
#include <gtkmm/textview.h>

namespace Inkscape {
namespace Extension {
class Extension;
}
}

namespace Inkscape {
namespace UI {
namespace Dialogs {


/**
 * A panel that displays information about extensions.
 */
class ExtensionsPanel : public Inkscape::UI::Widget::Panel
{
public:
    ExtensionsPanel();

    static ExtensionsPanel &getInstance();

    void set_full(bool full);

private:
    ExtensionsPanel(ExtensionsPanel const &); // no copy
    ExtensionsPanel &operator=(ExtensionsPanel const &); // no assign

    static void listCB(Inkscape::Extension::Extension *in_plug, gpointer in_data);

    void rescan();

    bool _showAll;
    Gtk::TextView _view;
};

} //namespace Dialogs
} //namespace UI
} //namespace Inkscape

#endif // SEEN_EXTENSIONS_H
