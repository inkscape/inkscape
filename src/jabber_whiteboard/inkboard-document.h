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
#include "jabber_whiteboard/defines.h"
#include "jabber_whiteboard/keynode.h"
#include "jabber_whiteboard/session-manager.h"

namespace Inkscape {

namespace Whiteboard {

class InkboardDocument : public XML::SimpleNode, public XML::Document {
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
    bool send(const Glib::ustring &destJid, Message::Wrapper &mwrapper, Message::Message &message);

    void sendDocument(Inkscape::XML::Node* root);

    bool handleOutgoingState(Message::Wrapper &wrapper,Glib::ustring const& message);
    bool handleIncomingState(Message::Wrapper &wrapper,Pedro::Element* data);

    bool handleState(State::SessionState expectedState, State::SessionState newstate);

    Glib::ustring addNodeToTracker(Inkscape::XML::Node* node);
    Message::Message composeNewMessage(Inkscape::XML::Node *node);

protected:
	/**
	 * Copy constructor.
	 * 
	 * \param orig Instance to copy.
	 */
	InkboardDocument(InkboardDocument const& orig) :
		XML::Node(), XML::SimpleNode(orig), XML::Document(), recipient(orig.recipient)
	{
		_initBindings();
	}

	XML::SimpleNode* _duplicate() const
	{
		return new InkboardDocument(*this);
	}

private:

    void _initBindings();

    SessionManager      *sm;

    State::SessionType  sessionType;

    Glib::ustring sessionId;
    Glib::ustring recipient;
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
