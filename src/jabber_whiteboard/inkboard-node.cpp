/**
 * Inkscape::Whiteboard::InkboardDocument - Inkboard document implementation
 *
 * Authors:
 * Dale Harvey <harveyd@gmail.com>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>
#include <glibmm.h>

#include "util/ucompose.hpp"

#include "jabber_whiteboard/message-utilities.h"
#include "jabber_whiteboard/defines.h"
#include "jabber_whiteboard/inkboard-document.h"


namespace Inkscape {

namespace Whiteboard {

Glib::ustring
InkboardDocument::addNodeToTracker(Inkscape::XML::Node *node)
{
    Glib::ustring key = this->tracker->generateKey(this->getRecipient());
    this->tracker->put(key,node);
    return key;
}

Message::Message
InkboardDocument::composeNewMessage(Inkscape::XML::Node *node)
{
    Glib::ustring parentKey;
    Glib::ustring key = this->tracker->get(node);
    Inkscape::XML::Node *parent = node->parent();

    Glib::ustring tempParentKey = this->tracker->get(node->parent());
    if(tempParentKey.size() < 1)
        parentKey = Vars::DOCUMENT_ROOT_NODE;
    else
        parentKey = tempParentKey;

    unsigned int index = parent->_childPosition(*node);

    Message::Message nodeMessage = MessageUtilities::objectToString(node);
    Message::Message message = String::ucompose(Vars::NEW_MESSAGE,parentKey,key,index,nodeMessage);

    return message;
}

void
InkboardDocument::changeConfigure(Glib::ustring target, signed int version, 
        Glib::ustring attribute, Glib::ustring value)
{
}

void 
InkboardDocument::changeNew(Glib::ustring target, Glib::ustring, 
        signed int index, Pedro::Element* data)
{
    Glib::ustring name = data->getName();

    if(name == "text")
    { 
        //XML::Node* parent = this->tracker->get(target);
        //parent->setContent(date->getValue());
    }

}

} // namespace Whiteboard
} // namespace Inkscape


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
