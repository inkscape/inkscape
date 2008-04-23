#ifndef __DOMIMPL_H__
#define __DOMIMPL_H__
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


#include "dom.h"

#include <map>

namespace org
{
namespace w3c
{
namespace dom
{



class DOMImplementationSourceImpl;
class DOMImplementationImpl;
class NodeImpl;
typedef Ptr<NodeImpl> NodeImplPtr;
class CharacterDataImpl;
class AttrImpl;
typedef Ptr<AttrImpl> AttrImplPtr;
class ElementImpl;
typedef Ptr<ElementImpl> ElementImplPtr;
class TextImpl;
class CommentImpl;
class TypeInfoImpl;
class UserDataHandlerImpl;
class DOMErrorImpl;
class DOMErrorHandlerImpl;
class DOMLocatorImpl;
class DOMConfigurationImpl;
class CDATASectionImpl;
class DocumentTypeImpl;
typedef Ptr<DocumentTypeImpl> DocumentTypeImplPtr;
class NotationImpl;
class EntityImpl;
class EntityReferenceImpl;
class ProcessingInstructionImpl;
class DocumentFragmentImpl;
class DocumentImpl;
typedef Ptr<DocumentImpl> DocumentImplPtr;



/*#########################################################################
## DOMImplementationSourceImpl
#########################################################################*/

class DOMImplementationSourceImpl : public DOMImplementationSource
{
public:

    /**
     *
     */
    virtual DOMImplementation *getDOMImplementation(const DOMString &features);

    /**
     *
     */
    virtual DOMImplementationList getDOMImplementationList(const DOMString &features);


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    DOMImplementationSourceImpl();

    /**
     *
     */
    virtual ~DOMImplementationSourceImpl();

protected:


    DOMImplementationImpl *domImpl;
    DOMImplementationList domImplList;
};





/*#########################################################################
## DOMImplementationImpl
#########################################################################*/
/**
 *
 */
class DOMImplementationImpl : public DOMImplementation
{
public:


    /**
     *
     */
    DOMImplementationImpl();

    /**
     *
     */
    virtual ~DOMImplementationImpl();

    /**
     *
     */
    virtual bool hasFeature(const DOMString& feature, const DOMString& version);


    /**
     *
     */
    virtual DocumentTypePtr createDocumentType(const DOMString& qualifiedName,
                                     const DOMString& publicId,
                                     const DOMString& systemId)
                                     throw(DOMException);

    /**
     *
     */
    virtual DocumentPtr createDocument(const DOMString& namespaceURI,
                             const DOMString& qualifiedName,
                             DocumentTypePtr doctype)
                             throw(DOMException);
    /**
     *
     */
    virtual DOMObject *getFeature(const DOMString& feature,
                             const DOMString& version);


protected:

};




/*#########################################################################
## NodeImpl
#########################################################################*/

/**
 *
 */
class NodeImpl : virtual public Node
{

    friend class DocumentImpl;

public:

    /**
     *
     */
    virtual DOMString getNodeName();

    /**
     *
     */
    virtual DOMString getNodeValue() throw (DOMException);

    /**
     *
     */
    virtual void setNodeValue(const DOMString& val) throw (DOMException);

    /**
     *
     */
    virtual unsigned short getNodeType();

    /**
     *
     */
    virtual NodePtr getParentNode();

    /**
     *
     */
    virtual NodeList getChildNodes();

    /**
     *
     */
    virtual NodePtr getFirstChild();

    /**
     *
     */
    virtual NodePtr getLastChild();

    /**
     *
     */
    virtual NodePtr getPreviousSibling();

    /**
     *
     */
    virtual NodePtr getNextSibling();

    /**
     *
     */
    virtual NamedNodeMap &getAttributes();


    /**
     *
     */
    virtual DocumentPtr getOwnerDocument();

    /**
     *
     */
    virtual NodePtr insertBefore(const NodePtr newChild,
                       const NodePtr refChild)
                       throw(DOMException);

    /**
     *
     */
    virtual NodePtr replaceChild(const NodePtr newChild,
                       const NodePtr oldChild)
                       throw(DOMException);

