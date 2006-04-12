#ifndef __DOM_H__
#define __DOM_H__
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

#include <string>
#include <vector>

#define OWN_STRING

#ifdef OWN_STRING
#include "domstring.h"
#else
#include <glibmm.h>
typedef Glib::ustring DOMString
#endif

#define XMLNSNAME "http://www.w3.org/2000/xmlns/"

namespace org
{
namespace w3c
{
namespace dom
{



#ifndef OWN_STRING
typedef unsigned short XMLCh;
typedef std::string DOMString;
#endif

/**
 *
 */
typedef unsigned long long DOMTimeStamp;

/**
 *
 */
typedef void DOMUserData;

/**
 *
 */
typedef void DOMObject;


class DOMException;
class DOMStringList;
class NameList;
class DOMImplementationList;
class DOMImplementationSource;
class DOMImplementation;
class Node;
class NodeList;
class NamedNodeMap;
class CharacterData;
class Attr;
class Element;
class Text;
class Comment;
class TypeInfo;
class UserDataHandler;
class DOMError;
class DOMErrorHandler;
class DOMLocator;
class DOMConfiguration;
class CDATASection;
class DocumentType;
class Notation;
class Entity;
class EntityReference;
class ProcessingInstruction;
class DocumentFragment;
class Document;


/**
 * NOTE: We were originally intending to split ALL specifications into
 * interface and implementation.   After consideration, though, it behooves
 * us to simplify things by implementing the base exception and
 * container classes directly:
 *
 * DOMException
 * DOMStringList
 * NameList
 * DOMImplementationList
 * DOMImplementationSource
 * DOMImplementation
 * NodeList
 * NamedNodeMap
 */


/*#########################################################################
## DOMException
#########################################################################*/
/**
 *  This is the only non-interface class
 */
class DOMException
{

public:

    DOMException(const DOMString &reasonMsg)
        { msg = reasonMsg; }

    DOMException(short theCode)
        {
        code = theCode;
        }

    virtual ~DOMException() throw()
       {}

    /**
     *
     */
    unsigned short code;

    /**
     *
     */
    DOMString msg;

    /**
     * Get a string, translated from the code.
     * Like std::exception. Not in spec.
     */
    const char *what()
        { return (const char *)msg.c_str(); }



};




/**
 * ExceptionCode
 */
typedef enum
{
        INDEX_SIZE_ERR                 = 1,
        DOMSTRING_SIZE_ERR             = 2,
        HIERARCHY_REQUEST_ERR          = 3,
        WRONG_DOCUMENT_ERR             = 4,
        INVALID_CHARACTER_ERR          = 5,
        NO_DATA_ALLOWED_ERR            = 6,
        NO_MODIFICATION_ALLOWED_ERR    = 7,
        NOT_FOUND_ERR                  = 8,
        NOT_SUPPORTED_ERR              = 9,
        INUSE_ATTRIBUTE_ERR            = 10,
        INVALID_STATE_ERR              = 11,
        SYNTAX_ERR                     = 12,
        INVALID_MODIFICATION_ERR       = 13,
        NAMESPACE_ERR                  = 14,
        INVALID_ACCESS_ERR             = 15,
        VALIDATION_ERR                 = 16,
        TYPE_MISMATCH_ERR              = 17
} ExceptionCode;


/*#########################################################################
## DOMStringList
#########################################################################*/

class DOMStringList
{
public:

    /**
     *
     */
    virtual DOMString item(unsigned long index)
        {
        if (index>=strings.size())
            return "";
        return strings[index];
        }

    /**
     *
     */
    virtual unsigned long getLength()
        {
        return (unsigned long) strings.size();
        }

    /**
     *
     */
    virtual bool contains(const DOMString &str)
        {
        for (unsigned int i=0; i<strings.size() ; i++)
            {
            if (strings[i] == str)
                return true;
            }
        return false;
        }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    DOMStringList() {}


    /**
     *
     */
    DOMStringList(const DOMStringList &other)
        {
        strings = other.strings;
        }

    /**
     *
     */
    virtual ~DOMStringList() {}


protected:

    /**
     *
     */
    virtual void add(const DOMString &str)
        {
        strings.push_back(str);
        }

    std::vector<DOMString>strings;

};



/*#########################################################################
## NameList
#########################################################################*/
class NamePair
{
public:
    NamePair(const DOMString &theNamespaceURI, const DOMString &theName)
        {
        namespaceURI = theNamespaceURI;
        name         = theName;
        }
    NamePair(const NamePair &other)
        {
        namespaceURI = other.namespaceURI;
        name         = other.name;
        }
    virtual ~NamePair() {}

    DOMString namespaceURI;
    DOMString name;
};



class NameList
{
public:

    /**
     *
     */
    virtual DOMString getName(unsigned long index)
        {
        if (index>=namePairs.size())
            return "";
        return namePairs[index].name;
        }

    /**
     *
     */
    virtual DOMString getNamespaceURI(unsigned long index)
        {
        if (index>=namePairs.size())
            return "";
        return namePairs[index].namespaceURI;
        }

    /**
     *
     */
    virtual unsigned long getLength()
        {
        return (unsigned long)namePairs.size();
        }

    /**
     *
     */
    virtual bool contains(const DOMString &name)
        {
        for (unsigned int i=0; i<namePairs.size() ; i++)
            {
            if (namePairs[i].name == name )
                return true;
            }
        return false;
        }

    /**
     *
     */
    virtual bool containsNS(const DOMString namespaceURI,const DOMString &name)
        {
        for (unsigned int i=0; i<namePairs.size() ; i++)
            {
            if (namePairs[i].namespaceURI == namespaceURI ||
                namePairs[i].name         == name           )
                return true;
            }
        return false;
        }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    NameList() {}

    /**
     *
     */
    NameList(const NameList &other)
        {
        namePairs = other.namePairs;
        }

    /**
     *
     */
    virtual ~NameList() {}
protected:

    std::vector<NamePair> namePairs;

};

/*#########################################################################
## DOMImplementationList
#########################################################################*/

class DOMImplementationList
{
public:

    /**
     *
     */
    virtual DOMImplementation *getDOMImplementation(unsigned long index)
        {
        if (index >implementations.size())
            return NULL;
        return implementations[index];
        }

    /**
     *
     */
    virtual unsigned long getLength()
        {
        return (unsigned long) implementations.size();
        }




    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    DOMImplementationList() {}


    /**
     *
     */
    DOMImplementationList(const DOMImplementationList &other)
        {
        implementations = other.implementations;
        }

    /**
     *
     */
    virtual ~DOMImplementationList() {}

protected:

    std::vector<DOMImplementation *>implementations;

};


/*#########################################################################
## DOMImplementationSource
#########################################################################*/

class DOMImplementationSource
{
public:

    /**
     *
     */
    virtual DOMImplementation *getDOMImplementation(const DOMString &features) = 0;

    /**
     *
     */
    virtual DOMImplementationList getDOMImplementationList(const DOMString &features) = 0;

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~DOMImplementationSource() {}

};





/*#########################################################################
## DOMImplementation
#########################################################################*/
/**
 *
 */
class DOMImplementation
{
public:
    /**
     *
     */
    virtual bool hasFeature(const DOMString& feature, const DOMString& version) = 0;


    /**
     *
     */
    virtual DocumentType *createDocumentType(const DOMString& qualifiedName,
                                     const DOMString& publicId,
                                     const DOMString& systemId)
                                     throw(DOMException) = 0;

    /**
     *
     */
    virtual Document *createDocument(const DOMString& namespaceURI,
                             const DOMString& qualifiedName,
                             DocumentType *doctype)
                             throw(DOMException) = 0;
    /**
     *
     */
    virtual DOMObject *getFeature(const DOMString& feature,
                             const DOMString& version) = 0;


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~DOMImplementation() {}

};




/*#########################################################################
## Node
#########################################################################*/

/**
 *
 */
class Node
{
public:

    typedef enum
        {
        ELEMENT_NODE                   = 1,
        ATTRIBUTE_NODE                 = 2,
        TEXT_NODE                      = 3,
        CDATA_SECTION_NODE             = 4,
        ENTITY_REFERENCE_NODE          = 5,
        ENTITY_NODE                    = 6,
        PROCESSING_INSTRUCTION_NODE    = 7,
        COMMENT_NODE                   = 8,
        DOCUMENT_NODE                  = 9,
        DOCUMENT_TYPE_NODE             = 10,
        DOCUMENT_FRAGMENT_NODE         = 11,
        NOTATION_NODE                  = 12
        } NodeType;

    /**
     *
     */
    virtual DOMString getNodeName() = 0;

    /**
     *
     */
    virtual DOMString getNodeValue() throw (DOMException) = 0;

    /**
     *
     */
    virtual void setNodeValue(const DOMString& val) throw (DOMException) = 0;

    /**
     *
     */
    virtual unsigned short getNodeType() = 0;

    /**
     *
     */
    virtual Node *getParentNode() = 0;

    /**
     *
     */
    virtual NodeList getChildNodes() = 0;

    /**
     *
     */
    virtual Node *getFirstChild() = 0;

    /**
     *
     */
    virtual Node *getLastChild() = 0;

    /**
     *
     */
    virtual Node *getPreviousSibling() = 0;

    /**
     *
     */
    virtual Node *getNextSibling() = 0;

    /**
     *
     */
    virtual NamedNodeMap &getAttributes() = 0;


    /**
     *
     */
    virtual Document *getOwnerDocument() = 0;

    /**
     *
     */
    virtual Node *insertBefore(const Node *newChild,
                       const Node *refChild)
                       throw(DOMException) = 0;

    /**
     *
     */
    virtual Node *replaceChild(const Node *newChild,
                       const Node *oldChild)
                       throw(DOMException) = 0;

    /**
     *
     */
    virtual Node *removeChild(const Node *oldChild)
                      throw(DOMException) = 0;

    /**
     *
     */
    virtual Node *appendChild(const Node *newChild)
                      throw(DOMException) = 0;

    /**
     *
     */
    virtual bool hasChildNodes() = 0;

    /**
     *
     */
    virtual Node *cloneNode(bool deep) = 0;

    /**
     *
     */
    virtual void normalize() = 0;

    /**
     *
     */
    virtual bool isSupported(const DOMString& feature,
                     const DOMString& version) = 0;

    /**
     *
     */
    virtual DOMString getNamespaceURI() = 0;

    /**
     *
     */
    virtual DOMString getPrefix() = 0;

    /**
     *
     */
    virtual void setPrefix(const DOMString& val) throw(DOMException) = 0;

    /**
     *
     */
    virtual DOMString getLocalName() = 0;

    /**
     *
     */
    virtual bool hasAttributes() = 0;

    /**
     *
     */
    virtual DOMString getBaseURI() = 0;

    typedef enum
        {
        DOCUMENT_POSITION_DISCONNECTED            = 0x01,
        DOCUMENT_POSITION_PRECEDING               = 0x02,
        DOCUMENT_POSITION_FOLLOWING               = 0x04,
        DOCUMENT_POSITION_CONTAINS                = 0x08,
        DOCUMENT_POSITION_CONTAINED_BY            = 0x10,
        DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC = 0x20
        } DocumentPosition;


    /**
     *
     */
    virtual unsigned short compareDocumentPosition(const Node *other) = 0;

    /**
     *
     */
    virtual DOMString getTextContext() throw(DOMException) = 0;


    /**
     *
     */
    virtual void setTextContext(const DOMString &val) throw(DOMException) = 0;


    /**
     *
     */
    virtual DOMString lookupPrefix(const DOMString &namespaceURI) =0;


    /**
     *
     */
    virtual bool isDefaultNamespace(const DOMString &namespaceURI) =0;


    /**
     *
     */
    virtual DOMString lookupNamespaceURI(const DOMString &prefix) =0;


    /**
     *
     */
    virtual bool isEqualNode(const Node *node) =0;



    /**
     *
     */
    virtual DOMObject *getFeature(const DOMString &feature,
                                 const DOMString &version) =0;

    /**
     *
     */
    virtual DOMUserData *setUserData(const DOMString &key,
                                     const DOMUserData *data,
                                     const UserDataHandler *handler) =0;


