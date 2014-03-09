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

namespace Inkscape {

namespace UI {
namespace View {
class View;
} // namespace View
} // namespace UI

namespace Extension {

class Effect;

namespace  Implementation
{
class ImplementationDocumentCache;
}

class ExecutionEnv {
private:
    enum state_t {
        INIT,     //< The context has been initialized
        COMPLETE, //< We've completed atleast once
        RUNNING   //< The effect is currently running
    };
    /** \brief  What state the execution engine is in. */
    state_t _state;

    /** \brief If there is a working dialog it'll be referenced
               right here. */
    Gtk::Dialog * _visibleDialog;
    /** \brief Signal that the run is complete. */
    sigc::signal<void> _runComplete;
    /** \brief  In some cases we need a mainLoop, when we do, this is
                a pointer to it. */
    Glib::RefPtr<Glib::MainLoop> _mainloop;
    /** \brief  The document that we're working on. */
    Inkscape::UI::View::View * _doc;
    /** \brief  A list of the IDs of all the selected objects before
                we started to work on this document. */
    std::list<Glib::ustring> _selected;
    /** \brief  A document cache if we were passed one. */
    Implementation::ImplementationDocumentCache * _docCache;

    /** \brief  The effect that we're executing in this context. */
    Effect * _effect;

    /** \brief  Show the working dialog when the effect is executing. */
    bool _show_working;
    /** \brief  Display errors if they occur. */
    bool _show_errors;
public:

    /** \brief  Create a new context for exection of an effect
        \param effect  The effect to execute
        \param doc     The document to execute the effect on
        \param docCache  The implementation cache of the document.  May be
                         NULL in which case it'll be created by the execution
                         environment.
        \prarm show_working  Show a small dialog signaling the effect
                             is working.  Allows for user canceling.
        \param show_errors   If the effect has an error, show it or not.
    */
    ExecutionEnv (Effect * effect,
                  Inkscape::UI::View::View * doc,
                  Implementation::ImplementationDocumentCache * docCache = NULL,
                  bool show_working = true,
                  bool show_errors = true);
    virtual ~ExecutionEnv (void);

    /** \brief Starts the execution of the effect
        \return Returns whether the effect was executed to completion */
    void run (void);
    /** \brief Cancel the execution of the effect */
    void cancel (void);
    /** \brief Commit the changes to the document */
    void commit (void);
    /** \brief Undoes what the effect completed. */
    void undo (void);
    /** \brief Wait for the effect to complete if it hasn't. */
    bool wait (void);

private:
    void runComplete (void);
    void createPrefsDialog (Gtk::Widget * controls);
    void createWorkingDialog (void);
    void workingCanceled (const int resp);
    void processingCancel (void);
    void processingComplete(void);
    void documentCancel (void);
    void documentCommit (void);
    void reselect (void);
    void genDocCache (void);
    void killDocCache (void);
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
