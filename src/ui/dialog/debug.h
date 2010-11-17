/** @file
 * @brief Dialog for displaying Inkscape messages
 */
/* Authors:
 *   Bob Jamison
 *   Other dudes from The Inkscape Organization
 *
 * Copyright (C) 2004 The Inkscape Organization
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_UI_DIALOGS_DEBUGDIALOG_H
#define SEEN_UI_DIALOGS_DEBUGDIALOG_H

namespace Inkscape {
namespace UI {
namespace Dialog {


/**
 * @brief A very simple dialog for displaying Inkscape messages.
 *
 * Messages sent to g_log(), g_warning(), g_message(), ets, are routed here,
 * in order to avoid messing with the startup console.
 */
class DebugDialog
{
public:
    DebugDialog() {};
    /**
     * Factory method
     */
    static DebugDialog *create();

    /**
     * Destructor
     */
    virtual ~DebugDialog() {};


    /**
     * Show the dialog
     */
    virtual void show() = 0;

    /**
     * Do not show the dialog
     */
    virtual void hide() = 0;

    /**
     * @brief Clear all information from the dialog
	 *
	 * Also a public method.  Remove all text from the dialog
     */
    virtual void clear() = 0;

    /**
     * Display a message
     */
    virtual void message(char const *msg) = 0;

    /**
     * Redirect g_log() messages to this widget
     */
    virtual void captureLogMessages() = 0;

    /**
     * Return g_log() messages to normal handling
     */
    virtual void releaseLogMessages() = 0;

    /**
	 * Factory method.  Use this to create a new DebugDialog
	 */
    static DebugDialog *getInstance();

    /**
     * Show the instance above
     */
    static void showInstance();
};

} //namespace Dialogs
} //namespace UI
} //namespace Inkscape

#endif /* __DEBUGDIALOG_H__ */

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
