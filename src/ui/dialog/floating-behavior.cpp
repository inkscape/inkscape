/**
 * \brief A floating dialog implementation.
 *
 * Author:
 *   Gustav Broberg <broberg@kth.se>
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#include <gtkmm/stock.h>
#include <gtk/gtk.h>

#include "floating-behavior.h"
#include "dialog.h"

#include "application/application.h"
#include "application/editor.h"
#include "inkscape.h"
#include "desktop.h"
#include "dialogs/dialog-events.h"
#include "interface.h"
#include "prefs-utils.h"
#include "verbs.h"

namespace Inkscape {
namespace UI {
namespace Dialog {
namespace Behavior {

FloatingBehavior::FloatingBehavior(Dialog &dialog) :
    Behavior(dialog),
    _d (new Gtk::Dialog(_dialog._title))
#if GTK_VERSION_GE(2, 12)
	,_dialog_active(_d->property_is_active())
	,_steps(0)
	,_trans_focus(prefs_get_double_attribute_limited("dialogs.transparency", "on-focus", 0.95, 0.0, 1.0))
	,_trans_blur(prefs_get_double_attribute_limited("dialogs.transparency", "on-blur", 0.50, 0.0, 1.0))
	,_trans_time(prefs_get_int_attribute_limited("dialogs.transparency", "animate-time", 100, 0, 5000))
#endif
{
    hide();
    _d->set_has_separator(false);

    signal_delete_event().connect(sigc::mem_fun(_dialog, &Inkscape::UI::Dialog::Dialog::_onDeleteEvent));

    sp_transientize(GTK_WIDGET(_d->gobj()));
    _dialog.retransientize_suppress = false;

#if GTK_VERSION_GE(2, 12)
	_focus_event();
	_dialog_active.signal_changed().connect(sigc::mem_fun(this, &FloatingBehavior::_focus_event));
#endif

}

#if GTK_VERSION_GE(2, 12)
/** \brief  A function called when the window gets focus

	This function gets called on a focus event.  It figures out how much
	time is required for a transition, and the number of steps that'll take,
	and sets up the _trans_timer function to do the work.  If the transition
	time is set to 0 ms it just calls _trans_timer once with _steps equal to
	zero so that the transition happens instantaneously.  This occurs on
	windows as opacity changes cause flicker there.
*/
void FloatingBehavior::_focus_event (void) {
	_steps = 0;
	_trans_focus = prefs_get_double_attribute_limited("dialogs.transparency", "on-focus", 0.95, 0.0, 1.0);
	_trans_blur = prefs_get_double_attribute_limited("dialogs.transparency", "on-blur", 0.50, 0.0, 1.0);
	_trans_time = prefs_get_int_attribute_limited("dialogs.transparency", "animate-time", 100, 0, 5000);

	if (_trans_time != 0) {
		float diff = _trans_focus - _trans_blur;
		if (diff < 0.0) diff *= -1.0;

		while (diff > 0.05) {
			_steps++;
			diff = diff / 2.0;
		}

		if (_steps != 0) {
			Glib::signal_timeout().connect(sigc::mem_fun(this, &FloatingBehavior::_trans_timer), _trans_time / _steps);
		}
	}
	_trans_timer();

	return;
}

/** \brief  Move the opacity of a window towards our goal

	This is a timer function that is set up by _focus_event to slightly
	move the opacity of the window along in an animated fashion.  It moves
	the opacity half way to the goal until it runs out of steps, and then
	it just forces the goal.
*/
bool FloatingBehavior::_trans_timer (void) {
	// printf("Go go gadget timer: %d\n", _steps);
	if (_steps == 0) {
		if (_dialog_active.get_value()) {
			_d->set_opacity(_trans_focus);
		} else {
			_d->set_opacity(_trans_blur);
		}

		return false;
	}

	float goal, current;
	goal = current = _d->get_opacity();

	if (_dialog_active.get_value()) {
		goal = _trans_focus;
	} else {
		goal = _trans_blur;
	}
	
	_d->set_opacity(current - ((current - goal) / 2));
	_steps--;
	return true;
}

