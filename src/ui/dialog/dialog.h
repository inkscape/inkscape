/** @file
 * @brief Base class for dialogs in Inkscape
 */
/* Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Gustav Broberg <broberg@kth.se>
 *   Kris De Gussem <Kris.DeGussem@gmail.com>
 *
 * Copyright (C) 2004--2007 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_DIALOG_H
#define INKSCAPE_DIALOG_H

#include "dock-behavior.h"
#include "floating-behavior.h"

class SPDesktop;
struct InkscapeApplication;

namespace Inkscape {
class Selection;
}

namespace Inkscape {
namespace UI {
namespace Dialog {

enum BehaviorType { FLOATING, DOCK };

gboolean sp_retransientize_again(gpointer dlgPtr);
void sp_dialog_shutdown(GObject *object, gpointer dlgPtr);

/**
 * Base class for Inkscape dialogs.
 * 
 * UI::Dialog::Dialog is a base class for all dialogs in Inkscape.  The
 * purpose of this class is to provide a unified place for ensuring
 * style and behavior. Specifically, this class provides functionality
 * for saving and restoring the size and position of dialogs (through
 * the user's preferences file).
 *
 * It also provides some general purpose signal handlers for things like
 * showing and hiding all dialogs.
 *
 * Fundamental parts of the dialog's behavior are controlled by
 * a UI::Dialog::Behavior subclass instance connected to the dialog.
 *
 * @see UI::Widget::Panel panel class from which the dialogs are actually derived from.
 * @see UI::Dialog::DialogManager manages the dialogs within inkscape.
 * @see UI::Dialog::PanelDialog which links Panel and Dialog together in a dockable and floatable dialog.
 */
class Dialog {

public:

    /**
     * Constructor.
     * 
     * @param behavior_factory floating or docked.
     * @param prefs_path characteristic path for loading/saving dialog position.
     * @param verb_num the dialog verb.
     */
    Dialog(Behavior::BehaviorFactory behavior_factory, const char *prefs_path = NULL,
           int verb_num = 0, Glib::ustring const &apply_label = "");

    virtual ~Dialog();

    virtual void onDesktopActivated(SPDesktop*);
    virtual void onShutdown();

    /* Hide and show dialogs */
    virtual void onHideF12();
    virtual void onShowF12();

    virtual operator Gtk::Widget &();
    virtual GtkWidget *gobj();
    virtual void present();
    virtual Gtk::Box *get_vbox();
    virtual void show();
    virtual void hide();
    virtual void show_all_children();
    virtual void set_size_request(int, int);
    virtual void size_request(Gtk::Requisition &);
    virtual void get_position(int &x, int &y);
    virtual void get_size(int &width, int &height);
    virtual void resize(int width, int height);
    virtual void move(int x, int y);
    virtual void set_position(Gtk::WindowPosition position);
    virtual void set_title(Glib::ustring title);
    virtual void set_sensitive(bool sensitive=true);

    virtual Glib::SignalProxy0<void> signal_show();
    virtual Glib::SignalProxy0<void> signal_hide();

    bool           _user_hidden; // when it is closed by the user, to prevent repopping on f12
    bool           _hiddenF12;

    /**
     * Read window position from preferences.
     */
    void           read_geometry();
    
    /**
     * Save window position to preferences.
     */
    void           save_geometry();
    void           save_status(int visible, int state, int placement);

    bool retransientize_suppress; // when true, do not retransientize (prevents races when switching new windows too fast)

protected:
    Glib::ustring const _prefs_path;
    int            _verb_num;
    Glib::ustring  _title;
    Glib::ustring  _apply_label;
    SPDesktop *    _desktop;
    bool           _is_active_desktop;

    virtual void   _handleResponse(int response_id);

    virtual bool   _onDeleteEvent (GdkEventAny*);
    virtual bool   _onEvent(GdkEvent *event);
    virtual bool   _onKeyPress(GdkEventKey *event);

    virtual void   _apply();
    
    /* Closes the dialog window.
     *
     * This code sends a delete_event to the dialog,
     * instead of just destroying it, so that the
     * dialog can do some housekeeping, such as remember
     * its position.
     */
    virtual void   _close();
    virtual void   _defocus();

    Inkscape::Selection*   _getSelection();

    sigc::connection _desktop_activated_connection;
    sigc::connection _dialogs_hidden_connection;
    sigc::connection _dialogs_unhidden_connection;
    sigc::connection _shutdown_connection;

private:
    Behavior::Behavior* _behavior;

    Dialog(); // no constructor without params

    Dialog(Dialog const &d);            // no copy
    Dialog& operator=(Dialog const &d); // no assign

    friend class Behavior::FloatingBehavior;
    friend class Behavior::DockBehavior;
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif //INKSCAPE_DIALOG_H

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