    /**
     *
     */
    virtual DOMUserData *getUserData(const DOMString &namespaceURI) =0;

    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    virtual ~Node() {}




};



/*#########################################################################
## NodeList
#########################################################################*/

/**
 *
 */
class NodeList
{
public:
    /**
     *
     */
    virtual Node *item(unsigned long index)
        {
        if (index>=nodes.size())
            return NULL;
        return nodes[index];
        }

    /**
     *
     */
    virtual unsigned long getLength()
        {
        return (unsigned long) nodes.size();
        }

    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    NodeList() {}

    /**
     *
     */
    NodeList(const NodeList &other)
        {
        nodes = other.nodes;
        }

    /**
     *
     */
    virtual ~NodeList() {}

protected:

friend class NodeImpl;
friend class ElementImpl;

    /*
     *
     */
    virtual void add(const Node *node)
        {
        nodes.push_back((Node *)node);
        }

protected:

    std::vector<Node *> nodes;

};




/*#########################################################################
## NamedNodeMap
#########################################################################*/

class NamedNodeMapEntry
{
public:
    NamedNodeMapEntry(const DOMString &theNamespaceURI,
                      const DOMString &theName,
                      const Node      *theNode)
        {
        namespaceURI = theNamespaceURI;
        name         = theName;
        node         = (Node *)theNode;
        }
    NamedNodeMapEntry(const NamedNodeMapEntry &other)
        {
        namespaceURI = other.namespaceURI;
        name         = other.name;
        node         = other.node;
        }
    virtual ~NamedNodeMapEntry()
        {
        }
    DOMString namespaceURI;
    DOMString name;
    Node      *node;
};

/**
 *
 */
class NamedNodeMap
{
public:

    /**
     *
     */
    virtual Node *getNamedItem(const DOMString& name)
        {
        std::vector<NamedNodeMapEntry>::iterator iter;
        for (iter = entries.begin() ; iter!=entries.end() ; iter++)
            {
            if (iter->name == name)
                {
                Node *node = iter->node;
                return node;
                }
            }
        return NULL;
        }

    /**
     *
     */
    virtual Node *setNamedItem(Node *arg) throw(DOMException)
        {
        if (!arg)
            return NULL;
        DOMString namespaceURI = arg->getNamespaceURI();
        DOMString name         = arg->getNodeName();
        std::vector<NamedNodeMapEntry>::iterator iter;
        for (iter = entries.begin() ; iter!=entries.end() ; iter++)
            {
            if (iter->name == name)
                {
                Node *node = iter->node;
                iter->node = arg;
                return node;
                }
            }
        NamedNodeMapEntry entry(namespaceURI, name, arg);
        entries.push_back(entry);
        return arg;
        }


    /**
     *
     */
    virtual Node *removeNamedItem(const DOMString& name) throw(DOMException)
        {
        std::vector<NamedNodeMapEntry>::iterator iter;
        for (iter = entries.begin() ; iter!=entries.end() ; iter++)
            {
            if (iter->name == name)
                {
                Node *node = iter->node;
                entries.erase(iter);
                return node;
                }
            }
        return NULL;
        }

    /**
     *
     */
    virtual Node *item(unsigned long index)
        {
        if (index>=entries.size())
            return NULL;
        return entries[index].node;
        }

    /**
     *
     */
    virtual unsigned long getLength()
        {
        return (unsigned long)entries.size();
        }

    /**
     *
     */
    virtual Node *getNamedItemNS(const DOMString& namespaceURI,
                                 const DOMString& localName)
        {
        std::vector<NamedNodeMapEntry>::iterator iter;
        for (iter = entries.begin() ; iter!=entries.end() ; iter++)
            {
            if (iter->namespaceURI == namespaceURI && iter->name == localName)
                {
                Node *node = iter->node;
                return node;
                }
            }
        return NULL;
        }

