/**
 * Phoebe DOM Implementation.
 *
 * This is a C++ approximation of the W3C DOM model, which follows
 * fairly closely the specifications in the various .idl files, copies of
 * which are provided for reference.  Most important is this one:
 *
 * http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/idl-definitions.html
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2005 Bob Jamison
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include "domimpl.h"

namespace org
{
namespace w3c
{
namespace dom
{


/**
 *  Test if the given substring exists for the length of the string
 *  in a given buffer
 */
/*
static bool match(const DOMString &buf, char *str)
{
    int pos = 0;
    while (*str)
       {
       if (buf[pos++] != *str++)
           return false;
       }
   return true;
}
*/



/*#########################################################################
## DOMImplementationSourceImpl
#########################################################################*/


/**
 *
 */
DOMImplementationSourceImpl::DOMImplementationSourceImpl()
{
    domImpl     = new DOMImplementationImpl();
}

/**
 *
 */
DOMImplementationSourceImpl::~DOMImplementationSourceImpl()
{
    delete domImpl;
}

/**
 *
 */
DOMImplementation *DOMImplementationSourceImpl::getDOMImplementation(
                           const DOMString &features)
{
    return domImpl;
}

/**
 *
 */
DOMImplementationList DOMImplementationSourceImpl::getDOMImplementationList(
                           const DOMString &features)
{
    return domImplList;
}







/*#########################################################################
## DOMImplementationImpl
#########################################################################*/


/**
 *
 */
DOMImplementationImpl::DOMImplementationImpl()
{
}

/**
 *
 */
DOMImplementationImpl::~DOMImplementationImpl()
{
}

/**
 *
 */
bool DOMImplementationImpl::hasFeature(const DOMString& feature,
                        const DOMString& version)
{
    return false;
}


/**
 *
 */
DocumentType *DOMImplementationImpl::createDocumentType(const DOMString& qualifiedName,
                                 const DOMString& publicId,
                                 const DOMString& systemId)
                                 throw(DOMException)
{
    DocumentTypeImpl *typeImpl = new DocumentTypeImpl(qualifiedName,
                                         publicId, systemId);
    return typeImpl;
}

/**
 *
 */
Document *DOMImplementationImpl::createDocument(
                         const DOMString& namespaceURI,
                         const DOMString& qualifiedName,
                         DocumentType *doctype)
                         throw(DOMException)
{
    DocumentImpl *doc = new DocumentImpl(this,
                                         namespaceURI,
                                         qualifiedName,
                                         doctype);
    return doc;
}

/**
 *
 */
DOMObject *DOMImplementationImpl::getFeature(const DOMString& feature,
                         const DOMString& version)

{
    return NULL;
}





/*#########################################################################
## NodeImpl
#########################################################################*/

/**
 * Utility for finding the first Element above
 * a given node.  Used by several methods below
 */
static Node *getAncestorElement(Node *node)
{
    if (!node)
        return NULL;
    node = node->getParentNode();
    //Either quit because I am an element, or because I am null
    while (node)
        {
        if (node->getNodeType() == Node::ELEMENT_NODE)
            return node;
        node = node->getParentNode();
        }
    return node;
}

/**
 *
 */
DOMString NodeImpl::getNodeName()
{
    return nodeName;
}

/**
 *
 */
DOMString NodeImpl::getNodeValue() throw (DOMException)
{
    return nodeValue;
}

/**
 *
 */
void NodeImpl::setNodeValue(const DOMString& val) throw (DOMException)
{
    nodeValue = val;
}

/**
 *
 */
unsigned short NodeImpl::getNodeType()
{
    return nodeType;
}

/**
 *
 */
Node *NodeImpl::getParentNode()
{
    return parent;
}

/**
 *
 */
NodeList NodeImpl::getChildNodes()
{
    NodeList list;
    for (NodeImpl *node = firstChild ; node ; node=node->next)
        list.add(node);
    return list;
}

/**
 *
 */
Node *NodeImpl::getFirstChild()
{
    return firstChild;
}

/**
 *
 */
Node *NodeImpl::getLastChild()
{
    return lastChild;
}

/**
 *
 */
Node *NodeImpl::getPreviousSibling()
{
    return prev;
}

/**
 *
 */
Node *NodeImpl::getNextSibling()
{
    return next;
}

/**
 *
 */
NamedNodeMap &NodeImpl::getAttributes()
{
    NamedNodeMap &attrs = attributes;
    return attrs;
}


/**
 *
 */
Document *NodeImpl::getOwnerDocument()
{
    return ownerDocument;
}

/**
 *
 */
Node *NodeImpl::insertBefore(const Node *newChild,
                   const Node *refChild)
                   throw(DOMException)
{
    Node *node = NULL;
    return node;
}

/**
 *
 */
Node *NodeImpl::replaceChild(const Node *newChild,
                   const Node *oldChild)
                   throw(DOMException)
{
    Node *node = NULL;
    return node;
}

/**
 *
 */
Node *NodeImpl::removeChild(const Node *oldChild)
                  throw(DOMException)
{
    Node *node = NULL;
    return node;
}

/**
 *
 */
Node *NodeImpl::appendChild(const Node *newChild)
                  throw(DOMException)
{
    if (!newChild)
        return NULL;

    Node *child = (Node *)newChild;
    NodeImpl *childImpl = dynamic_cast<NodeImpl *> (child);

    childImpl->parent        = this;
    childImpl->ownerDocument = ownerDocument;

    if (!firstChild || !lastChild)
        {
        //Set up our first member
        firstChild = childImpl;
        lastChild  = childImpl;
        }
    else
        {
        //link at the last position
        lastChild->next = childImpl;
        childImpl->prev = lastChild;
        lastChild       = childImpl;
        }

    return child;
}

/**
 *
 */
bool NodeImpl::hasChildNodes()
{
    return (firstChild != NULL);
}

/**
 *
 */
Node *NodeImpl::cloneNode(bool deep)
{
    NodeImpl *node = new NodeImpl(ownerDocument, nodeName);
    node->parent        = parent;
    node->prev          = prev;
    node->next          = next;
    node->userData      = userData;
    node->nodeValue     = nodeValue;

    if (deep)
        {
        node->firstChild = node->lastChild = NULL;
        for (NodeImpl *child = firstChild ; child ; child=child->next)
            {
            node->appendChild(child->cloneNode(deep));
            }
        }
    else
        {
        node->firstChild = firstChild;
        node->lastChild  = lastChild;
        }

    return node;
}

/**
 *  Concatenate adjoining text subnodes, remove null-length nodes
 */
void NodeImpl::normalize()
{
    //First, concatenate adjoining text nodes
    NodeImpl *next = NULL;
    for (NodeImpl *child = firstChild ; child ; child=next)
        {
        if (child->getNodeType() != Node::TEXT_NODE)
            continue;
        next = NULL;
        DOMString sval = child->getNodeValue();
        for (NodeImpl *sibling = child->next ; sibling ; sibling=next)
            {
            next = sibling->next;
            if (sibling->getNodeType() != Node::TEXT_NODE)
                break;
            sval.append(sibling->getNodeValue());
            //unlink and delete
            child->next = sibling->next;
            if (sibling->next)
                sibling->next->prev = child;
            delete sibling;
            }
        child->setNodeValue(sval);
        }

    //Next, we remove zero-length text subnodes
    next = NULL;
    for (NodeImpl *child = firstChild ; child ; child=next)
        {
        next = child->next;
        if (child->getNodeType() != Node::TEXT_NODE)
            continue;
        if (child->getNodeValue().size() == 0)
            {
            //unlink and delete
            if (child->prev)
                child->prev->next = child->next;
            if (child->next)
                child->next->prev = child->prev;
            delete child;
            }
        }

}

