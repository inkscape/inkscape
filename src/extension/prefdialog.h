/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_EXTENSION_DIALOG_H__
#define INKSCAPE_EXTENSION_DIALOG_H__

#include <glibmm/ustring.h>

#include <gdkmm/types.h>

#include <gtkmm/dialog.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/socket.h>

#include "execution-env.h"
#include "parameter.h"

namespace Inkscape {
namespace Extension {

/** \brief  A class to represent the preferences for an extension */
class PrefDialog : public Gtk::Dialog {
    /** \brief  Help string if it exists */
    gchar const * _help;
    /** \brief  Name of the extension */
    Glib::ustring _name;
    /** \brief  An execution environment if there is one */
    ExecutionEnv * _exEnv;

    /** \brief  A pointer to the OK button */
    Gtk::Button * _button_ok;
    /** \brief  A pointer to the CANCEL button */
    Gtk::Button * _button_cancel;

    /** \brief  Button to control live preview */
    Gtk::Widget * _button_preview;
    /** \brief  Button to control whether the dialog is pinned */
    Gtk::Widget * _button_pinned;

    /** \brief  Parameter to control live preview */
    Parameter * _param_preview;
    /** \brief  Parameter to control pinning the dialog */
    Parameter * _param_pinned;

    /** \brief  XML to define the pinned parameter on the dialog */
    static const char * pinned_param_xml;
    /** \brief  XML to define the live effects parameter on the dialog */
    static const char * live_param_xml;

    /** \brief Signal that the user is changing the live effect state */
    sigc::signal<void> _signal_preview;
    /** \brief Signal that the user is changing the pinned state */
    sigc::signal<void> _signal_pinned;

    Effect * _effect;

    void preview_toggle(void);
    void pinned_toggle(void);

    void on_response (int signal);

public:
    PrefDialog (Glib::ustring name,
                gchar const * help,
                Gtk::Widget * controls,
                ExecutionEnv * exEnv = NULL,
                Effect * effect = NULL);
    ~PrefDialog ();
    int run (void);

    void setPreviewState (Glib::ustring state);
};


};}; /* namespace Inkscape, Extension */

#endif /* INKSCAPE_EXTENSION_DIALOG_H__ */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
