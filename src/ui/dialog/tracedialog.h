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
#include "dialog.h"

namespace Inkscape {
namespace UI {
namespace Dialog {


/**
 * A dialog that displays log messages
 */
class TraceDialog : public Dialog
{

public:


    /**
     * Constructor
     */
    TraceDialog(Behavior::BehaviorFactory behavior_factory) : 
	Dialog (behavior_factory, "dialogs.trace", SP_VERB_SELECTION_TRACE)
        {}


    /**
     * Factory method
     */
    static TraceDialog *create(Behavior::BehaviorFactory behavior_factory);

    /**
     * Destructor
     */
    virtual ~TraceDialog() {};


};


} //namespace Dialog
} //namespace UI
} //namespace Inkscape




#endif /* __TRACEDIALOG_H__ */