/**
 *
 */
bool NodeImpl::isSupported(const DOMString& feature,
                 const DOMString& version)
{
    //again, no idea
    return false;
}

/**
 *
 */
DOMString NodeImpl::getNamespaceURI()
{
    return namespaceURI;
}

/**
 *
 */
DOMString NodeImpl::getPrefix()
{
    return prefix;
}

/**
 *
 */
void NodeImpl::setPrefix(const DOMString& val) throw(DOMException)
{
    prefix = val;
    if (prefix.size()>0)
        nodeName = prefix + ":" + localName;
    else
        nodeName = localName;
}

/**
 *
 */
DOMString NodeImpl::getLocalName()
{
    return localName;
}

/**
 *
 */
bool NodeImpl::hasAttributes()
{
    return (attributes.getLength() > 0);
}

/**
 *
 */
DOMString NodeImpl::getBaseURI()
{
    return baseURI;
}

/**
 *
 */
unsigned short NodeImpl::compareDocumentPosition(const Node *otherArg)
{
    if (!otherArg || this == otherArg)
        return 0;//no flags

    Node *node;
    Node *other = (Node *)otherArg;

    //Look above me
    for (node=getParentNode() ; node ; node=node->getParentNode())
        if (node == other)
            return DOCUMENT_POSITION_CONTAINED_BY;

    //Look above the other guy. See me?
    for (node=other->getParentNode() ; node ; node=node->getParentNode())
        if (node == this)
            return DOCUMENT_POSITION_CONTAINS;


    return DOCUMENT_POSITION_DISCONNECTED |
           DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC;
}

/**
 *
 */
DOMString NodeImpl::getTextContext() throw(DOMException)
{
    //no idea
    return DOMString("");
}


/**
 *
 */
void NodeImpl::setTextContext(const DOMString &val) throw(DOMException)
{
    //no idea
}


/**
 * From DOM3 Namespace algorithms
 */
DOMString NodeImpl::lookupPrefix(const DOMString &theNamespaceURI)
{

    if (theNamespaceURI.size()==0)
        {
        return DOMString("");
        }

    switch (nodeType)
        {
        case Node::ELEMENT_NODE:
            {
            ElementImpl *elem = (ElementImpl *)this;
            return lookupNamespacePrefix(theNamespaceURI, elem);
            }
        case Node::DOCUMENT_NODE:
            {
            Document *doc = (Document *)this;
            Element *elem = doc->getDocumentElement();
            return elem->lookupPrefix(theNamespaceURI);
            }
        case Node::ENTITY_NODE :
        case Node::NOTATION_NODE:
        case Node::DOCUMENT_FRAGMENT_NODE:
        case Node::DOCUMENT_TYPE_NODE:
            return DOMString("");  // type is unknown
        case Node::ATTRIBUTE_NODE:
            {
            Attr *attr = (Attr *)this;
            Element *elem = (Element *)attr->getOwnerElement();
            if ( elem )
                {
                return elem->lookupPrefix(theNamespaceURI);
                }
             return DOMString("");
            }
        default:
            {
            //Get ancestor element, if any
            if ( Node *ancestor = getAncestorElement(this) )
               {
               return ancestor->lookupPrefix(theNamespaceURI);
               }
            return DOMString("");
            }
        }//switch
    return DOMString("");
}


/**
 *
 */
bool NodeImpl::isDefaultNamespace(const DOMString &theNamespaceURI)
{
    switch (nodeType)
        {
        case ELEMENT_NODE:
            if ( namespaceURI.size()>0 && prefix.size()==0 )
                {
                return (namespaceURI == theNamespaceURI);
                }
            if ( Node *attr = attributes.getNamedItem("xmlns"))
                {
	        return (attr->getNodeValue() == theNamespaceURI);
                }


            if ( Node *ancestor = getAncestorElement(this) )
                {
                return ancestor->isDefaultNamespace(theNamespaceURI);
                }
            else
                {
                return false;
                }
        case DOCUMENT_NODE:
            { //just use braces for local declaration
            Document *doc = (Document *)this;
            Element *elem = doc->getDocumentElement();
            return elem->isDefaultNamespace(theNamespaceURI);
            }
        case ENTITY_NODE:
        case NOTATION_NODE:
        case DOCUMENT_TYPE_NODE:
        case DOCUMENT_FRAGMENT_NODE:
            return false;
        case ATTRIBUTE_NODE:
           {//braces only for scope
           Attr *attr = (Attr *)this;
           Element *ownerElement = attr->getOwnerElement();
           if ( ownerElement )
                {
                return ownerElement->isDefaultNamespace(theNamespaceURI);
                }
           else
                {
                return false;
                }
           }
        default:
            if ( Node *ancestor = getAncestorElement(this) )
                {
                return ancestor->isDefaultNamespace(theNamespaceURI);
                }
            else
                {
                return false;
                }
        }//switch

    return false;
}


/**
 *
 */
DOMString NodeImpl::lookupNamespaceURI(const DOMString &thePrefix)
{
    switch (nodeType)
        {
        case ELEMENT_NODE:
            {
            if ( namespaceURI.size()>0 && prefix == thePrefix )
                {
                DOMString nsURI = namespaceURI;
                return (nsURI);
                }
            if ( hasAttributes() )
                {
                NamedNodeMap attributes = getAttributes();
                int nrAttrs = attributes.getLength();
                for (int i=0 ; i<nrAttrs ; i++)
                    {
                    Node *attr = attributes.item(i);
                    if (attr->getPrefix() == "xmlns" && attr->getLocalName() == thePrefix )
                        { // non default namespace
                        if (attr->getNodeValue().size()>0)
                            {
                            return (attr->getNodeValue());
                            }
                        return DOMString("");
                        }
                    else if (attr->getLocalName() == "xmlns" && thePrefix.size()==0)
                        { // default namespace
                        if (attr->getNodeValue().size()>0)
                            {
                            return (attr->getNodeValue());
                            }
                        return DOMString("");
                        }
                    }
                }

            if ( Node *ancestor = getAncestorElement(this) )
                {
                return ancestor->lookupNamespaceURI(thePrefix);
                }
            return DOMString("");
            }
        case DOCUMENT_NODE:
            {
            Document *doc = (Document *)this;
            Element *elem = doc->getDocumentElement();
            return elem->lookupNamespaceURI(thePrefix);
            }
        case ENTITY_NODE:
        case NOTATION_NODE:
        case DOCUMENT_TYPE_NODE:
        case DOCUMENT_FRAGMENT_NODE:
            return DOMString("");

        case ATTRIBUTE_NODE:
            {
            if (Element *ownerElement = ((Attr *)this)->getOwnerElement())
                {
                return ownerElement->lookupNamespaceURI(thePrefix);
                }
            else
                {
                return DOMString("");
                }
            }
        default:
            if ( Node *ancestor = getAncestorElement(this) )
                {
                return ancestor->lookupNamespaceURI(thePrefix);
                }
            else
                {
                return DOMString("");
                }
        }//switch
}


