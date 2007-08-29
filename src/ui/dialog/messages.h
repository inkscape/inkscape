/*
 * \brief Messages Dialog
 *
 * A very simple dialog for displaying Inkscape messages. Messages
 * sent to g_log(), g_warning(), g_message(), ets, are routed here,
 * in order to avoid messing with the startup console.
 *
 * Authors:
 *   Bob Jamison
 *   Other dudes from The Inkscape Organization
 *
 * Copyright (C) 2004, 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_UI_DIALOG_MESSAGES_H
#define INKSCAPE_UI_DIALOG_MESSAGES_H

#include <gtkmm/box.h>
#include <gtkmm/dialog.h>
#include <gtkmm/textview.h>
#include <gtkmm/button.h>
#include <gtkmm/menubar.h>
#include <gtkmm/menu.h>
#include <gtkmm/scrolledwindow.h>

#include <glibmm/i18n.h>

#include "dialog.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

class Messages : public Dialog {
public:
    Messages(Behavior::BehaviorFactory behavior_factory);
    virtual ~Messages();

    static Messages *create(Behavior::BehaviorFactory behavior_factory) 
    { return new Messages(behavior_factory); }

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

protected:
    Gtk::MenuBar        menuBar;
    Gtk::Menu           fileMenu;
    Gtk::ScrolledWindow textScroll;
    Gtk::TextView       messageText;

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
