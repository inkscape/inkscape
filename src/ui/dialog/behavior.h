/** @file
 * @brief Dialog behavior interface
 */
/* Author:
 *   Gustav Broberg <broberg@kth.se>
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_DIALOG_BEHAVIOR_H
#define INKSCAPE_UI_DIALOG_BEHAVIOR_H

#include <gtkmm/button.h>
#include <gtkmm/box.h>

class SPDesktop;

namespace Inkscape {
namespace UI {
namespace Dialog {

class Dialog;

namespace Behavior {

class Behavior;

typedef Behavior *(*BehaviorFactory)(Dialog &dialog);

template <typename T>
Behavior *create(Dialog &dialog)
{
    return T::create(dialog);
}


class Behavior {

public:
    virtual ~Behavior() { }

    /** Gtk::Dialog methods */
    virtual operator Gtk::Widget&() =0;
    virtual GtkWidget *gobj() =0;
    virtual void present() =0;
    virtual Gtk::Box *get_vbox() =0;
    virtual void show() =0;
    virtual void hide() =0;
    virtual void show_all_children() =0;
    virtual void resize(int width, int height) =0;
    virtual void move(int x, int y) =0;
    virtual void set_position(Gtk::WindowPosition) =0;
    virtual void set_size_request(int width, int height) =0;
    virtual void size_request(Gtk::Requisition &requisition) =0;
    virtual void get_position(int &x, int &y) =0;
    virtual void get_size(int &width, int &height) =0;
    virtual void set_title(Glib::ustring title) =0;
    virtual void set_sensitive(bool sensitive) =0;

    /** Gtk::Dialog signal proxies */
    virtual Glib::SignalProxy0<void> signal_show() =0;
    virtual Glib::SignalProxy0<void> signal_hide() =0;
    virtual Glib::SignalProxy1<bool, GdkEventAny *> signal_delete_event() =0;

    /** Custom signal handlers */
    virtual void onHideF12() =0;
    virtual void onShowF12() =0;
    virtual void onShutdown() =0;
    virtual void onDesktopActivated(SPDesktop *desktop) =0;

protected:
    Behavior(Dialog &dialog)
        : _dialog (dialog)
    { }
        
    Dialog& _dialog;  //< reference to the owner

private:
    Behavior(); // no constructor without params
    Behavior(const Behavior &);            // no copy
    Behavior &operator=(const Behavior &); // no assign
};

} // namespace Behavior
} // namespace Dialog
} // namespace UI
} // namespace Inkscape


#endif //INKSCAPE_UI_DIALOG_BEHAVIOR_H

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