/**
 *
 */
bool NodeImpl::isEqualNode(const Node *nodeArg)
{
    if (!nodeArg)
        return false;

    if (this == nodeArg)
        return true;

    Node *node = (Node *)nodeArg;

    if (getNodeType()     != node->getNodeType()     ||
        getNodeName()     != node->getNodeName()     ||
        getLocalName()    != node->getLocalName()    ||
        getNamespaceURI() != node->getNamespaceURI() ||
        getPrefix()       != node->getPrefix()       ||
        getNodeValue()    != node->getNodeValue()    ||
        getBaseURI()      != node->getBaseURI()      )
        return false;

    return true;
}



/**
 *
 */
DOMObject *NodeImpl::getFeature(const DOMString &feature,
                             const DOMString &version)
{
    //dont know
    return NULL;
}

/**
 *
 */
DOMUserData *NodeImpl::setUserData(const DOMString &key,
                                   const DOMUserData *data,
                                   const UserDataHandler *handler)
{
    UserDataEntry *entry = userDataEntries;
    UserDataEntry *prev  = NULL;
    while (entry)
        {
        if (entry->key == key)
            {
            DOMUserData *oldData = entry->data;
            entry->data    = (DOMUserData *)data;
            entry->handler = (UserDataHandler *)handler;
            return oldData;
            }
        prev  = entry;
        entry = entry->next;
        }

    //Make a new one
    UserDataEntry *newEntry = new UserDataEntry(key, data, handler);
    if (!prev)
        userDataEntries = newEntry;
    else
        prev->next = newEntry;

    return NULL;
}

/**
 *
 */
DOMUserData *NodeImpl::getUserData(const DOMString &key)
{
    UserDataEntry *entry = userDataEntries;
    while (entry)
        {
        if (entry->key == key)
            return entry->data;
        entry = entry->next;
        }
    return NULL;
}



//##################
//# Non-API methods
//##################

/**
 *
 */
void NodeImpl::setNodeName(const DOMString &qualifiedName)
{
    nodeName  = qualifiedName;
    prefix    = "";
    localName = "";
    for (unsigned int i=0 ; i<qualifiedName.size() ; i++)
        {
        int ch = qualifiedName[i];
        if (ch == ':')
            {
            prefix    = localName;
            localName = "";
            }
        else
            {
            localName.push_back(ch);
            }
        }
}

/**
 *
 */
void NodeImpl::setNamespaceURI(const DOMString &theNamespaceURI)
{
    namespaceURI = theNamespaceURI;
}


/**
 * From DOM3 Namespace algorithms
 */
DOMString NodeImpl::lookupNamespacePrefix(const DOMString &theNamespaceURI,
                                Node *originalElement)
{
    if (!originalElement)
        return DOMString("");

    if ( namespaceURI.size()>0 && namespaceURI==theNamespaceURI &&
         prefix.size()>0 &&
         originalElement->lookupNamespaceURI(prefix) == theNamespaceURI)
        {
        return (prefix);
        }

    if ( hasAttributes() )
        {
        NamedNodeMap attributes = getAttributes();
        int nrAttrs = attributes.getLength();
        for (int i=0 ; i<nrAttrs ; i++)
            {
            Node *attr = attributes.item(i);
            DOMString attrLocalName = attr->getLocalName();
            if (attr->getPrefix()    == "xmlns" &&
                attr->getNodeValue() == theNamespaceURI &&
                originalElement->lookupNamespaceURI(attrLocalName)
                                     == theNamespaceURI)
                {
                return (attrLocalName);
                }
            }
        }

    //Get ancestor element, if any
    NodeImpl *ancestor = parent;
    while (ancestor && ancestor->getNodeType()!= Node::ELEMENT_NODE)
        ancestor = ancestor->parent;

    if ( ancestor )
        {
        return ancestor->lookupNamespacePrefix(theNamespaceURI, originalElement);
        }

    return DOMString("");
}


/**
 *
 */
NodeImpl::NodeImpl()
{
    init();
}




/**
 *
 */
NodeImpl::NodeImpl(DocumentImpl *owner)
{
    init();
    ownerDocument = owner;
}

/**
 *
 */
NodeImpl::NodeImpl(DocumentImpl *owner, const DOMString &nodeName)
{
    init();
    ownerDocument = owner;
    setNodeName(nodeName);
}

/**
 *
 */
NodeImpl::NodeImpl(DocumentImpl *owner, const DOMString &theNamespaceURI,
                      const DOMString &qualifiedName)
{
    init();
    ownerDocument = owner;
    //if (owner)
    //    namespaceURI  = owner->stringCache(theNamespaceURI);
    setNodeName(qualifiedName);
}



/**
 *
 */
void NodeImpl::init()
{
    nodeType        = 0; //none yet
    nodeValue       = "";
    setNodeName("");
    namespaceURI    = "";
    parent          = NULL;
    prev            = NULL;
    next            = NULL;
    userData        = NULL;
    firstChild      = NULL;
    lastChild       = NULL;
    ownerDocument   = NULL;
    userDataEntries = NULL;
}

/**
 *
 */
void NodeImpl::assign(const NodeImpl &other)
{
    ownerDocument = other.ownerDocument;
    prefix        = other.prefix;
    localName     = other.localName;
    nodeName      = other.nodeName;
    nodeValue     = other.nodeValue;
    namespaceURI  = other.namespaceURI;
    attributes    = other.attributes;
}


/**
 *
 */
NodeImpl::~NodeImpl()
{
    if (userDataEntries)
        delete userDataEntries;
}



/*#########################################################################
## CharacterDataImpl
#########################################################################*/


/**
 *
 */
CharacterDataImpl::CharacterDataImpl() : NodeImpl()
{
}

/**
 *
 */
CharacterDataImpl::CharacterDataImpl(DocumentImpl *owner,
                                     const DOMString &theValue) : NodeImpl()
{
    ownerDocument = owner;
    nodeValue     = theValue;
}

/**
 *
 */
CharacterDataImpl::~CharacterDataImpl()
{
}

/**
 *
 */
DOMString CharacterDataImpl::getData() throw(DOMException)
{
    return nodeValue;
}

/**
 *
 */
void CharacterDataImpl::setData(const DOMString& val) throw(DOMException)
{
    nodeValue = val;
}

/**
 *
 */
unsigned long CharacterDataImpl::getLength()
{
    return nodeValue.size();
}

/**
 *
 */
DOMString CharacterDataImpl::substringData(unsigned long offset,
                        unsigned long count)
                        throw(DOMException)
{
    return nodeValue.substr(offset, count);
}

/**
 *
 */
void CharacterDataImpl::appendData(const DOMString& arg) throw(DOMException)
{
    nodeValue += arg;
}

/**
 *
 */
