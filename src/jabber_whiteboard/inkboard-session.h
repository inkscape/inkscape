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

#include "pedro/pedroxmpp.h"
#include "jabber_whiteboard/defines.h"

#include "xml/simple-session.h"
#include "util/share.h"

namespace Inkscape {

namespace Whiteboard {

class InkboardSession : public GC::Managed<>,
						public XML::Session,
						public XML::TransactionLogger
{
public:
	InkboardSession() : _in_transaction(false) { }
	InkboardSession(Glib::ustring const& name) : _in_transaction(false), _name(name) { }
	virtual ~InkboardSession() { }
    
    /**
     * Returns the name of this session.
	 * \return The name of this session.
     */
    virtual Glib::ustring getName() const
        { return _name; }
        
    /**
     * Sets the name of this session.
	 *
	 * \param val The name to use.
     */
    virtual void setName(const Glib::ustring &val)
        { _name = val; }

    /**
     * Returns status attributes of this session.
     *
     * \return Status of this session.
     */
    virtual std::bitset< NUM_FLAGS > const& getStatus() const
    {
    	return status;
    }

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
    // XML::NodeObserver methods
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

    std::bitset< NUM_FLAGS > status;
    Glib::ustring _name;
};

}

}

#endif
