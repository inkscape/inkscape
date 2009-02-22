/** @file
 * @brief Base class for dialogs in Inkscape
 */
/* Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Gustav Broberg <broberg@kth.se>
 *
 * Copyright (C) 2004--2007 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_DIALOG_H
#define INKSCAPE_DIALOG_H

#include <gtkmm/dialog.h>
#include <gtkmm/tooltips.h>

#include "dock-behavior.h"
#include "floating-behavior.h"

class SPDesktop;

namespace Inkscape { 
class Selection; 
class Application; 
}

namespace Inkscape {
namespace UI {
namespace Dialog {

enum BehaviorType { FLOATING, DOCK };

void sp_retransientize(Inkscape::Application *inkscape, SPDesktop *desktop, gpointer dlgPtr);
gboolean sp_retransientize_again(gpointer dlgPtr);
void sp_dialog_shutdown(GtkObject *object, gpointer dlgPtr);

/** 
 * @brief Base class for Inkscape dialogs
 * This class provides certain common behaviors and styles wanted of all dialogs
 * in the application.  Fundamental parts of the dialog's behavior are controlled by
 * a Dialog::Behavior subclass instance connected to the dialog.
 */
class Dialog {

public:

    Dialog(Behavior::BehaviorFactory behavior_factory, const char *prefs_path = NULL, 
           int verb_num = 0, Glib::ustring const &apply_label = "");

    virtual ~Dialog();

    virtual void onDesktopActivated(SPDesktop*);
    virtual void onShutdown();

    /** Hide and show dialogs */
    virtual void onHideF12();
    virtual void onShowF12();

    virtual operator Gtk::Widget &();
    virtual GtkWidget *gobj();
    virtual void present();
    virtual Gtk::VBox *get_vbox();
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

    void           read_geometry();
    void           save_geometry();

    bool retransientize_suppress; // when true, do not retransientize (prevents races when switching new windows too fast)

protected:
    Glib::ustring const _prefs_path;
    int            _verb_num;
    Glib::ustring  _title;
    Glib::ustring  _apply_label;

    /**
     * Tooltips object for all descendants to use
     */
    Gtk::Tooltips tooltips;

    virtual void   _handleResponse(int response_id);

    virtual bool   _onDeleteEvent (GdkEventAny*);
    virtual bool   _onEvent(GdkEvent *event);
    virtual bool   _onKeyPress(GdkEventKey *event);

    virtual void   _apply();
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