void CharacterDataImpl::insertData(unsigned long offset,
                const DOMString& arg)
                throw(DOMException)
{
    nodeValue.insert(offset, arg);
}

/**
 *
 */
void CharacterDataImpl::deleteData(unsigned long offset,
                unsigned long count)
                throw(DOMException)
{
    nodeValue.erase(offset, count);
}

/**
 *
 */
void  CharacterDataImpl::replaceData(unsigned long offset,
                  unsigned long count,
                  const DOMString& arg)
                  throw(DOMException)
{
    nodeValue.replace(offset, count, arg);
}






/*#########################################################################
## AttrImpl
#########################################################################*/

/**
 *
 */
DOMString AttrImpl::getName()
{
    return nodeName;
}

/**
 *
 */
bool AttrImpl::getSpecified()
{
    return (nodeValue.size() > 0);
}

/**
 *
 */
DOMString AttrImpl::getValue()
{
    return nodeValue;
}

/**
 *
 */
void AttrImpl::setValue(const DOMString& val) throw(DOMException)
{
    nodeValue = val;
}

/**
 *
 */
Element *AttrImpl::getOwnerElement()
{
    return ownerElement;
}


/**
 *
 */
TypeInfo *AttrImpl::getSchemaTypeInfo()
{
    return NULL;
}


/**
 *
 */
bool AttrImpl::getIsId()
{
    return (nodeName == "id");
}



//##################
//# Non-API methods
//##################


void AttrImpl::setOwnerElement(const Element *elem)
{
    ownerElement = (Element *)elem;
}

/**
 *
 */
AttrImpl::AttrImpl(DocumentImpl *owner, const DOMString &theName)
                   : NodeImpl()
{
    nodeType     = ATTRIBUTE_NODE;
    ownerDocument = owner;
    setNodeName(theName);
}

/**
 *
 */
AttrImpl::AttrImpl(DocumentImpl *owner,
                   const DOMString &theNamespaceURI,
                   const DOMString &theQualifiedName)
                   : NodeImpl()
{
    nodeType     = ATTRIBUTE_NODE;
    ownerDocument = owner;
    //if (owner)
    //    namespaceURI  = owner->stringCache(theNamespaceURI);
    setNodeName(theQualifiedName);
}

/**
 *
 */
AttrImpl::~AttrImpl()
{
}





/*#########################################################################
## ElementImpl
#########################################################################*/


/**
 *
 */
DOMString ElementImpl::getTagName()
{
    if (prefix.size() > 0)
        return prefix + ":" + nodeName;
    else
        return nodeName;
}

/**
 *
 */
DOMString ElementImpl::getAttribute(const DOMString& name)
{
    Node *node = attributes.getNamedItem(name);
    if (!node || node->getNodeType() != ATTRIBUTE_NODE)
        return DOMString("");
    Attr *attr = dynamic_cast<Attr *>(node);
    return attr->getValue();
}

/**
 *
 */
void ElementImpl::setAttribute(const DOMString& name,
                  const DOMString& value)
                  throw(DOMException)
{
    AttrImpl *attr = new AttrImpl(ownerDocument, name);
    attr->setValue(value);
    attr->setOwnerElement(this);
    attributes.setNamedItem(attr);
}

/**
 *
 */
void ElementImpl::removeAttribute(const DOMString& name)
                     throw(DOMException)
{
    attributes.removeNamedItem(name);
}

/**
 *
 */
Attr *ElementImpl::getAttributeNode(const DOMString& name)
{
    Node *node = attributes.getNamedItem(name);
    if (!node || node->getNodeType() != ATTRIBUTE_NODE)
        return NULL;
    Attr *attr = dynamic_cast<Attr *>(node);
    return attr;
}

/**
 *
 */
Attr *ElementImpl::setAttributeNode(Attr *attr)
                      throw(DOMException)
{
    attributes.setNamedItem(attr);
    return attr;
}

/**
 *
 */
Attr *ElementImpl::removeAttributeNode(Attr *attr)
                         throw(DOMException)
{
    if (!attr)
        return NULL;
    attributes.removeNamedItem(attr->getName());
    return attr;
}


/**
 *
 */
void ElementImpl::getElementsByTagNameRecursive(NodeList &list,
                        const DOMString& name, Element *elem)
{
    if (!elem)
        return;

    if (name == elem->getTagName())
        list.add(elem);
    for (Node *node = elem->getFirstChild() ; node ; node=node->getNextSibling())
        {
        if (node->getNodeType() != Node::ELEMENT_NODE)
            continue;
        Element *childElem = dynamic_cast<Element *>(node);
        getElementsByTagNameRecursive(list, name, childElem);
        }
}


/**
 *
 */
NodeList ElementImpl::getElementsByTagName(const DOMString& tagName)
{
    NodeList list;
    getElementsByTagNameRecursive(list, tagName, this);
    return list;
}

/**
 *
 */
DOMString ElementImpl::getAttributeNS(const DOMString& namespaceURI,
                         const DOMString& localName)
{
    Node *node = attributes.getNamedItemNS(namespaceURI, localName);
    if (!node || node->getNodeType()!=ATTRIBUTE_NODE)
        return DOMString("");
    Attr *attr = dynamic_cast<Attr *>(node);
    return attr->getValue();
}

/**
 *
 */
void ElementImpl::setAttributeNS(const DOMString& namespaceURI,
                    const DOMString& qualifiedName,
                    const DOMString& value)
                    throw(DOMException)
{
    AttrImpl *attr = new AttrImpl(ownerDocument, namespaceURI, qualifiedName);
    attr->setValue(value);
    attr->setOwnerElement(this);
    attributes.setNamedItemNS(attr);
}

/**
 *
 */
void ElementImpl::removeAttributeNS(const DOMString& namespaceURI,
                       const DOMString& localName)
                       throw(DOMException)
{
    attributes.removeNamedItemNS(namespaceURI, localName);
}

/**
 *
 */
 Attr *ElementImpl::getAttributeNodeNS(const DOMString& namespaceURI,
                        const DOMString& localName)
{
    Node *node = attributes.getNamedItemNS(namespaceURI, localName);
    if (!node || node->getNodeType() != ATTRIBUTE_NODE)
        return NULL;
    Attr *attr = dynamic_cast<Attr *>(node);
    return attr;
}

/**
 *
 */
Attr *ElementImpl::setAttributeNodeNS(Attr *attr)
                        throw(DOMException)
{
    attributes.setNamedItemNS(attr);
    return attr;
}


/**
 *
 */
void ElementImpl::getElementsByTagNameNSRecursive(NodeList &list,
             const DOMString& namespaceURI, const DOMString& tagName, Element *elem)
{
    if (!elem)
        return;

    if (namespaceURI == elem->getNamespaceURI() && tagName == elem->getTagName())
        list.add(elem);
    for (Node *node = elem->getFirstChild() ; node ; node=node->getNextSibling())
        {
        if (node->getNodeType() != Node::ELEMENT_NODE)
            continue;
        Element *childElem = dynamic_cast<Element *>(node);
        getElementsByTagNameNSRecursive(list, namespaceURI, tagName, childElem);
        }
}

/**
 *
 */
