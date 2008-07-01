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

#include "pedro/pedrodom.h"

#include "xml/attribute-record.h"
#include "xml/element-node.h"
#include "xml/text-node.h"

#include "jabber_whiteboard/message-utilities.h"
#include "jabber_whiteboard/defines.h"
#include "jabber_whiteboard/inkboard-document.h"


namespace Inkscape {

namespace Whiteboard {

Glib::ustring
InkboardDocument::addNodeToTracker(Inkscape::XML::Node *node)
{
    Glib::ustring rec = this->getRecipient();
    Glib::ustring key = this->tracker->generateKey(rec);
    this->tracker->put(key,node);
    return key;
}

Message::Message
InkboardDocument::composeNewMessage(Inkscape::XML::Node *node)
{
    Glib::ustring parentKey;
    Glib::ustring key = this->tracker->get(node);

    Glib::ustring tempParentKey = this->tracker->get(node->parent());
    if(tempParentKey.size() < 1)
        parentKey = Vars::DOCUMENT_ROOT_NODE;
    else
        parentKey = tempParentKey;

    unsigned int index = node->position();

    Message::Message nodeMessage = MessageUtilities::objectToString(node);
    Message::Message message = String::ucompose(Vars::NEW_MESSAGE,parentKey,key,index,0,nodeMessage);

    return message;
}

void
InkboardDocument::changeConfigureText(Glib::ustring target,
                                      unsigned int /*version*/,
                                      Glib::ustring text)
{
    XML::Node *node = this->tracker->get(target);
    //unsigned int elementVersion = this->tracker->getVersion(node);

    if(node)// && version == (elementVersion + 1))
    {
        this->tracker->incrementVersion(node);
        this->tracker->addHistory(node, "text", text);
        node->setContent(text.c_str());
    }
}

void
InkboardDocument::changeConfigure(Glib::ustring target,
                                  unsigned int /*version*/, 
                                  Glib::ustring attribute,
								  Glib::ustring value)
{
    XML::Node *node = this->tracker->get(target);
    //unsigned int elementVersion = this->tracker->getVersion(node);

    if(node)// && version == (elementVersion + 1))
    {
        this->tracker->incrementVersion(node);
        this->tracker->addHistory(node, attribute, value.c_str());

        if(attribute != "transform")
            node->setAttribute(attribute.c_str(),value.c_str());
    }
}

void 
InkboardDocument::changeNew(Glib::ustring parentid, Glib::ustring id, 
        signed int /*index*/, Pedro::Element* data)
{

    Glib::ustring name(data->getName());

    if(name == "text")
    { 
        XML::Node *parent = this->tracker->get(parentid);
        XML::Node *node = new XML::TextNode(Util::share_string(data->getValue().c_str()), this);

        if(parent && node)
        {
            this->tracker->put(id,node);
            parent->appendChild(node);
        }
    }else
    {
        XML::Node *node = new XML::ElementNode(g_quark_from_string(name.c_str()), this);
        this->tracker->put(id,node);

        XML::Node *parent = (parentid != "ROOT") 
            ? this->tracker->get(parentid.c_str()) : this->root();

        std::vector<Pedro::Attribute> attributes = data->getAttributes();

        for (unsigned int i=0; i<attributes.size(); i++) 
        {
            node->setAttribute(
                (attributes[i].getName()).c_str(),
                (attributes[i].getValue()).c_str());
        }

        if(parent != NULL)
            parent->appendChild(node);
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
