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

#include "xml/document.h"
#include "xml/simple-node.h"

#include "jabber_whiteboard/keynode.h"
#include "jabber_whiteboard/session-manager.h"

namespace Inkscape {

namespace Whiteboard {

class InkboardDocument : public XML::SimpleNode, public XML::Document {
public:
	explicit InkboardDocument(int code, SessionType type, Glib::ustring const& to);

	XML::NodeType type() const
	{
		return Inkscape::XML::DOCUMENT_NODE;
	}

	void setRecipient(Glib::ustring const& val);
	Glib::ustring getRecipient() const;

    void startSessionNegotiation();
    void terminateSession();
    void processInkboardEvent(MessageType mtype, unsigned int seqnum, Glib::ustring const& data);

protected:
	/**
	 * Copy constructor.
	 * 
	 * \param orig Instance to copy.
	 */
	InkboardDocument(InkboardDocument const& orig) :
		XML::Node(), XML::SimpleNode(orig), XML::Document(), _recipient(orig._recipient)
	{
		_initBindings();
	}

	XML::SimpleNode* _duplicate() const
	{
		return new InkboardDocument(*this);
	}

private:
	void _initBindings();

    SessionType _type;
	Glib::ustring _recipient;

    KeyNodeTable _tracker;
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
