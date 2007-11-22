#ifndef __TRACEDIALOG_H__
#define __TRACEDIALOG_H__
/*
 * A simple dialog for setting the parameters for autotracing a
 * bitmap <image> into an svg <path>
 *
 * Authors:
 *   Bob Jamison
 *   Other dudes from The Inkscape Organization
 *
 * Copyright (C) 2004, 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include "verbs.h"
#include "ui/widget/panel.h"

namespace Inkscape {
namespace UI {
namespace Dialog {


/**
 * A dialog that displays log messages
 */
class TraceDialog : public UI::Widget::Panel
{

public:

    /**
     * Constructor
     */
    TraceDialog() : 
     UI::Widget::Panel("", "dialogs.trace", SP_VERB_SELECTION_TRACE)
     {}


    /**
     * Factory method
     */
    static TraceDialog &getInstance();

    /**
     * Destructor
     */
    virtual ~TraceDialog() {};


};


} //namespace Dialog
} //namespace UI
} //namespace Inkscape




#endif /* __TRACEDIALOG_H__ */