NodeList ElementImpl::getElementsByTagNameNS(const DOMString& namespaceURI,
                                const DOMString& localName)
{
    NodeList list;
    getElementsByTagNameNSRecursive(list, namespaceURI, localName, this);
    return list;
}

/**
 *
 */
bool ElementImpl::hasAttribute(const DOMString& attrName)
{
    Node *node = attributes.getNamedItem(attrName);
    if (!node || node->getNodeType() != ATTRIBUTE_NODE)
        return false;
    return true;
}

/**
 *
 */
bool ElementImpl::hasAttributeNS(const DOMString& namespaceURI,
                    const DOMString& localName)
{
    Node *node = attributes.getNamedItemNS(namespaceURI, localName);
    if (!node || node->getNodeType() != ATTRIBUTE_NODE)
        return false;
    return true;
}

/**
 *
 */
TypeInfo *ElementImpl::getSchemaTypeInto()
{
    //fixme
    return NULL;
}


/**
 *
 */
void ElementImpl::setIdAttribute(const DOMString &name,
                            bool isId) throw (DOMException)
{
    //fixme
}

/**
 *
 */
void ElementImpl::setIdAttributeNS(const DOMString &namespaceURI,
                              const DOMString &localName,
                              bool isId) throw (DOMException)
{
    //fixme
}

/**
 *
 */
void ElementImpl::setIdAttributeNode(const Attr *idAttr,
                                bool isId) throw (DOMException)
{
    //fixme
}


//##################
//# Non-API methods
//##################


/**
 *
 */
ElementImpl::ElementImpl() : NodeImpl()
{
    nodeType = ELEMENT_NODE;
}

/**
 *
 */
ElementImpl::ElementImpl(DocumentImpl *owner, const DOMString &tagName)
                                  : NodeImpl()
{
    nodeType = ELEMENT_NODE;
    ownerDocument = owner;
    setNodeName(tagName);
}

/**
 *
 */
ElementImpl::ElementImpl(DocumentImpl *owner,
                         const DOMString &theNamespaceURI,
                         const DOMString &qualifiedName) :
                         NodeImpl()
{
    nodeType = ELEMENT_NODE;
    ownerDocument = owner;
    setNodeName(qualifiedName);
}

/**
 *
 */
ElementImpl::~ElementImpl()
{
}


/**
 *
 */
void ElementImpl::normalizeNamespaces()
{
    //printf("### NORMALIZE\n");

    NamedNodeMap attrs = getAttributes();

    //#######################################
    //# Pick up local namespace declarations
    //#######################################
    bindingsClear();  //Reset bindings on this node

    int nrAttrs = attrs.getLength();
    for (int i=0; i<nrAttrs ; i++)
        {
        Node *attrNode = attrs.item(i);
        if (attrNode->getNodeType() != Node::ATTRIBUTE_NODE)
            continue;
        AttrImpl *attr = dynamic_cast<AttrImpl *>(attrNode);
        DOMString attrNS     = attr->getNamespaceURI();
        DOMString attrName   = attr->getLocalName();
        DOMString attrPrefix = attr->getPrefix();
        DOMString attrValue  = attr->getNodeValue();
        if (attrName != "xmlns" && attrPrefix != "xmlns")
            continue;

        //is the namespace declaration is invalid?
        if (attrValue == XMLNSNAME || attrName == attrPrefix)
            {
            // Note: The prefix xmlns is used only to declare namespace bindings and
            // is by definition bound to the namespace name http://www.w3.org/2000/xmlns/.
            // It must not be declared. No other prefix may be bound to this namespace name.

            //==> Report an error.
            printf("normalizeNamespaces() error: Namespace %s cannot be reassigned\n",
                        XMLNSNAME);

            }
        else
            {
            //==>  Record the namespace declaration
            attr->setNamespaceURI(XMLNSNAME);
            if (attrPrefix.size() > 0)
                bindingsAdd(attrPrefix, attrValue);
            else
                bindingsAdd("*", attrValue);//default

            }
        }


    //#######################################
    //# Fixup element's namespace
    //#######################################
    if ( namespaceURI.size() > 0 )
        {
        DOMString key = prefix;
        if (key.size() == 0)
            key = "*";
        DOMString binding = bindingsFind(key);
        //Element's prefix/namespace pair (or default namespace, if no prefix)
        // are within the scope of a binding
        if ( binding == namespaceURI )
            {
            //==> do nothing, declaration in scope is inherited

            // See section "B.1.1: Scope of a binding" for an example

            }
        else
            {

            /*
            ==> Create a local namespace declaration attr for this namespace,
            with Element's current prefix (or a default namespace, if
            no prefix). If there's a conflicting local declaration
            already present, change its value to use this namespace.

            See section "B.1.2: Conflicting namespace declaration" for an example
            */
            DOMString attrName = "xmlns";
            if (prefix.size() > 0)
                {
                attrName.append(":");
                attrName.append(prefix);
                }
            setAttribute(attrName, namespaceURI);
            // NOTE that this may break other nodes within this Element's
            // subtree, if they're already using this prefix.
            // They will be repaired when we reach them.
            }
        }
    else  // Element has no namespace URI:
        {
        //###############################################
        //# Bob -- alter this from the specs a bit.
        //#  Since the XmlReader does not set namespaces,
        //#    do it here
        //###############################################
        DOMString localName = getLocalName();
        if ( localName.size()==0 )
            {
            // DOM Level 1 node
            /*
            ==> if in process of validation against a namespace aware schema
            (i.e XML Schema) report a fatal error: the processor can not recover
            in this situation.
            Otherwise, report an error: no namespace fixup will be performed on this node.
            */
            printf("normalizeNamespaces() error: no localName\n");
            }
        else
            {
            // Element has no pseudo-prefix
            //there's a conflicting local default namespace declaration already present
            if ( prefix.size()==0 )
                {
                //==> change its value to use this empty namespace.
                namespaceURI = bindingsFind("*");
                //setAttribute("xmlns", "");
                }
            else  //#BOB .   I added this.
                {
                namespaceURI = bindingsFind(prefix);
                }
            // NOTE that this may break other nodes within this Element's
            // subtree, if they're already using the default namespaces.
            // They will be repaired when we reach them.
            }
        }


    //#######################################
    //# Examine and polish the attributes
    //#######################################
    nrAttrs = attrs.getLength();
    for (int i=0; i<nrAttrs ; i++)// all non-namespace Attrs of Element
        {
        Node *attrNode = attrs.item(i);
        if (attrNode->getNodeType() != Node::ATTRIBUTE_NODE)
            continue;
        Attr *attr = dynamic_cast<Attr *>(attrNode);
        DOMString attrNS     = attr->getNamespaceURI();
        DOMString attrPrefix = attr->getPrefix();
        DOMString attrValue  = attr->getNodeValue();
        if (attrNS == XMLNSNAME)
            continue;

        if ( attrNS.size()>0 ) //Attr[i] has a namespace URI
            {
            DOMString attrBinding = bindingsFind(attrPrefix);
            /*
             if attribute has no prefix (default namespace decl does not apply to attributes)
             OR
             attribute prefix is not declared
             OR
             conflict: attribute has a prefix that conflicts with a binding
                       already active in scope
            */
            if ( attrPrefix.size() == 0 || attrBinding.size() == 0)
                {
                //namespaceURI matches an in scope declaration of one or more prefixes)
                DOMString prefixForNS = lookupNamespacePrefix(attrNS, this);
                if ( prefixForNS.size() > 0 )
                    {
                    // pick the most local binding available;
                    // if there is more than one pick one arbitrarily

                    //==> change attribute's prefix.
                    attr->setPrefix(prefixForNS);
                    }
                else
                    {
                    // the current prefix is not null and it has no in scope declaration)
                    if ( attrPrefix.size() > 0 || attrBinding.size() == 0 )
                        {
                        //==> declare this prefix
                        DOMString newAttrName = "xmlns:";
                        newAttrName.append(attrPrefix);
                        setAttribute(newAttrName, attrNS);
                        bindingsAdd(attrPrefix, attrNS);
                        }
                    else
                        {
                        // find a prefix following the pattern "NS" +index (starting at 1)
                        // make sure this prefix is not declared in the current scope.
                        // create a local namespace declaration attribute

                        //==> declare this prefix
                        char buf[16];
                        sprintf(buf, "%d" , ownerDocument->namespaceIndex++);
                        DOMString newPrefix = "NS";
                        newPrefix.append(buf);
                        DOMString newAttrName = "xmlns:";
                        newAttrName.append(newPrefix);
                        setAttribute(newAttrName, attrNS);
                        bindingsAdd(newPrefix, attrNS);
                        //==> change attribute's prefix.
                        }
                    }
                }
            }
        else  // Attr has no namespace URI
            {
            // Attr has no localName
            if ( attr->getLocalName().size() == 0 )
                {
                // DOM Level 1 node
                /*
                ==> if in process of validation against a namespace aware schema
                (i.e XML Schema) report a fatal error: the processor can not recover
                in this situation.
                Otherwise, report an error: no namespace fixup will be performed on this node.
                */
                printf("normalizeNamespaces:  no local name for attribute\n");
                }
            else
                {
                // attr has no namespace URI and no prefix
                // no action is required, since attrs don't use default
                //==> do nothing
                }
            }
        } // end for-all-Attrs


    //#######################################
    //# Recursively normalize children
    //#######################################
    for (Node *child=getFirstChild() ; child ; child=child->getNextSibling())
        {
        if (child->getNodeType() != Node::ELEMENT_NODE)
            continue;
        ElementImpl *childElement = dynamic_cast<ElementImpl *>(child);
        childElement->normalizeNamespaces();
        }

}