    /**
     *
     */
    virtual NodePtr removeChild(const NodePtr oldChild)
                      throw(DOMException);

    /**
     *
     */
    virtual NodePtr appendChild(const NodePtr newChild)
                      throw(DOMException);

    /**
     *
     */
    virtual bool hasChildNodes();

    /**
     *
     */
    virtual NodePtr cloneNode(bool deep);

    /**
     *
     */
    virtual void normalize();

    /**
     *
     */
    virtual bool isSupported(const DOMString& feature,
                     const DOMString& version);

    /**
     *
     */
    virtual DOMString getNamespaceURI();

    /**
     *
     */
    virtual DOMString getPrefix();

    /**
     *
     */
    virtual void setPrefix(const DOMString& val) throw(DOMException);

    /**
     *
     */
    virtual DOMString getLocalName();

    /**
     *
     */
    virtual bool hasAttributes();

    /**
     *
     */
    virtual DOMString getBaseURI();

    /**
     *
     */
    virtual unsigned short compareDocumentPosition(const NodePtr other);

    /**
     *
     */
    virtual DOMString getTextContent() throw(DOMException);


    /**
     *
     */
    virtual void setTextContent(const DOMString &val) throw(DOMException);


    /**
     *
     */
    virtual DOMString lookupPrefix(const DOMString &namespaceURI);


    /**
     *
     */
    virtual bool isDefaultNamespace(const DOMString &namespaceURI);


    /**
     *
     */
    virtual DOMString lookupNamespaceURI(const DOMString &prefix);


    /**
     *
     */
    virtual bool isEqualNode(const NodePtr node);



    /**
     *
     */
    virtual DOMObject *getFeature(const DOMString &feature,
                                  const DOMString &version);

    /**
     *
     */
    virtual DOMUserData *setUserData(const DOMString &key,
                                     const DOMUserData *data,
                                     const UserDataHandler *handler);


    /**
     *
     */
    virtual DOMUserData *getUserData(const DOMString &key);


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual void bindingsAdd(const DOMString &prefix, const DOMString &namespaceURI)
        {
        bindings[prefix] = namespaceURI;
        }

    /**
     *
     */
    virtual void bindingsClear()
        {
        bindings.clear();
        }

    DOMString bindingsFind(const DOMString &prefix)
        {
        std::map<DOMString, DOMString>::iterator iter =
             bindings.find(prefix);
        if (iter != bindings.end())
            {
            DOMString ret = iter->second;
            return ret;
            }
        if (parent.get())
            {
            DOMString ret = parent->bindingsFind(prefix);
            if (ret.size() > 0)
                return ret;
            }
        return "";
        }

    /**
     *
     */
    virtual void setNodeName(const DOMString &qualifiedName);

    /**
     *
     */
    virtual void setNamespaceURI(const DOMString &theNamespaceURI);

    /**
     *
     */
    DOMString lookupNamespacePrefix(const DOMString &namespaceURI,
                                    NodePtr originalElement);
    /**
     *
     */
    NodeImpl();

    /**
     *
     */
    NodeImpl(const NodeImpl &other);

    /**
     *
     */
    NodeImpl &operator=(const NodeImpl &other);

    /**
     *
     */
    NodeImpl(DocumentImplPtr owner);

    /**
     *
     */
    NodeImpl(DocumentImplPtr owner, const DOMString &nodeName);

    /**
     *
     */
    NodeImpl(DocumentImplPtr owner, const DOMString &namespaceURI,
	               const DOMString &nodeName);

    /**
     *
     */
    virtual ~NodeImpl();


    /**
     *
     */
    void assign(const NodeImpl &other);

protected:

    /**
     * Set up the internal values
     */
    void init();

    unsigned short nodeType;

    NodeImplPtr parent;

    NodeImplPtr prev;

    NodeImplPtr next;

    DOMUserData *userData;

    DOMString prefix;

    DOMString localName;

    DOMString nodeName;

    DOMString namespaceURI;

    DOMString baseURI;

    DOMString nodeValue;

    NodeImplPtr firstChild;
    NodeImplPtr lastChild;

    DocumentImplPtr ownerDocument;

