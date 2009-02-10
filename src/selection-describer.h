/*
 * Inkscape::SelectionDescriber - shows messages describing selection
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_SELECTION_DESCRIPTION_HANDLER_H
#define SEEN_INKSCAPE_SELECTION_DESCRIPTION_HANDLER_H

#include <sigc++/sigc++.h>
#include "message-context.h"

namespace Inkscape { class Selection; }

namespace Inkscape {

class MessageStack;

class SelectionDescriber : public sigc::trackable {
public:
    SelectionDescriber(Inkscape::Selection *selection, MessageStack *stack);
    ~SelectionDescriber();

private:
    void _updateMessageFromSelection(Inkscape::Selection *selection);
    void _selectionModified(Inkscape::Selection *selection, guint /*flags*/);

    sigc::connection *_selection_changed_connection;
    sigc::connection *_selection_modified_connection;

    MessageContext _context;
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
