/** @file
 * @brief Bitmap tracing settings dialog
 */
/* Authors:
 *   Bob Jamison
 *   Other dudes from The Inkscape Organization
 *
 * Copyright (C) 2004, 2005 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __TRACEDIALOG_H__
#define __TRACEDIALOG_H__

#include "ui/widget/panel.h"
#include "verbs.h"

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
     UI::Widget::Panel("", "/dialogs/trace", SP_VERB_SELECTION_TRACE)
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