    NamedNodeMap attributes;

    class UserDataEntry
    {
    public:
        UserDataEntry(const DOMString       &theKey,
                      const DOMUserData     *theData,
                      const UserDataHandler *theHandler)
        {
            next    = NULL;
            key     = theKey;
            data    = (DOMUserData *)theData;
            handler = (UserDataHandler *)theHandler;
        }

        virtual ~UserDataEntry()
        {
            //delete anything after me, too
            if (next)
                delete next;
        }

        UserDataEntry   *next;
        DOMString       key;
        DOMUserData     *data;
        UserDataHandler *handler;
    };

    UserDataEntry *userDataEntries;
    
    TypeInfo typeInfo;

    //### Our prefix->namespaceURI bindings

    std::map<DOMString, DOMString> bindings;


};



/*#########################################################################
## CharacterDataImpl
#########################################################################*/

/**
 *
 */
class CharacterDataImpl : virtual public CharacterData, protected NodeImpl
{
public:

    /**
     *
     */
    virtual DOMString getData() throw(DOMException);

    /**
     *
     */
    virtual void setData(const DOMString& val) throw(DOMException);

    /**
     *
     */
    virtual unsigned long getLength();

    /**
     *
     */
    virtual DOMString substringData(unsigned long offset,
                            unsigned long count)
                            throw(DOMException);

    /**
     *
     */
    virtual void appendData(const DOMString& arg) throw(DOMException);

    /**
     *
     */
    virtual void insertData(unsigned long offset,
                    const DOMString& arg)
                    throw(DOMException);

    /**
     *
     */
    virtual void deleteData(unsigned long offset,
                    unsigned long count)
                    throw(DOMException);

    /**
     *
     */
    virtual void  replaceData(unsigned long offset,
                      unsigned long count,
                      const DOMString& arg)
                      throw(DOMException);


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    CharacterDataImpl();


    /**
     *
     */
    CharacterDataImpl(DocumentImplPtr owner, const DOMString &value);

    /**
     *
     */
    virtual ~CharacterDataImpl();

protected:

   //'data' is the nodeValue

};





/*#########################################################################
## AttrImpl
#########################################################################*/

/**
 *
 */
class AttrImpl : virtual public Attr, public NodeImpl
{
public:

    /**
     *
     */
    virtual DOMString getName();

    /**
     *
     */
    virtual bool getSpecified();

    /**
     *
     */
    virtual DOMString getValue();

    /**
     *
     */
    virtual void setValue(const DOMString& val) throw(DOMException);

    /**
     *
     */
    virtual ElementPtr getOwnerElement();


    /**
     *
     */
    virtual TypeInfo &getSchemaTypeInfo();


    /**
     *
     */
    virtual bool getIsId();



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual void setOwnerElement(const ElementPtr elem);

    /**
     *
     */
    AttrImpl(DocumentImplPtr owner, const DOMString &name);

    /**
     *
     */
    AttrImpl(DocumentImplPtr owner, const DOMString &namespaceURI,
	                       const DOMString &name);

    /**
     *
     */
    virtual ~AttrImpl();

protected:


    ElementPtr ownerElement;


};





/*#########################################################################
## ElementImpl
#########################################################################*/

/**
 *
 */
class ElementImpl : virtual public Element, public NodeImpl
{
public:

    /**
     *
     */
    virtual DOMString getTagName();

    /**
     *
     */
    virtual DOMString getAttribute(const DOMString& name);

    /**
     *
     */
    virtual void setAttribute(const DOMString& name,
                      const DOMString& value)
                      throw(DOMException);

    /**
     *
     */
    virtual void removeAttribute(const DOMString& name)
                         throw(DOMException);

    /**
     *
     */
    virtual AttrPtr getAttributeNode(const DOMString& name);

    /**
     *
     */
    virtual AttrPtr setAttributeNode(AttrPtr newAttr)
                          throw(DOMException);

    /**
     *
     */
    virtual AttrPtr removeAttributeNode(AttrPtr oldAttr)
                             throw(DOMException);

    /**
     *
     */
    virtual NodeList getElementsByTagName(const DOMString& name);