#endif

FloatingBehavior::~FloatingBehavior() 
{ 
    delete _d;
}

Behavior *
FloatingBehavior::create(Dialog &dialog)
{
    return new FloatingBehavior(dialog);
}

inline FloatingBehavior::operator Gtk::Widget &()                          { return *_d; }
inline GtkWidget *FloatingBehavior::gobj()                                { return GTK_WIDGET(_d->gobj()); }
inline Gtk::VBox* FloatingBehavior::get_vbox()                            { return _d->get_vbox(); }
inline void FloatingBehavior::present()                                   { _d->present(); }
inline void FloatingBehavior::hide()                                      { _d->hide(); }
inline void FloatingBehavior::show()                                      { _d->show(); }
inline void FloatingBehavior::show_all_children()                         { _d->show_all_children(); }
inline void FloatingBehavior::resize(int width, int height)               { _d->resize(width, height); }
inline void FloatingBehavior::move(int x, int y)                          { _d->move(x, y); }
inline void FloatingBehavior::set_position(Gtk::WindowPosition position)  { _d->set_position(position); }
inline void FloatingBehavior::set_size_request(int width, int height)     { _d->set_size_request(width, height); }
inline void FloatingBehavior::size_request(Gtk::Requisition &requisition) { _d->size_request(requisition); }
inline void FloatingBehavior::get_position(int &x, int &y)                { _d->get_position(x, y); }
inline void FloatingBehavior::get_size(int &width, int &height)           { _d->get_size(width, height); }
inline void FloatingBehavior::set_title(Glib::ustring title)              { _d->set_title(title); }
inline void FloatingBehavior::set_sensitive(bool sensitive)               { _d->set_sensitive(sensitive); }

Glib::SignalProxy0<void> FloatingBehavior::signal_show() { return _d->signal_show(); }
Glib::SignalProxy0<void> FloatingBehavior::signal_hide() { return _d->signal_hide(); }
Glib::SignalProxy1<bool, GdkEventAny *> FloatingBehavior::signal_delete_event () { return _d->signal_delete_event(); }


void
FloatingBehavior::onHideF12()
{
    _dialog.save_geometry();
    hide();
}

void
FloatingBehavior::onShowF12()
{
    show();
    _dialog.read_geometry();
}

void
FloatingBehavior::onShutdown() {}

void
FloatingBehavior::onDesktopActivated (SPDesktop *desktop)
{
    gint transient_policy = prefs_get_int_attribute_limited ( "options.transientpolicy", "value", 1, 0, 2);

#ifdef WIN32 // Win32 special code to enable transient dialogs
    transient_policy = 2;
#endif        

    if (!transient_policy) 
        return;

    GtkWindow *dialog_win = GTK_WINDOW(_d->gobj());

    if (_dialog.retransientize_suppress) {
         /* if retransientizing of this dialog is still forbidden after
          * previous call warning turned off because it was confusingly fired
          * when loading many files from command line
          */

         // g_warning("Retranzientize aborted! You're switching windows too fast!");
        return;
    }

    if (dialog_win)
    {
        _dialog.retransientize_suppress = true; // disallow other attempts to retranzientize this dialog

        desktop->setWindowTransient (dialog_win);

        /*
         * This enables "aggressive" transientization,
         * i.e. dialogs always emerging on top when you switch documents. Note
         * however that this breaks "click to raise" policy of a window
         * manager because the switched-to document will be raised at once
         * (so that its transients also could raise)
         */
        if (transient_policy == 2 && ! _dialog._hiddenF12 && !_dialog._user_hidden) {
            // without this, a transient window not always emerges on top
            gtk_window_present (dialog_win);
        }
    }

    // we're done, allow next retransientizing not sooner than after 120 msec
    gtk_timeout_add (120, (GtkFunction) sp_retransientize_again, (gpointer) _d);
}


} // namespace Behavior
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
