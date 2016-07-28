/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ui/dialog/extensions.h"

#include <glibmm/i18n.h>
#include "inkscape.h"
#include "preferences.h"
#include "extension/extension.h"

#include "error-file.h"

/** The name and group of the preference to say whether the error
    dialog should be shown on startup. */
#define PREFERENCE_ID  "/dialogs/extension-error/show-on-startup"

namespace Inkscape {
namespace Extension {

/** \brief  An initializer which builds the dialog

    Really a simple function.  Basically the message dialog itself gets
    built with the first initializer.  The next step is to add in the
    message, and attach the filename for the error file.  After that
    the checkbox is built, and has the call back attached to it.  Also,
    it is set based on the preferences setting for show on startup (really,
    it should always be checked if you can see the dialog, but it is
    probably good to check anyway).
*/
ErrorFileNotice::ErrorFileNotice (void) :
    Gtk::MessageDialog(
            "",                    /* message */
            false,                 /* use markup */
            Gtk::MESSAGE_WARNING,  /* dialog type */
            Gtk::BUTTONS_OK,       /* buttons */
            true                   /* modal */
        )

{
    // \FIXME change this
    /* This is some filler text, needs to change before relase */
    Glib::ustring dialog_text(_("<span weight=\"bold\" size=\"larger\">One or more extensions failed to load</span>\n\nThe failed extensions have been skipped.  Inkscape will continue to run normally but those extensions will be unavailable.  For details to troubleshoot this problem, please refer to the error log located at: "));
    gchar * ext_error_file = Inkscape::Application::profile_path(EXTENSION_ERROR_LOG_FILENAME);
    dialog_text += ext_error_file;
    g_free(ext_error_file);
    set_message(dialog_text, true);

#if WITH_GTKMM_3_0
    Gtk::Box * vbox = get_content_area();
#else
    Gtk::Box * vbox = get_vbox();
#endif

    /* This is some filler text, needs to change before relase */
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    checkbutton = Gtk::manage(new Gtk::CheckButton(_("Show dialog on startup")));
    vbox->pack_start(*checkbutton, true, false, 5);
    checkbutton->show();
    checkbutton->set_active(prefs->getBool(PREFERENCE_ID, true));

    checkbutton->signal_toggled().connect(sigc::mem_fun(this, &ErrorFileNotice::checkbox_toggle));

    set_resizable(true);

    Inkscape::UI::Dialogs::ExtensionsPanel* extens = new Inkscape::UI::Dialogs::ExtensionsPanel();
    extens->set_full(false);
    vbox->pack_start( *extens, true, true );
    extens->show();

    return;
}

/** \brief Sets the preferences based on the checkbox value */
void
ErrorFileNotice::checkbox_toggle (void)
{
    // std::cout << "Toggle value" << std::endl;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setBool(PREFERENCE_ID, checkbutton->get_active());
}

/** \brief Shows the dialog

    This function only shows the dialog if the preferences say that the
    user wants to see the dialog, otherwise it just exits.
*/
int
ErrorFileNotice::run (void)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (!prefs->getBool(PREFERENCE_ID, true))
        return 0;
    return Gtk::Dialog::run();
}

}; };  /* namespace Inkscape, Extension */

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