    /**
     *
     */
    virtual DOMString getAttributeNS(const DOMString& namespaceURI,
                             const DOMString& localName);

    /**
     *
     */
    virtual void setAttributeNS(const DOMString& namespaceURI,
                        const DOMString& qualifiedName,
                        const DOMString& value)
                        throw(DOMException);

    /**
     *
     */
    virtual void removeAttributeNS(const DOMString& namespaceURI,
                           const DOMString& localName)
                           throw(DOMException);

    /**
     *
     */
    virtual AttrPtr getAttributeNodeNS(const DOMString& namespaceURI,
                            const DOMString& localName);

    /**
     *
     */
    virtual AttrPtr setAttributeNodeNS(AttrPtr newAttr)
                            throw(DOMException);

    /**
     *
     */
    virtual NodeList getElementsByTagNameNS(const DOMString& namespaceURI,
                                    const DOMString& localName);

    /**
     *
     */
    virtual bool hasAttribute(const DOMString& name);

    /**
     *
     */
    virtual bool hasAttributeNS(const DOMString& namespaceURI,
                        const DOMString& localName);

    /**
     *
     */
    virtual TypeInfo &getSchemaTypeInfo();


    /**
     *
     */
    virtual void setIdAttribute(const DOMString &name,
                                bool isId) throw (DOMException);

    /**
     *
     */
    virtual void setIdAttributeNS(const DOMString &namespaceURI,
                                  const DOMString &localName,
                                  bool isId) throw (DOMException);

    /**
     *
     */
    virtual void setIdAttributeNode(const AttrPtr idAttr,
                                    bool isId) throw (DOMException);



    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    ElementImpl();

    /**
     *
     */
    ElementImpl(DocumentImplPtr owner, const DOMString &tagName);

    /**
     *
     */
    ElementImpl(DocumentImplPtr owner, const DOMString &namespaceURI,
	               const DOMString &tagName);

    /**
     *
     */
    virtual ~ElementImpl();

    /**
     *
     */
    void normalizeNamespaces();

protected:

friend class DocumentImpl;

    static void getElementsByTagNameRecursive(NodeList &list,
                        const DOMString& name, ElementPtr elem);
    static void getElementsByTagNameNSRecursive(NodeList &list,
             const DOMString& namespaceURI, const DOMString& tagName,
			  ElementPtr elem);
};





/*#########################################################################
## TextImpl
#########################################################################*/

/**
 *
 */
class TextImpl : virtual public Text, protected CharacterDataImpl
{
public:

    /**
     *
     */
    virtual TextPtr splitText(unsigned long offset)
                    throw(DOMException);

    /**
     *
     */
    virtual bool getIsElementContentWhitespace();

    /**
     *
     */
    virtual DOMString getWholeText();


    /**
     *
     */
    virtual TextPtr replaceWholeText(const DOMString &content)
                                 throw(DOMException);

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    TextImpl();


    /**
     *
     */
    TextImpl(DocumentImplPtr owner, const DOMString &val);

    /**
     *
     */
    virtual ~TextImpl();

protected:

};



/*#########################################################################
## CommentImpl
#########################################################################*/

/**
 *
 */
class CommentImpl : virtual public Comment, protected CharacterDataImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    CommentImpl();

    /**
     *
     */
    CommentImpl(DocumentImplPtr owner, const DOMString &theValue);

    /**
     *
     */
    virtual ~CommentImpl();
};



/*#########################################################################
## TypeInfoImpl
#########################################################################*/

/**
 *
 */
class TypeInfoImpl : public TypeInfo
{
public:

    /**
     *
     */
    virtual DOMString getTypeName();

    /**
     *
     */
    virtual DOMString getTypeNamespace();

    /**
     *
     */
    virtual bool isDerivedFrom(const DOMString &typeNamespaceArg,
                               const DOMString &typeNameArg,
                               const DerivationMethod derivationMethod);


    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    TypeInfoImpl(const DOMString &typeNamespaceArg,
                 const DOMString &typeNameArg,
                 const DerivationMethod derivationMethod);

    /**
     *
     */
    virtual ~TypeInfoImpl();

protected:

    DOMString typeName;