    /**
     *
     */
    virtual Node *setNamedItemNS(Node *arg) throw(DOMException)
        {
        if (!arg)
            return NULL;
        DOMString namespaceURI = arg->getNamespaceURI();
        DOMString name         = arg->getNodeName();
        std::vector<NamedNodeMapEntry>::iterator iter;
        for (iter = entries.begin() ; iter!=entries.end() ; iter++)
            {
            if (iter->namespaceURI == namespaceURI && iter->name == name)
                {
                Node *node = iter->node;
                iter->node = arg;
                return node;
                }
            }
        NamedNodeMapEntry entry(namespaceURI, name, arg);
        entries.push_back(entry);
        return arg;
        }

    /**
     *
     */
    virtual Node *removeNamedItemNS(const DOMString& namespaceURI,
                                    const DOMString& localName)
                                    throw(DOMException)
        {
        std::vector<NamedNodeMapEntry>::iterator iter;
        for (iter = entries.begin() ; iter!=entries.end() ; iter++)
            {
            if (iter->namespaceURI == namespaceURI && iter->name == localName)
                {
                Node *node = iter->node;
                entries.erase(iter);
                return node;
                }
            }
        return NULL;
        }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    NamedNodeMap() {}


    /**
     *
     */
    NamedNodeMap(const NamedNodeMap &other)
        {
        entries = other.entries;
        }


    /**
     *
     */
    virtual ~NamedNodeMap() {}

protected:

    std::vector<NamedNodeMapEntry> entries;

};




/*#########################################################################
## CharacterData
#########################################################################*/

/**
 *
 */
class CharacterData : virtual public Node
{
public:

    /**
     *
     */
    virtual DOMString getData() throw(DOMException) = 0;

    /**
     *
     */
    virtual void setData(const DOMString& val) throw(DOMException) = 0;

    /**
     *
     */
    virtual unsigned long getLength() = 0;

    /**
     *
     */
    virtual DOMString substringData(unsigned long offset,
                            unsigned long count)
                            throw(DOMException) = 0;

    /**
     *
     */
    virtual void appendData(const DOMString& arg) throw(DOMException) = 0;

    /**
     *
     */
    virtual void insertData(unsigned long offset,
                    const DOMString& arg)
                    throw(DOMException) = 0;

    /**
     *
     */
    virtual void deleteData(unsigned long offset,
                    unsigned long count)
                    throw(DOMException) = 0;

    /**
     *
     */
    virtual void  replaceData(unsigned long offset,
                      unsigned long count,
                      const DOMString& arg)
                      throw(DOMException) = 0;


    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    virtual ~CharacterData() {}

};





/*#########################################################################
## Attr
#########################################################################*/

/**
 *
 */
class Attr : virtual public Node
{
public:

    /**
     *
     */
    virtual DOMString getName() = 0;

    /**
     *
     */
    virtual bool getSpecified() = 0;

    /**
     *
     */
    virtual DOMString getValue() = 0;

    /**
     *
     */
    virtual void setValue(const DOMString& val) throw(DOMException) = 0;

    /**
     *
     */
    virtual Element *getOwnerElement() = 0;


    /**
     *
     */
    virtual TypeInfo *getSchemaTypeInfo() = 0;


    /**
     *
     */
    virtual bool getIsId() = 0;

    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    virtual ~Attr() {}

};





/*#########################################################################
## Element
#########################################################################*/

/**
 *
 */
class Element : virtual public Node
{
public:


    /**
     *
     */
    virtual DOMString getTagName() = 0;

    /**
     *
     */
    virtual DOMString getAttribute(const DOMString& name) = 0;

    /**
     *
     */
    virtual void setAttribute(const DOMString& name,
                      const DOMString& value)
                      throw(DOMException) = 0;

    /**
     *
     */
    virtual void removeAttribute(const DOMString& name)
                         throw(DOMException) = 0;

    /**
     *
     */
    virtual Attr *getAttributeNode(const DOMString& name) = 0;

    /**
     *
     */
    virtual Attr *setAttributeNode(Attr *newAttr)
                          throw(DOMException) = 0;

    /**
     *
     */
    virtual Attr *removeAttributeNode(Attr *oldAttr)
                             throw(DOMException) = 0;

    /**
     *
     */
    virtual NodeList getElementsByTagName(const DOMString& name) = 0;