/*#########################################################################
## TextImpl
#########################################################################*/


/**
 *
 */
TextImpl::TextImpl() : CharacterDataImpl()
{
    nodeType = TEXT_NODE;
    nodeName = "#text";
}


/**
 *
 */
TextImpl::TextImpl(DocumentImpl *owner, const DOMString &value)
                               : CharacterDataImpl()
{
    nodeType      = TEXT_NODE;
    nodeName      = "#text";
    ownerDocument = owner;
    nodeValue     = value;
}


/**
 *
 */
TextImpl::~TextImpl()
{
}

/**
 *
 */
Text *TextImpl::splitText(unsigned long offset)
                throw(DOMException)
{
    return NULL;
}

/**
 *
 */
bool TextImpl::getIsElementContentWhitespace()
{
    return false;
}

/**
 *
 */
DOMString TextImpl::getWholeText()
{
    return nodeValue;
}


/**
 *
 */
Text *TextImpl::replaceWholeText(const DOMString &content)
                             throw(DOMException)
{
    return NULL;
}


/*#########################################################################
## CommentImpl
#########################################################################*/

/**
 *
 */
CommentImpl::CommentImpl() : CharacterDataImpl()
{
    nodeType = COMMENT_NODE;
    nodeName = "#comment";
}


/**
 *
 */
CommentImpl::CommentImpl(DocumentImpl *owner, const DOMString &value)
                       : CharacterDataImpl()
{
    nodeType      = COMMENT_NODE;
    nodeName      = "#comment";
    ownerDocument = owner;
    nodeValue     = value;
}


/**
 *
 */
CommentImpl::~CommentImpl()
{
}




/*#########################################################################
## TypeInfoImpl
#########################################################################*/


/**
 *
 */
TypeInfoImpl::TypeInfoImpl(const DOMString &typeNamespaceArg,
                 const DOMString &typeNameArg,
                 const DerivationMethod derivationMethodArg)
{
    typeNamespace    = typeNamespaceArg;
    typeName         = typeNameArg;
    derivationMethod = derivationMethodArg;
}


/**
 *
 */
TypeInfoImpl::~TypeInfoImpl()
{
}


/**
 *
 */
DOMString TypeInfoImpl::getTypeName()
{
    return typeName;
}

/**
 *
 */
DOMString TypeInfoImpl::getTypeNamespace()
{
    return typeNamespace;
}

/**
 *
 */
bool TypeInfoImpl::isDerivedFrom(const DOMString &typeNamespaceArg,
                           const DOMString &typeNameArg,
                           const DerivationMethod derivationMethodArg)
{
    if (typeNamespaceArg == typeNamespace &&
        typeName         == typeNameArg &&
        derivationMethod == derivationMethodArg)
        return true;
    return false;
}



/*#########################################################################
## UserDataHandlerImpl
#########################################################################*/



/**
 *
 */
UserDataHandlerImpl::UserDataHandlerImpl()
{
}


/**
 *
 */
UserDataHandlerImpl::~UserDataHandlerImpl()
{
}

/**
 *
 */
void UserDataHandlerImpl::handle(unsigned short operation,
                     const DOMString &key,
                     const DOMUserData *data,
                     const Node *src,
                     const Node *dst)
{
    //do nothing.  do we need anything here?
}



/*#########################################################################
## DOMErrorImpl
#########################################################################*/



/**
 *
 */
DOMErrorImpl::DOMErrorImpl()
{
}


/**
 *
 */
DOMErrorImpl::~DOMErrorImpl()
{
}

/**
 *
 */
unsigned short DOMErrorImpl::getSeverity()
{
    return severity;
}

/**
 *
 */
DOMString DOMErrorImpl::getMessage()
{
    return message;
}

/**
 *
 */
DOMString DOMErrorImpl::getType()
{
    return type;
}

/**
 *
 */
DOMObject *DOMErrorImpl::getRelatedException()
{
    return NULL;
}

/**
 *
 */
DOMObject *DOMErrorImpl::getRelatedData()
{
    return NULL;
}

/**
 *
 */
DOMLocator *DOMErrorImpl::getLocation()
{
    //really should fill this in
    return NULL;
}




/*#########################################################################
## DOMErrorHandlerImpl
#########################################################################*/



/**
 *
 */
DOMErrorHandlerImpl::DOMErrorHandlerImpl()
{
}


/**
 *
 */
DOMErrorHandlerImpl::~DOMErrorHandlerImpl()
{
}

/**
 *
 */
bool DOMErrorHandlerImpl::handleError(const DOMError *error)
{
    if (!error)
        return false;
    return true;
}




/*#########################################################################
## DOMLocatorImpl
#########################################################################*/