    DOMString typeNamespace;

    unsigned short derivationMethod;

};




/*#########################################################################
## UserDataHandlerImpl
#########################################################################*/

/**
 *
 */
class UserDataHandlerImpl : public UserDataHandler
{
public:

    /**
     *
     */
    virtual  void handle(unsigned short operation,
                         const DOMString &key,
                         const DOMUserData *data,
                         const NodePtr src,
                         const NodePtr dst);

    //##################
    //# Non-API methods
    //##################


protected:

    /**
     *
     */
    UserDataHandlerImpl();

    /**
     *
     */
    virtual ~UserDataHandlerImpl();
};


/*#########################################################################
## DOMErrorImpl
#########################################################################*/

/**
 *
 */
class DOMErrorImpl : public DOMError
{
public:

    /**
     *
     */
    virtual unsigned short getSeverity();

    /**
     *
     */
    virtual DOMString getMessage();

    /**
     *
     */
    virtual DOMString getType();

    /**
     *
     */
    virtual DOMObject *getRelatedException();

    /**
     *
     */
    virtual DOMObject *getRelatedData();

    /**
     *
     */
    virtual DOMLocator *getLocation();


    //##################
    //# Non-API methods
    //##################


protected:

    /**
     *
     */
    DOMErrorImpl();

    /**
     *
     */
    virtual ~DOMErrorImpl();

    unsigned short severity;

    DOMString message;

    DOMString type;


};


/*#########################################################################
## DOMErrorHandlerImpl
#########################################################################*/

/**
 *
 */
class DOMErrorHandlerImpl : public DOMErrorHandler
{
public:

    /**
     *
     */
    virtual bool handleError(const DOMError *error);



    //##################
    //# Non-API methods
    //##################



protected:

    /**
     *
     */
    DOMErrorHandlerImpl();

    /**
     *
     */
    virtual ~DOMErrorHandlerImpl();


};



/*#########################################################################
## DOMLocatorImpl
#########################################################################*/

/**
 *
 */
class DOMLocatorImpl : public DOMLocator
{
public:

    /**
     *
     */
    virtual long getLineNumber();

    /**
     *
     */
    virtual long getColumnNumber();

    /**
     *
     */
    virtual long getByteOffset();

    /**
     *
     */
    virtual long getUtf16Offset();


    /**
     *
     */
    virtual NodePtr getRelatedNode();


    /**
     *
     */
    virtual DOMString getUri();



    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    DOMLocatorImpl();

    /**
     *
     */
    virtual ~DOMLocatorImpl();

protected:


    long lineNumber;

    long columnNumber;

    long byteOffset;

    long utf16Offset;

    NodePtr relatedNode;

    DOMString uri;
};


/*#########################################################################
## DOMConfigurationImpl
#########################################################################*/

/**
 *
 */
class DOMConfigurationImpl : public DOMConfiguration
{
public:

    /**
     *
     */
    virtual void setParameter(const DOMString &name,
                              const DOMUserData *value) throw (DOMException);

    /**
     *
     */
    virtual DOMUserData *getParameter(const DOMString &name)
                                      throw (DOMException);

    /**
     *
     */
    virtual bool canSetParameter(const DOMString &name,
                                 const DOMUserData *data);

    /**
     *
     */
    virtual DOMStringList *getParameterNames();


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    DOMConfigurationImpl();

    /**
     *
     */
    virtual ~DOMConfigurationImpl();

protected:

};






/*#########################################################################
## CDATASectionImpl
#########################################################################*/
/**
 *
 */
class CDATASectionImpl : public CDATASection, public TextImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    CDATASectionImpl();


    /**
     *
     */
    CDATASectionImpl(DocumentImplPtr owner, const DOMString &value);

    /**
     *
     */
    virtual ~CDATASectionImpl();

};




/*#########################################################################
## DocumentTypeImpl
#########################################################################*/

/**
 *
 */
class DocumentTypeImpl : public DocumentType, public NodeImpl
{
public:

    /**
     *
     */
    virtual DOMString getName();

    /**
     *
     */
    virtual NamedNodeMap getEntities();

    /**
     *
     */
    virtual NamedNodeMap getNotations();

