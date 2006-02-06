/*
 * Inkscape::XML::SimpleSession - simple session/logging implementation
 *
 * Copyright 2005 MenTaLguY <mental@rydia.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
 *
 */

#ifndef SEEN_INKSCAPE_XML_SIMPLE_SESSION_H
#define SEEN_INKSCAPE_XML_SIMPLE_SESSION_H

#include "gc-managed.h"
#include "xml/session.h"
#include "xml/transaction-logger.h"
#include "xml/log-builder.h"

namespace Inkscape {

namespace XML {

class SimpleSession : public GC::Managed<>,
                      public Session,
                      public TransactionLogger
{
public:
    SimpleSession() : _in_transaction(false) {}

    bool inTransaction() { return _in_transaction; }

    void beginTransaction();
    void rollback();
    void commit();
    Inkscape::XML::Event *commitUndoable();

    Node *createElementNode(char const *name);
    Node *createTextNode(char const *content);
    Node *createCommentNode(char const *content);

    Session &session() { return *this; }

    void notifyChildAdded(Inkscape::XML::Node &parent, Inkscape::XML::Node &child, Inkscape::XML::Node *prev);

    void notifyChildRemoved(Inkscape::XML::Node &parent, Inkscape::XML::Node &child, Inkscape::XML::Node *prev);

    void notifyChildOrderChanged(Inkscape::XML::Node &parent, Inkscape::XML::Node &child,
                                 Inkscape::XML::Node *old_prev, Inkscape::XML::Node *new_prev);

    void notifyContentChanged(Inkscape::XML::Node &node,
                              Util::shared_ptr<char> old_content,
                              Util::shared_ptr<char> new_content);

    void notifyAttributeChanged(Inkscape::XML::Node &node, GQuark name,
                                Util::shared_ptr<char> old_value,
                                Util::shared_ptr<char> new_value);

private:
    SimpleSession(SimpleSession const &); // no copy
    void operator=(SimpleSession const &); // no assign

    bool _in_transaction;
    LogBuilder _log_builder;
};

}

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
