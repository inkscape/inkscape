/** @file
 * @brief Messages dialog
 *
 * A very simple dialog for displaying Inkscape messages. Messages
 * sent to g_log(), g_warning(), g_message(), ets, are routed here,
 * in order to avoid messing with the startup console.
 */
/* Authors:
 *   Bob Jamison
 *   Other dudes from The Inkscape Organization
 *
 * Copyright (C) 2004, 2005 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_UI_DIALOG_MESSAGES_H
#define INKSCAPE_UI_DIALOG_MESSAGES_H

#include <gtkmm/box.h>
#include <gtkmm/textview.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/menubar.h>
#include <gtkmm/menu.h>
#include <gtkmm/scrolledwindow.h>

#include <glibmm/i18n.h>

#include "ui/widget/panel.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

class Messages : public UI::Widget::Panel {
public:
    Messages();
    virtual ~Messages();

    static Messages &getInstance() { return *new Messages(); }

    /**
     * Clear all information from the dialog
     */
    void clear();

    /**
     * Display a message
     */
    void message(char *msg);

    /**
     * Redirect g_log() messages to this widget
     */
    void captureLogMessages();

    /**
     * Return g_log() messages to normal handling
     */
    void releaseLogMessages();

    void toggleCapture();

protected:
    //Gtk::MenuBar        menuBar;
    //Gtk::Menu           fileMenu;
    Gtk::ScrolledWindow textScroll;
    Gtk::TextView       messageText;
    Gtk::HBox           buttonBox;
    Gtk::Button         buttonClear;
    Gtk::CheckButton    checkCapture;

    //Handler ID's
    guint handlerDefault;
    guint handlerGlibmm;
    guint handlerAtkmm;
    guint handlerPangomm;
    guint handlerGdkmm;
    guint handlerGtkmm;

private:
    Messages(Messages const &d);
    Messages operator=(Messages const &d);
};


} //namespace Dialog
} //namespace UI
} //namespace Inkscape

#endif // INKSCAPE_UI_DIALOG_MESSAGES_H

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