    /**
     *
     */
    virtual DOMString getPublicId();

    /**
     *
     */
    virtual DOMString getSystemId();

    /**
     *
     */
    virtual DOMString getInternalSubset();

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
   DocumentTypeImpl();

    /**
     *
     */
   DocumentTypeImpl(const DOMString& name,
                    const DOMString& publicId,
                    const DOMString& systemId);
    /**
     *
     */
   virtual ~DocumentTypeImpl();


protected:
   DOMString name;
   DOMString publicId;
   DOMString systemId;

   NamedNodeMap entities;
   NamedNodeMap notations;

};





/*#########################################################################
## NotationImpl
#########################################################################*/

/**
 *
 */
class NotationImpl : public Notation, public NodeImpl
{
public:

    /**
     *
     */
    virtual DOMString getPublicId();

    /**
     *
     */
    virtual DOMString getSystemId();


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    NotationImpl();

    /**
     *
     */
    NotationImpl(DocumentImplPtr owner);

    /**
     *
     */
    virtual ~NotationImpl();


protected:



    DOMString publicId;

    DOMString systemId;
};






/*#########################################################################
## EntityImpl
#########################################################################*/

/**
 *
 */
class EntityImpl : public Entity, public NodeImpl
{
public:

    /**
     *
     */
    virtual DOMString getPublicId();

    /**
     *
     */
    virtual DOMString getSystemId();

    /**
     *
     */
    virtual DOMString getNotationName();

    /**
     *
     */
    virtual DOMString getInputEncoding();

    /**
     *
     */
    virtual DOMString getXmlEncoding();

    /**
     *
     */
    virtual DOMString getXmlVersion();


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    EntityImpl();


    /**
     *
     */
    EntityImpl(DocumentImplPtr owner);

    /**
     *
     */
    virtual ~EntityImpl();

protected:



    DOMString publicId;

    DOMString systemId;

    DOMString notationName;

    DOMString inputEncoding;

    DOMString xmlEncoding;

    DOMString xmlVersion;

};





/*#########################################################################
## EntityReferenceImpl
#########################################################################*/
/**
 *
 */
class EntityReferenceImpl : public EntityReference, public NodeImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    EntityReferenceImpl();


    /**
     *
     */
    EntityReferenceImpl(DocumentImplPtr owner, const DOMString &theName);

    /**
     *
     */
    virtual ~EntityReferenceImpl();

};





/*#########################################################################
## ProcessingInstructionImpl
#########################################################################*/

/**
 *
 */
class ProcessingInstructionImpl : 
              public ProcessingInstruction,
			  public NodeImpl
{
public:

    /**
     *
     */
    virtual DOMString getTarget();

    /**
     *
     */
    virtual DOMString getData();

    /**
     *
     */
   virtual void setData(const DOMString& val) throw(DOMException);


    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    ProcessingInstructionImpl();


    /**
     *
     */
    ProcessingInstructionImpl(DocumentImplPtr owner,
                              const DOMString &target,
                              const DOMString &data);

    /**
     *
     */
    virtual ~ProcessingInstructionImpl();


protected:


    //'target' is nodeName

    //'data' is nodeValue


};





/*#########################################################################
## DocumentFragmentImpl
#########################################################################*/
/**
 *
 */
class DocumentFragmentImpl : public DocumentFragment, public NodeImpl
{

public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    DocumentFragmentImpl();

    /**
     *
     */
    DocumentFragmentImpl(DocumentImplPtr owner);

    /**
     *
     */
    virtual ~DocumentFragmentImpl();

};






/*#########################################################################
## DocumentImpl
#########################################################################*/

/**
 *
 */
class DocumentImpl : virtual public Document, public NodeImpl
{
public:

    /**
     *
     */
    virtual DocumentTypePtr getDoctype();

    /**
     *
     */
    virtual DOMImplementation *getImplementation();

    /**
     *
     */
    virtual ElementPtr getDocumentElement();

    /**
     *
     */
    virtual ElementPtr createElement(const DOMString& tagName)
                           throw(DOMException);

    /**
     *
     */
    virtual DocumentFragmentPtr createDocumentFragment();