/**
 *
 */
DOMLocatorImpl::DOMLocatorImpl()
{
}


/**
 *
 */
DOMLocatorImpl::~DOMLocatorImpl()
{
}


/**
 *
 */
long DOMLocatorImpl::getLineNumber()
{
    return lineNumber;
}

/**
 *
 */
long DOMLocatorImpl::getColumnNumber()
{
    return columnNumber;
}

/**
 *
 */
long DOMLocatorImpl::getByteOffset()
{
    return byteOffset;
}

/**
 *
 */
long DOMLocatorImpl::getUtf16Offset()
{
    return utf16Offset;
}


/**
 *
 */
Node *DOMLocatorImpl::getRelatedNode()
{
    return relatedNode;
}


/**
 *
 */
DOMString DOMLocatorImpl::getUri()
{
    return uri;
}



/*#########################################################################
## DOMConfigurationImpl
#########################################################################*/


/**
 *
 */
DOMConfigurationImpl::DOMConfigurationImpl()
{
}


/**
 *
 */
DOMConfigurationImpl::~DOMConfigurationImpl()
{
}

/**
 *
 */
void DOMConfigurationImpl::setParameter(const DOMString &name,
                          const DOMUserData *value) throw (DOMException)
{
}

/**
 *
 */
DOMUserData *DOMConfigurationImpl::getParameter(const DOMString &name)
                                  throw (DOMException)
{
    return NULL;
}

/**
 *
 */
bool DOMConfigurationImpl::canSetParameter(const DOMString &name,
                             const DOMUserData *data)
{
    return false;
}

/**
 *
 */
DOMStringList *DOMConfigurationImpl::getParameterNames()
{
    return NULL;
}



/*#########################################################################
## CDATASectionImpl
#########################################################################*/

/**
 *
 */
CDATASectionImpl::CDATASectionImpl() : TextImpl()
{
    nodeType = CDATA_SECTION_NODE;
    nodeName = "#cdata-section";
}

/**
 *
 */
CDATASectionImpl::CDATASectionImpl(DocumentImpl *owner, const DOMString &theValue)
                                 : TextImpl()
{
    nodeType      = CDATA_SECTION_NODE;
    nodeName      = "#cdata-section";
    ownerDocument = owner;
    nodeValue     = theValue;
}


/**
 *
 */
CDATASectionImpl::~CDATASectionImpl()
{
}





/*#########################################################################
## DocumentTypeImpl
#########################################################################*/

/**
 *
 */
DocumentTypeImpl::DocumentTypeImpl(const DOMString& theName,
                                   const DOMString& thePublicId,
                                   const DOMString& theSystemId)
                                  : NodeImpl()
{
    nodeType = DOCUMENT_TYPE_NODE;
    nodeName = theName;
    publicId = thePublicId;
    systemId = theSystemId;
}

/**
 *
 */
DocumentTypeImpl::~DocumentTypeImpl()
{
}

/**
 *
 */
DOMString DocumentTypeImpl::getName()
{
    return nodeName;
}

/**
 *
 */
NamedNodeMap DocumentTypeImpl::getEntities()
{
    return entities;
}

/**
 *
 */
NamedNodeMap DocumentTypeImpl::getNotations()
{
    return notations;
}

/**
 *
 */
DOMString DocumentTypeImpl::getPublicId()
{
    return publicId;
}

/**
 *
 */
DOMString DocumentTypeImpl::getSystemId()
{
    return systemId;
}

/**
 *
 */
DOMString DocumentTypeImpl::getInternalSubset()
{
    return DOMString("");
}






/*#########################################################################
## NotationImpl
#########################################################################*/



/**
 *
 */
NotationImpl::NotationImpl(DocumentImpl *owner) : NodeImpl()
{
    nodeType = NOTATION_NODE;
    ownerDocument = owner;
}


/**
 *
 */
NotationImpl::~NotationImpl()
{
}

/**
 *
 */
DOMString NotationImpl::getPublicId()
{
    return publicId;
}

/**
 *
 */
DOMString NotationImpl::getSystemId()
{
    return systemId;
}








/*#########################################################################
## EntityImpl
#########################################################################*/


/**
 *
 */
EntityImpl::EntityImpl() : NodeImpl()
{
    nodeType = ENTITY_NODE;
}


/**
 *
 */
EntityImpl::EntityImpl(DocumentImpl *owner) : NodeImpl()
{
    nodeType = ENTITY_NODE;
    ownerDocument = owner;
}


/**
 *
 */
EntityImpl::~EntityImpl()
{
}

/**
 *
 */
DOMString EntityImpl::getPublicId()
{
    return publicId;
}

/**
 *
 */
DOMString EntityImpl::getSystemId()
{
    return systemId;
}

/**
 *
 */
DOMString EntityImpl::getNotationName()
{
    return notationName;
}

/**
 *
 */
DOMString EntityImpl::getInputEncoding()
{
    return inputEncoding;
}

/**
 *
 */
DOMString EntityImpl::getXmlEncoding()
{
    return xmlEncoding;
}

/**
 *
 */
DOMString EntityImpl::getXmlVersion()
{
    return xmlVersion;
}






/*#########################################################################
## EntityReferenceImpl
#########################################################################*/



/**
 *
 */
EntityReferenceImpl::EntityReferenceImpl() : NodeImpl()
{
    nodeType = ENTITY_REFERENCE_NODE;
}


/**
 *
 */
EntityReferenceImpl::EntityReferenceImpl(DocumentImpl *owner,
                                         const DOMString &theName)
                                         : NodeImpl()
{
    nodeType = ENTITY_REFERENCE_NODE;
    nodeName = theName;
    ownerDocument = owner;
}


/**
 *
 */
EntityReferenceImpl::~EntityReferenceImpl()
{
}



/*#########################################################################
## ProcessingInstructionImpl
#########################################################################*/




/**
 *
 */
ProcessingInstructionImpl::ProcessingInstructionImpl(): NodeImpl()
{
    nodeType = PROCESSING_INSTRUCTION_NODE;
}



/**
 *
 */
ProcessingInstructionImpl::ProcessingInstructionImpl(DocumentImpl *owner,
                                                     const DOMString &target,
                                                     const DOMString &data)
                                                     : NodeImpl()
{
    nodeType      = PROCESSING_INSTRUCTION_NODE;
    ownerDocument = owner;
    nodeName      = target;
    nodeValue     = data;
}


/**
 *
 */
ProcessingInstructionImpl::~ProcessingInstructionImpl()
{
}




/**
 *
 */
DOMString ProcessingInstructionImpl::getTarget()
{
    return nodeName;
}

/**
 *
 */
DOMString ProcessingInstructionImpl::getData()
{
    return nodeValue;
}

/**
 *
 */
void ProcessingInstructionImpl::setData(const DOMString& val) throw(DOMException)
{
     //do something here
}







/*#########################################################################
## DocumentFragmentImpl
#########################################################################*/

/**
 *
 */
DocumentFragmentImpl::DocumentFragmentImpl() : NodeImpl()
{
    nodeType = DOCUMENT_FRAGMENT_NODE;
    nodeName = "#document-fragment";
}


