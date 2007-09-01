/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_EXTENSION_EXECUTION_ENV_H__
#define INKSCAPE_EXTENSION_EXECUTION_ENV_H__

#include <config.h>

#include <glibmm/main.h>
#include <glibmm/ustring.h>

#include <gtkmm/dialog.h>

#include "forward.h"
#include "extension-forward.h"

namespace Inkscape {
namespace Extension {

class ExecutionEnv {
private:
    Gtk::Dialog * _visibleDialog;
    bool _prefsVisible;
    bool _finished;
    bool _humanWait;
    bool _canceled;
    bool _prefsChanged;
    bool _livePreview;
    Glib::RefPtr<Glib::MainLoop> _mainloop;
    Inkscape::UI::View::View * _doc;
    std::list<Glib::ustring> _selected;

public:
    Effect * _effect;

    ExecutionEnv (Effect * effect, Inkscape::UI::View::View * doc, Gtk::Widget * controls = NULL, Gtk::Dialog * prefDialog = NULL);
    ~ExecutionEnv (void);

    void run (void);
    void preferencesChange (void);
    void livePreview (bool state = true);
    void shutdown (void);

private:
    void createPrefsDialog (Gtk::Widget * controls);
    void createWorkingDialog (void);
    void workingCanceled (const int resp);
    void preferencesResponse (const int resp);
    void processingCancel (void);
    void processingComplete(void);
    void documentCancel (void);
    void documentCommit (void);
    void reselect (void);
};

} }  /* namespace Inkscape, Extension */
#endif /* INKSCAPE_EXTENSION_EXECUTION_ENV_H__ */

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
