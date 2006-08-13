/**
 * Inkscape::Whiteboard::InkboardSession - Whiteboard implementation of XML::Session interface
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __INKSCAPE_WHITEBOARD_SESSION_H__
#define __INKSCAPE_WHITEBOARD_SESSION_H__

#include <glibmm.h>
#include <bitset>

#include "gc-managed.h"

#include "xml/session.h"
#include "xml/transaction-logger.h"
#include "xml/log-builder.h"
#include "xml/node-observer.h"
#include "xml/simple-session.h"

#include "pedro/pedroxmpp.h"

#include "jabber_whiteboard/inkboard-document.h"
#include "jabber_whiteboard/inkboard-node.h"

#include "jabber_whiteboard/defines.h"

#include "util/share.h"

namespace Inkscape {

namespace Whiteboard {

class InkboardDocument;

class InkboardSession : public GC::Managed<>, public XML::Session,
                         public XML::TransactionLogger
{
public:

    InkboardSession() : _in_transaction(false) { }
    InkboardSession(InkboardDocument *document) : _in_transaction(false), doc(document) {}
    virtual ~InkboardSession() { }

    //
    // XML::TransactionLogger methods
    //
    Session& session()
    {
    	return *this;
    }

    // 
    // XML::Session methods
    // 
    bool inTransaction() 
    {
    	return _in_transaction;
    }

    void beginTransaction();
    void rollback();
    void commit();

    XML::Event* commitUndoable();

    XML::Node* createElementNode(char const* name);
    XML::Node* createTextNode(char const* content);
    XML::Node* createCommentNode(char const* content);

    //
    // XML::NodeObserver methodscd ../
    // (inherited from XML::TransactionLogger)
    //
    void notifyChildAdded(Inkscape::XML::Node &parent, Inkscape::XML::Node &child, Inkscape::XML::Node *prev);

    void notifyChildRemoved(Inkscape::XML::Node &parent, Inkscape::XML::Node &child, Inkscape::XML::Node *prev);

    void notifyChildOrderChanged(Inkscape::XML::Node &parent, Inkscape::XML::Node &child,
                                 Inkscape::XML::Node *old_prev, Inkscape::XML::Node *new_prev);

    void notifyContentChanged(Inkscape::XML::Node &node,
                              Util::ptr_shared<char> old_content,
                              Util::ptr_shared<char> new_content);

    void notifyAttributeChanged(Inkscape::XML::Node &node, GQuark name,
                                Util::ptr_shared<char> old_value,
                                Util::ptr_shared<char> new_value);

private:

    InkboardSession(InkboardSession const &); // no copy
    void operator=(InkboardSession const &); // no assign

    bool _in_transaction;

    InkboardDocument *doc;
};

}

}

#endif
