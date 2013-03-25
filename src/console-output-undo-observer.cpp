/*
 * Inkscape::ConsoleOutputUndoObserver - observer for tracing calls to
 * SPDocumentUndo::undo, SPDocumentUndo::redo, SPDocumentUndo::maybe_done
 *
 * Authors:
 * David Yip <yipdw@alumni.rose-hulman.edu>
 *   Abhishek Sharma
 *
 * Copyright (c) 2006 Authors
 *
 * Released under GNU GPL, see the file 'COPYING' for more information
 */

#include "console-output-undo-observer.h"

namespace Inkscape {

void
ConsoleOutputUndoObserver::notifyUndoEvent(Event* /*log*/)
{
    // g_message("notifyUndoEvent(SPDocumentUndo::undo) called; log=%p\n", log->event);
}

void
ConsoleOutputUndoObserver::notifyRedoEvent(Event* /*log*/)
{
    // g_message("notifyRedoEvent(SPDocumentUndo::redo) called; log=%p\n", log->event);
}

void
ConsoleOutputUndoObserver::notifyUndoCommitEvent(Event* /*log*/)
{
    //g_message("notifyUndoCommitEvent(SPDocumentUndo::maybe_done) called; log=%p\n", log->event);
}

void
ConsoleOutputUndoObserver::notifyClearUndoEvent()
{
    //g_message("notifyClearUndoEvent(sp_document_clear_undo) called);
}

void
ConsoleOutputUndoObserver::notifyClearRedoEvent()
{
    //g_message("notifyClearRedoEvent(sp_document_clear_redo) called);
}

}

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
