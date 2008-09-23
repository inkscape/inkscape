/**
 * Inkscape::Whiteboard::InkboardDocument - Inkboard document implementation
 *
 * Authors:
 * David Yip <yipdw@rose-hulman.edu>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __INKSCAPE_WHITEBOARD_INKBOARDDOCUMENT_H__
#define __INKSCAPE_WHITEBOARD_INKBOARDDOCUMENT_H__

#include <glibmm.h>

#include "document.h"
#include "xml/document.h"
#include "xml/simple-node.h"
#include "xml/node-observer.h"
#include "jabber_whiteboard/defines.h"
#include "jabber_whiteboard/keynode.h"
#include "jabber_whiteboard/session-manager.h"

namespace Inkscape {

namespace Whiteboard {

class InkboardDocument : public XML::SimpleNode,
                         public XML::Document,
                         public XML::NodeObserver
{
public:
	
    explicit InkboardDocument(int code, State::SessionType sessionType, Glib::ustring const& to);

    XML::NodeType type() const
    {
	return Inkscape::XML::DOCUMENT_NODE;
    }

    State::SessionState state;
    KeyNodeTable *tracker;

    void setRecipient(Glib::ustring const& val);
    Glib::ustring getRecipient() const;

    void setSessionId(Glib::ustring const& val);
    Glib::ustring getSessionId() const;

    void startSessionNegotiation();
    void terminateSession();

    void recieve(Message::Wrapper &wrapper, Pedro::Element* data);
    bool send(const Glib::ustring &destJid, Message::Wrapper &mwrapper, 
            Message::Message &message);

    void sendDocument(Inkscape::XML::Node* root);

    bool handleOutgoingState(Message::Wrapper &wrapper,Glib::ustring const& message);
    bool handleIncomingState(Message::Wrapper &wrapper,Pedro::Element* data);

    bool handleState(State::SessionState expectedState, 
            State::SessionState newstate);

    void handleChange(Message::Wrapper &wrapper, Pedro::Element* data);

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

    XML::Node* createElement(char const* name);
    XML::Node* createTextNode(char const* content);
    XML::Node* createComment(char const* content);
    XML::Node* createPI(char const *target, char const* content);

    //
    // XML::NodeObserver methods
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

    /* Functions below are defined in inkboard-node.cpp */
    Glib::ustring addNodeToTracker(Inkscape::XML::Node* node);
    Message::Message composeNewMessage(Inkscape::XML::Node *node);

    void changeConfigure(Glib::ustring target, unsigned int version,
            Glib::ustring attribute, Glib::ustring value);

    void changeNew(Glib::ustring target, Glib::ustring, 
            signed int index, Pedro::Element* data);

    void changeConfigureText(Glib::ustring target, unsigned int version,
            Glib::ustring text);

protected:
	/**
	 * Copy constructor.
	 * 
	 * \param orig Instance to copy.
	 */
	InkboardDocument(InkboardDocument const& orig) :
		XML::Node(), XML::SimpleNode(orig),
                XML::Document(), XML::NodeObserver(),
                recipient(orig.recipient), _in_transaction(false)
	{
		_initBindings();
	}

	XML::SimpleNode* _duplicate(XML::Document* /*xml_doc*/) const
	{
		return new InkboardDocument(*this);
	}
	NodeObserver *logger() { return this; }

private:
    void _initBindings();

    SessionManager      *sm;

    State::SessionType  sessionType;

    Glib::ustring sessionId;
    Glib::ustring recipient;

    bool _in_transaction;
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
