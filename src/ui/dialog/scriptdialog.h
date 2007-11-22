#ifndef __SCRIPTDIALOG_H__
#define __SCRIPTDIALOG_H__
/*
 * This dialog is for launching scripts whose main purpose if
 * the scripting of Inkscape itself.
 *
 * Authors:
 *   Bob Jamison
 *   Other dudes from The Inkscape Organization
 *
 * Copyright (C) 2004, 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include "ui/widget/panel.h"
#include "verbs.h"

namespace Inkscape {
namespace UI {
namespace Dialog {


/**
 * A script editor, loader, and executor
 */
class ScriptDialog : public UI::Widget::Panel
{

    public:


    /**
     * Constructor
     */
    ScriptDialog() : 
     UI::Widget::Panel("", "dialogs.script", SP_VERB_DIALOG_SCRIPT)
    {}


    /**
     * Factory method
     */
    static ScriptDialog &getInstance();

    /**
     * Destructor
     */
    virtual ~ScriptDialog() {};


    private:
        int _max_dialog_width;
        int _max_dialog_height;


}; // class ScriptDialog


} //namespace Dialog
} //namespace UI
} //namespace Inkscape




#endif /* __DEBUGDIALOG_H__ */

