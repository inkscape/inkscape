/**
 * Inkscape::ConsoleOutputUndoObserver - observer for tracing calls to
 * sp_document_undo, sp_document_redo, sp_document_maybe_done
 *
 * Authors:
 * David Yip <yipdw@alumni.rose-hulman.edu>
 *
 * Copyright (c) 2006 Authors
 *
 * Released under GNU GPL, see the file 'COPYING' for more information
 */

#ifndef __INKSCAPE_CONSOLE_OUTPUT_UNDO_OBSERVER_H__
#define __INKSCAPE_CONSOLE_OUTPUT_UNDO_OBSERVER_H__

#include "undo-stack-observer.h"

namespace Inkscape {

class ConsoleOutputUndoObserver : public UndoStackObserver {
public:
    ConsoleOutputUndoObserver() : UndoStackObserver() { }
    virtual ~ConsoleOutputUndoObserver() { }

    void notifyUndoEvent(Event* log);
    void notifyRedoEvent(Event* log);
    void notifyUndoCommitEvent(Event* log);
    void notifyClearUndoEvent();
    void notifyClearRedoEvent();

};
}

#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