/**
 *
 */
DocumentFragmentImpl::DocumentFragmentImpl(DocumentImpl *owner) : NodeImpl()
{
    nodeType = DOCUMENT_FRAGMENT_NODE;
    nodeName = "#document-fragment";
    ownerDocument = owner;
}


/**
 *
 */
DocumentFragmentImpl::~DocumentFragmentImpl()
{
}






/*#########################################################################
## DocumentImpl
#########################################################################*/



/**
 *
 */
DocumentType *DocumentImpl::getDoctype()
{
    return doctype;
}

/**
 *
 */
DOMImplementation *DocumentImpl::getImplementation()
{
    return parent;
}

/**
 *
 */
Element *DocumentImpl::getDocumentElement()
{
    return documentElement;
}

/**
 *
 */
Element *DocumentImpl::createElement(const DOMString& tagName)
                       throw(DOMException)
{
    ElementImpl *impl = new ElementImpl(this, tagName);
    return impl;
}

/**
 *
 */
DocumentFragment *DocumentImpl::createDocumentFragment()
{
    DocumentFragmentImpl *frag = new DocumentFragmentImpl(this);
    return frag;
}

/**
 *
 */
Text *DocumentImpl::createTextNode(const DOMString& data)
{
    TextImpl *text = new TextImpl(this, data);
    return text;
}

/**
 *
 */
Comment *DocumentImpl::createComment(const DOMString& data)
{
    CommentImpl *comment = new CommentImpl(this, data);
    return comment;
}

/**
 *
 */
CDATASection *DocumentImpl::createCDATASection(const DOMString& data)
                                 throw(DOMException)
{
    CDATASectionImpl *cdata = new CDATASectionImpl(this, data);
    return cdata;
}

/**
 *
 */
ProcessingInstruction *DocumentImpl::createProcessingInstruction(const DOMString& target,
                                                   const DOMString& data)
                                                   throw(DOMException)
{
    ProcessingInstructionImpl *cdata =
        new ProcessingInstructionImpl(this, target, data);
    return cdata;
}

/**
 *
 */
Attr *DocumentImpl::createAttribute(const DOMString& attrName)
                      throw(DOMException)
{
    AttrImpl *attr = new AttrImpl(this, attrName);
    return attr;
}

/**
 *
 */
EntityReference *DocumentImpl::createEntityReference(const DOMString& erName)
                                       throw(DOMException)
{
    EntityReferenceImpl *ref = new EntityReferenceImpl(this, erName);
    return ref;
}


/**
 *
 */
NodeList DocumentImpl::getElementsByTagName(const DOMString& tagname)
{
    NodeList list;
    ElementImpl::getElementsByTagNameRecursive(list,
                     tagname, documentElement);
    return list;
}


/**
 *
 */
Node *DocumentImpl::importNode(const Node *importedNode,
                 bool deep)
                 throw(DOMException)
{
    return NULL;
}

/**
 *
 */
Element *DocumentImpl::createElementNS(const DOMString& namespaceURI,
                         const DOMString& qualifiedName)
                         throw(DOMException)
{
    ElementImpl *elem = new ElementImpl(this, namespaceURI, qualifiedName);
    return elem;
}

/**
 *
 */
Attr *DocumentImpl::createAttributeNS(const DOMString& namespaceURI,
                        const DOMString& qualifiedName)
                        throw(DOMException)
{
    AttrImpl *attr = new AttrImpl(this, namespaceURI, qualifiedName);
    return attr;
}


/**
 *
 */
NodeList DocumentImpl::getElementsByTagNameNS(const DOMString& namespaceURI,
                                 const DOMString& localName)
{
    NodeList list;
    ElementImpl::getElementsByTagNameNSRecursive(list, namespaceURI,
                          localName, documentElement);
    return list;
}

/**
 *
 */
Element *DocumentImpl::getElementById(const DOMString& elementId)
{
    for (NamedElementItem *entry = elementsById.next; entry ; entry=entry->next)
        if (entry->name == elementId)
            return entry->elem;
    return NULL;
}


/**
 *
 */
DOMString DocumentImpl::getInputEncoding()
{
    return inputEncoding;
}


/**
 *
 */
DOMString DocumentImpl::getXmlEncoding()
{
    return xmlEncoding;
}

/**
 *
 */
bool DocumentImpl::getXmlStandalone()
{
    return xmlStandalone;
}

/**
 *
 */
void DocumentImpl::setXmlStandalone(bool val) throw (DOMException)
{
    xmlStandalone = val;
}

/**
 *
 */
DOMString DocumentImpl::getXmlVersion()
{
    return xmlVersion;
}

/**
 *
 */
void DocumentImpl::setXmlVersion(const DOMString &version) throw (DOMException)
{
    xmlVersion = version;
}

/**
 *
 */
bool DocumentImpl::getStrictErrorChecking()
{
    return strictErrorChecking;
}

/**
 *
 */
void DocumentImpl::setStrictErrorChecking(bool val)
{
    strictErrorChecking = val;
}


/**
 *
 */
DOMString DocumentImpl::getDocumentURI()
{
    if (!documentURI)
        return DOMString("");
    DOMString docURI = *documentURI;
    return docURI;
}

/**
 *
 */
void DocumentImpl::setDocumentURI(const DOMString &uri)
{
    //documentURI = stringCache(uri);
}

/**
 *
 */
Node *DocumentImpl::adoptNode(const Node *source) throw (DOMException)
{
    return (Node *)source;
}

/**
 *
 */
DOMConfiguration *DocumentImpl::getDomConfig()
{
    return domConfig;
}

/**
 *
 */
void DocumentImpl::normalizeDocument()
{
    //i assume that this means adjusting namespaces & prefixes
    if (documentElement)
        documentElement->normalizeNamespaces();
}

/**
 *
 */
Node *DocumentImpl::renameNode(const Node *n,
                               const DOMString &namespaceURI,
                               const DOMString &qualifiedName)
                               throw (DOMException)
{
    Node *node = (Node *) n;
    NodeImpl *nodeImpl = dynamic_cast<NodeImpl *> (node);
    //nodeImpl->namespaceURI = stringCache(namespaceURI);
    nodeImpl->setNodeName(qualifiedName);
    return node;
}



//##################
//# Non-API methods
//##################

/**
 *
 */
DocumentImpl::DocumentImpl(const DOMImplementation *domImpl,
                 const DOMString &theNamespaceURI,
                 const DOMString &theQualifiedName,
                 const DocumentType *theDoctype) : NodeImpl()
{
    nodeType        = DOCUMENT_NODE;
    nodeName        = "#document";
    parent          = (DOMImplementation *)domImpl;
    //documentURI     = stringCache(theNamespaceURI);
    qualifiedName   = theQualifiedName;
    if (theDoctype) //only assign if not null.
        doctype     = (DocumentType *)theDoctype;
    else
        doctype     = new DocumentTypeImpl("", "", "");
    documentElement = new ElementImpl(this, "root");
    namespaceIndex  = 0;
}


/**
 *
 */
DocumentImpl::~DocumentImpl()
{
    delete documentElement;
}












}  //namespace dom
}  //namespace w3c
}  //namespace org



/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/




