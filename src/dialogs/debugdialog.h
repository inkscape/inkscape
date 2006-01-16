#ifndef __DEBUGDIALOG_H__
#define __DEBUGDIALOG_H__
/*
 * A very simple dialog for displaying Inkscape messages. Messages
 * sent to g_log(), g_warning(), g_message(), ets, are routed here,
 * in order to avoid messing with the startup console.
 *
 * Authors:
 *   Bob Jamison
 *   Other dudes from The Inkscape Organization
 *
 * Copyright (C) 2004 The Inkscape Organization
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */



namespace Inkscape {
namespace UI {
namespace Dialogs {


/**
 * A dialog that displays log messages
 */
class DebugDialog
{

    public:
    

    /**
     * Constructor
     */
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
     * Clear all information from the dialog
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
     * Get a shared singleton instance
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