    /**
     *
     */
    virtual DOMString getAttributeNS(const DOMString& namespaceURI,
                             const DOMString& localName) = 0;

    /**
     *
     */
    virtual void setAttributeNS(const DOMString& namespaceURI,
                        const DOMString& qualifiedName,
                        const DOMString& value)
                        throw(DOMException) = 0;

    /**
     *
     */
    virtual void removeAttributeNS(const DOMString& namespaceURI,
                           const DOMString& localName)
                           throw(DOMException) = 0;

    /**
     *
     */
    virtual Attr *getAttributeNodeNS(const DOMString& namespaceURI,
                            const DOMString& localName) = 0;

    /**
     *
     */
    virtual Attr *setAttributeNodeNS(Attr *newAttr)
                            throw(DOMException) = 0;

    /**
     *
     */
    virtual NodeList getElementsByTagNameNS(const DOMString& namespaceURI,
                                    const DOMString& localName) = 0;

    /**
     *
     */
    virtual bool hasAttribute(const DOMString& name) = 0;

    /**
     *
     */
    virtual bool hasAttributeNS(const DOMString& namespaceURI,
                        const DOMString& localName) = 0;

    /**
     *
     */
    virtual TypeInfo *getSchemaTypeInto() = 0;


    /**
     *
     */
    virtual void setIdAttribute(const DOMString &name,
                                bool isId) throw (DOMException) = 0;

    /**
     *
     */
    virtual void setIdAttributeNS(const DOMString &namespaceURI,
                                  const DOMString &localName,
                                  bool isId) throw (DOMException) = 0;

    /**
     *
     */
    virtual void setIdAttributeNode(const Attr *idAttr,
                                    bool isId) throw (DOMException) = 0;

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~Element() {}

};





/*#########################################################################
## Text
#########################################################################*/

/**
 *
 */
class Text : virtual public CharacterData
{
public:

    /**
     *
     */
    virtual Text *splitText(unsigned long offset)
                    throw(DOMException) = 0;

    /**
     *
     */
    virtual bool getIsElementContentWhitespace()= 0;

    /**
     *
     */
    virtual DOMString getWholeText() = 0;


    /**
     *
     */
    virtual Text *replaceWholeText(const DOMString &content)
                                 throw(DOMException) = 0;

    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    virtual ~Text() {}

};



/*#########################################################################
## Comment
#########################################################################*/

/**
 *
 */
class Comment : virtual public CharacterData
{
public:

    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    virtual ~Comment() {}


};



/*#########################################################################
## TypeInfo
#########################################################################*/

/**
 *
 */
class TypeInfo
{
public:

    /**
     *
     */
    virtual DOMString getTypeName() =0;

    /**
     *
     */
    virtual DOMString getTypeNamespace() =0;

    typedef enum
        {
        DERIVATION_RESTRICTION = 0x00000001,
        DERIVATION_EXTENSION   = 0x00000002,
        DERIVATION_UNION       = 0x00000004,
        DERIVATION_LIST        = 0x00000008
        } DerivationMethod;


    /**
     *
     */
    virtual bool isDerivedFrom(const DOMString &typeNamespaceArg,
                               const DOMString &typeNameArg,
                               DerivationMethod derivationMethod) =0;

    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    virtual ~TypeInfo() {}

};




/*#########################################################################
## UserDataHandler
#########################################################################*/

/**
 *
 */
class UserDataHandler
{
public:

    typedef enum
        {
        NODE_CLONED     = 1,
        NODE_IMPORTED   = 2,
        NODE_DELETED    = 3,
        NODE_RENAMED    = 4,
        NODE_ADOPTED    = 5
        } OperationType;


    /**
     *
     */
    virtual  void handle(unsigned short operation,
                         const DOMString &key,
                         const DOMUserData *data,
                         const Node *src,
                         const Node *dst) =0;

    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    virtual ~UserDataHandler() {}

};


/*#########################################################################
## DOMError
#########################################################################*/

/**
 *
 */
class DOMError
{
public:

    typedef enum
        {
        DOMERROR_SEVERITY_WARNING     = 1,
        DOMERROR_SEVERITY_ERROR       = 2,
        DOMERROR_SEVERITY_FATAL_ERROR = 3
        } ErrorSeverity;