    /**
     *
     */
    virtual TextPtr createTextNode(const DOMString& data);

    /**
     *
     */
    virtual CommentPtr createComment(const DOMString& data);

    /**
     *
     */
    virtual CDATASectionPtr createCDATASection(const DOMString& data)
                                     throw(DOMException);

    /**
     *
     */
    virtual ProcessingInstructionPtr createProcessingInstruction(
	                                const DOMString& target,
                                    const DOMString& data)
                                    throw(DOMException);

    /**
     *
     */
    virtual AttrPtr createAttribute(const DOMString& name)
                          throw(DOMException);

    /**
     *
     */
    virtual EntityReferencePtr createEntityReference(const DOMString& name)
                                           throw(DOMException);

    /**
     *
     */
    virtual NodeList getElementsByTagName(const DOMString& tagname);


    /**
     *
     */
    virtual NodePtr importNode(const NodePtr importedNode,
                     bool deep)
                     throw(DOMException);

    /**
     *
     */
    virtual ElementPtr createElementNS(const DOMString& namespaceURI,
                             const DOMString& qualifiedName)
                             throw(DOMException);

    /**
     *
     */
    virtual AttrPtr createAttributeNS(const DOMString& namespaceURI,
                            const DOMString& qualifiedName)
                            throw(DOMException);

    /**
     *
     */
    virtual NodeList getElementsByTagNameNS(const DOMString& namespaceURI,
                                     const DOMString& localName);

    /**
     *
     */
    virtual ElementPtr getElementById(const DOMString& elementId);


    /**
     *
     */
    virtual DOMString getInputEncoding();


    /**
     *
     */
    virtual DOMString getXmlEncoding();

    /**
     *
     */
    virtual bool getXmlStandalone();

    /**
     *
     */
    virtual void setXmlStandalone(bool val) throw (DOMException);

    /**
     *
     */
    virtual DOMString getXmlVersion();

    /**
     *
     */
    virtual void setXmlVersion(const DOMString &version) throw (DOMException);

    /**
     *
     */
    virtual bool getStrictErrorChecking();

    /**
     *
     */
    virtual void setStrictErrorChecking(bool val);


    /**
     *
     */
    virtual DOMString getDocumentURI();

    /**
     *
     */
    virtual void setDocumentURI(const DOMString &uri);

    /**
     *
     */
    virtual NodePtr adoptNode(const NodePtr source) throw (DOMException);

    /**
     *
     */
    virtual DOMConfiguration *getDomConfig();

    /**
     *
     */
    virtual void normalizeDocument();

    /**
     *
     */
    virtual NodePtr renameNode(const NodePtr n,
                             const DOMString &name,
                             const DOMString &qualifiedName)
                             throw (DOMException);


    //##################
    //# Non-API methods
    //##################

    DocumentImpl(const DOMImplementation *domImpl,
                 const DOMString       &namespaceURI,
                 const DOMString       &qualifiedName,
                 const DocumentTypePtr doctype);

    virtual ~DocumentImpl();


    DOMString *stringCache(const DOMString &val);

    int namespaceIndex;

protected:

    DOMImplementation *parent;

    DOMString documentURI;

    DOMString qualifiedName;

    DocumentTypePtr doctype;

    ElementImplPtr documentElement;

    class NamedElementItem
    {
    public:
        NamedElementItem()
        {
            next = NULL;
        }
        NamedElementItem(const DOMString &nameArg, ElementPtr elemArg)
        {
            next = NULL;
            name = nameArg;
            elem = elemArg;
        }
        ~NamedElementItem()
        {
            if (next)
                delete next;
        }
        NamedElementItem *next;
        DOMString        name;
        ElementPtr       elem;
    };

    NamedElementItem elementsById;


    DOMString xmlEncoding;

    DOMString inputEncoding;

    DOMString xmlVersion;

    bool xmlStandalone;

    bool strictErrorChecking;

    DOMConfiguration *domConfig;

    NamedNodeMap namespaceURIs;


};











}  //namespace dom
}  //namespace w3c
}  //namespace org


#endif // __DOMIMPL_H__


/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/