    /**
     *
     */
    virtual unsigned short getSeverity() =0;

    /**
     *
     */
    virtual DOMString getMessage() =0;

    /**
     *
     */
    virtual DOMString getType() =0;

    /**
     *
     */
    virtual DOMObject *getRelatedException() =0;

    /**
     *
     */
    virtual DOMObject *getRelatedData() =0;

    /**
     *
     */
    virtual DOMLocator *getLocation() =0;


    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    virtual ~DOMError() {}

};


/*#########################################################################
## DOMErrorHandler
#########################################################################*/

/**
 *
 */
class DOMErrorHandler
{
public:
    /**
     *
     */
    virtual bool handleError(const DOMError *error) =0;

    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    virtual ~DOMErrorHandler() {}

};



/*#########################################################################
## DOMLocator
#########################################################################*/

/**
 *
 */
class DOMLocator
{
public:

    /**
     *
     */
    virtual long getLineNumber() =0;

    /**
     *
     */
    virtual long getColumnNumber() =0;

    /**
     *
     */
    virtual long getByteOffset() =0;

    /**
     *
     */
    virtual long getUtf16Offset() =0;


    /**
     *
     */
    virtual Node *getRelatedNode() =0;


    /**
     *
     */
    virtual DOMString getUri() =0;

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~DOMLocator() {}
};


/*#########################################################################
## DOMConfiguration
#########################################################################*/

/**
 *
 */
class DOMConfiguration
{
public:

    /**
     *
     */
    virtual void setParameter(const DOMString &name,
                              const DOMUserData *value) throw (DOMException) =0;

    /**
     *
     */
    virtual DOMUserData *getParameter(const DOMString &name)
                                      throw (DOMException) =0;

    /**
     *
     */
    virtual bool canSetParameter(const DOMString &name,
                                 const DOMUserData *data) =0;

    /**
     *
     */
    virtual DOMStringList *getParameterNames() =0;

    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    virtual ~DOMConfiguration() {}

};






/*#########################################################################
## CDATASection
#########################################################################*/
/**
 *
 */
class CDATASection : virtual public Text
{
public:

    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    virtual ~CDATASection() {}

};




/*#########################################################################
## DocumentType
#########################################################################*/

/**
 *
 */
class DocumentType : virtual public Node
{
public:

    /**
     *
     */
    virtual DOMString getName() = 0;

    /**
     *
     */
    virtual NamedNodeMap getEntities() = 0;

    /**
     *
     */
    virtual NamedNodeMap getNotations() = 0;

    /**
     *
     */
    virtual DOMString getPublicId() = 0;

    /**
     *
     */
    virtual DOMString getSystemId() = 0;

    /**
     *
     */
    virtual DOMString getInternalSubset() = 0;

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~DocumentType() {}

};





/*#########################################################################
## Notation
#########################################################################*/

/**
 *
 */
class Notation : virtual public Node
{
public:

    /**
     *
     */
    virtual DOMString getPublicId() = 0;

    /**
     *
     */
    virtual DOMString getSystemId() = 0;

    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    virtual ~Notation() {}
};






/*#########################################################################
## Entity
#########################################################################*/

/**
 *
 */
class Entity : virtual public Node
{
public:

    /**
     *
     */
    virtual DOMString getPublicId() = 0;

    /**
     *
     */
    virtual DOMString getSystemId() = 0;

    /**
     *
     */
    virtual DOMString getNotationName() = 0;

    /**
     *
     */
    virtual DOMString getInputEncoding() = 0;

    /**
     *
     */
    virtual DOMString getXmlEncoding() = 0;

    /**
     *
     */
    virtual DOMString getXmlVersion() = 0;

    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    virtual ~Entity() {}
};





/*#########################################################################
## EntityReference
#########################################################################*/
/**
 *
 */
class EntityReference : virtual public Node
{
public:


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~EntityReference() {}
};





/*#########################################################################
## ProcessingInstruction
#########################################################################*/

/**
 *
 */
class ProcessingInstruction : virtual public Node
{
public:

    /**
     *
     */
    virtual DOMString getTarget() = 0;

    /**
     *
     */
    virtual DOMString getData() = 0;

    /**
     *
     */
   virtual void setData(const DOMString& val) throw(DOMException) = 0;

    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    virtual ~ProcessingInstruction() {}

};





/*#########################################################################
## DocumentFragment
#########################################################################*/
/**
 *
 */
class DocumentFragment : virtual public Node
{
public:

    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    virtual ~DocumentFragment() {}
};






/*#########################################################################
## Document
#########################################################################*/

/**
 *
 */
class Document : virtual public Node
{
public:

    /**
     *
     */
    virtual DocumentType *getDoctype() = 0;

    /**
     *
     */
    virtual DOMImplementation *getImplementation() = 0;

    /**
     *
     */
    virtual Element *getDocumentElement() = 0;

    /**
     *
     */
    virtual Element *createElement(const DOMString& tagName)
                           throw(DOMException) = 0;

    /**
     *
     */
    virtual DocumentFragment *createDocumentFragment() = 0;

    /**
     *
     */
    virtual Text *createTextNode(const DOMString& data) = 0;

    /**
     *
     */
    virtual Comment  *createComment(const DOMString& data) = 0;

    /**
     *
     */
    virtual CDATASection *createCDATASection(const DOMString& data)
                                     throw(DOMException) = 0;

    /**
     *
     */
    virtual ProcessingInstruction *createProcessingInstruction(const DOMString& target,
                                                       const DOMString& data)
                                                       throw(DOMException) = 0;

    /**
     *
     */
    virtual Attr *createAttribute(const DOMString& name)
                          throw(DOMException) = 0;

    /**
     *
     */
    virtual EntityReference *createEntityReference(const DOMString& name)
                                           throw(DOMException) = 0;

    /**
     *
     */
    virtual NodeList getElementsByTagName(const DOMString& tagname) = 0;


    /**
     *
     */
    virtual Node *importNode(const Node *importedNode,
                     bool deep)
                     throw(DOMException) = 0;

    /**
     *
     */
    virtual Element *createElementNS(const DOMString& namespaceURI,
                             const DOMString& qualifiedName)
                             throw(DOMException) = 0;

    /**
     *
     */
    virtual Attr *createAttributeNS(const DOMString& namespaceURI,
                            const DOMString& qualifiedName)
                            throw(DOMException) = 0;

    /**
     *
     */
    virtual NodeList getElementsByTagNameNS(const DOMString& namespaceURI,
                                     const DOMString& localName) = 0;

    /**
     *
     */
    virtual Element *getElementById(const DOMString& elementId) = 0;


    /**
     *
     */
    virtual DOMString getInputEncoding() = 0;


    /**
     *
     */
    virtual DOMString getXmlEncoding() = 0;

    /**
     *
     */
    virtual bool getXmlStandalone() = 0;

    /**
     *
     */
    virtual void setXmlStandalone(bool val) throw (DOMException) = 0;

    /**
     *
     */
    virtual DOMString getXmlVersion() = 0;

    /**
     *
     */
    virtual void setXmlVersion(const DOMString &version) throw (DOMException) = 0;

    /**
     *
     */
    virtual bool getStrictErrorChecking() = 0;

    /**
     *
     */
    virtual void setStrictErrorChecking(bool val) = 0;


    /**
     *
     */
    virtual DOMString getDocumentURI() = 0;

    /**
     *
     */
    virtual void setDocumentURI(const DOMString &uri) = 0;

    /**
     *
     */
    virtual Node *adoptNode(const Node *source) throw (DOMException) = 0;

    /**
     *
     */
    virtual DOMConfiguration *getDomConfig() = 0;

    /**
     *
     */
    virtual void normalizeDocument() = 0;

    /**
     *
     */
    virtual Node *renameNode(const Node *n,
                             const DOMString &namespaceURI,
                             const DOMString &qualifiedName)
                             throw (DOMException) = 0;


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~Document() {}

};











}  //namespace dom
}  //namespace w3c
}  //namespace org


#endif // __DOM_H__


/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/




