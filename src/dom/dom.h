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
 * More thorough explanations of the various classes and their algorithms
 * can be found there.
 *     
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2006-2008 Bob Jamison
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

#include <vector>

/**
 * What type of string do we want?  Pick one of the following
 * Then below, select one of the corresponding typedefs? 
 */ 

#include <glibmm.h>
//#include <string>

//# Unfortunate hack for a name collision
#ifdef SEVERITY_ERROR
#undef SEVERITY_ERROR
#endif

#define XMLNSNAME "http://www.w3.org/2000/xmlns/"

namespace org
{
namespace w3c
{
namespace dom
{


/**
 * This is the org::w3c::dom::DOMString definition.
 * Which type do we want?
 */  
typedef Glib::ustring DOMString;
typedef gunichar XMLCh;

//typedef std::string DOMString;
//typedef unsigned short XMLCh;


/**
 *  At least 64 bit time stamp value.
 */
typedef unsigned long long DOMTimeStamp;

/**
 *  This is used for storing refs to user-supplied data.
 */
typedef void DOMUserData;


/*#########################################################################
## NodePtr
#########################################################################*/

/**
 * A simple Smart Pointer class that handles Nodes and all of its
 * descendants.  This is very similar to shared_ptr, but it is customized
 * to handle our needs. 
 */ 
template<class T> class Ptr
{
public:

    /**
     * Simple constructor
     */ 
    Ptr()
        { _ref = 0; }

    /**
     * Constructor upon a reference
     */ 
    template<class Y> Ptr(const Ptr<Y> &other)
        {
        _ref = other._ref;
	    incrementRefCount(_ref);
        }

    /**
     * Constructor upon a reference
     */ 
    Ptr(T * refArg, bool addRef = true)
        {
        _ref = refArg;
        if(addRef)
		    incrementRefCount(_ref);
        }


    /**
     * Copy constructor
     */ 
    Ptr(const Ptr &other)
        {
        _ref = other._ref;
	    incrementRefCount(_ref);
        }

    /**
     * Destructor
     */ 
    virtual ~Ptr()
    {
        decrementRefCount(_ref);
    }


    /**
     * Assignment operator
     */ 
    template<class Y> Ptr &operator=(const Ptr<Y> &other)
        {
        decrementRefCount(_ref);
        _ref = other._ref;
        incrementRefCount(_ref);
        return *this;
        }

    /**
     * Assignment operator
     */ 
    Ptr &operator=(const Ptr &other)
        {
        decrementRefCount(_ref);
        _ref = other._ref;
        incrementRefCount(_ref);
        return *this;
        }

    /**
     * Assignment operator
     */ 
    template<class Y> Ptr &operator=(Y * ref)
        {
        decrementRefCount(_ref);
        _ref = ref;
        incrementRefCount(_ref);
        return *this;
        }

    /**
     * Assignment operator
     */ 
    template<class Y> Ptr &operator=(const Y * ref)
        {
        decrementRefCount(_ref);
        _ref = (Y *) ref;
        incrementRefCount(_ref);
        return *this;
        }

    /**
     * Return the reference
     */ 
    T * get() const
        {
        return _ref;
        }

    /**
     * Dereference operator
     */ 
    T &operator*() const
        {
        return *_ref;
        }

    /**
     * Point-to operator
     */ 
    T *operator->() const
        {
        return _ref;
        }

    /**
     * NOT bool operator.  How to check if we are null without a comparison
     */	     
    bool operator! () const
        {
        return (_ref == 0);
        }

    /**
     * Swap what I reference with the other guy
     */	     
    void swap(Ptr &other)
        {
        T *tmp = _ref;
        _ref = other._ref;
        other._ref = tmp;
        }

    //The referenced item
    T *_ref;
};


/**
 * Global definitions.  Many of these are used to mimic behaviour of
 * a real pointer 
 */

/**
 * Equality
 */ 
template<class T, class U> inline bool
   operator==(const Ptr<T> &a, const Ptr<U> &b)
{
    return a.get() == b.get();
}

/**
 * Inequality
 */ 
template<class T, class U> inline bool
     operator!=(const Ptr<T> &a, const Ptr<U> &b)
{
    return a.get() != b.get();
}

/**
 * Equality
 */ 
template<class T> inline bool
     operator==(const Ptr<T> &a, T * b)
{
    return a.get() == b;
}

/**
 * Inequality
 */ 
template<class T> inline bool
     operator!=(const Ptr<T> &a, T * b)
{
    return a.get() != b;
}

/**
 * Equality
 */ 
template<class T> inline bool
     operator==(T * a, const Ptr<T> &b)
{
    return a == b.get();
}

/**
 * Inequality
 */ 
template<class T> inline bool
     operator!=(T * a, const Ptr<T> &b)
{
    return a != b.get();
}


/**
 * Less than
 */ 
template<class T> inline bool
     operator<(const Ptr<T> &a, const Ptr<T> &b)
{
    return std::less<T *>()(a.get(), b.get());
}

/**
 * Swap
 */ 
template<class T> void
     swap(Ptr<T> &a, Ptr<T> &b)
{
    a.swap(b);
}


/**
 * Get the pointer globally, for <algo>
 */ 
template<class T> T * 
    get_pointer(const Ptr<T> &p)
{
    return p.get();
}

/**
 * Static cast
 */ 
template<class T, class U> Ptr<T>
     static_pointer_cast(const Ptr<U> &p)
{
    return static_cast<T *>(p.get());
}

/**
 * Const cast
 */ 
template<class T, class U> Ptr<T>
     const_pointer_cast(const Ptr<U> &p)
{
    return const_cast<T *>(p.get());
}

/**
 * Dynamic cast
 */ 
template<class T, class U> Ptr<T>
     dynamic_pointer_cast(const Ptr<U> &p)
{
    return dynamic_cast<T *>(p.get());
}


/**
 *  This is used for opaque references to arbitrary objects from
 *  the DOM tree. 
 */
typedef void DOMObject;


/**
 * Forward references.  These are needed because of extensive
 * inter-referencing within the DOM tree.
 */  
class NodeList;
class NamedNodeMap;
class DOMException;
class DOMStringList;
class NameList;
class DOMImplementationList;
class DOMImplementationSource;
class DOMImplementation;
class TypeInfo;
class UserDataHandler;
class DOMError;
class DOMErrorHandler;
class DOMLocator;
class DOMConfiguration;

/**
 * Smart pointer definitions.  Most methods that return references to
 * Nodes of various types, will return one of these smart pointers instead,
 * to allow refcounting and GC.
 */   
class Node;
typedef Ptr<Node> NodePtr;
class CharacterData;
typedef Ptr<CharacterData> CharacterDataPtr;
class Attr;
typedef Ptr<Attr> AttrPtr;
class Element;
typedef Ptr<Element> ElementPtr;
class Text;
typedef Ptr<Text> TextPtr;
class Comment;
typedef Ptr<Comment> CommentPtr;
class DocumentType;
typedef Ptr<DocumentType> DocumentTypePtr;
class CDATASection;
typedef Ptr<CDATASection> CDATASectionPtr;
class Notation;
typedef Ptr<Notation> NotationPtr;
class Entity;
typedef Ptr<Entity> EntityPtr;
class EntityReference;
typedef Ptr<EntityReference> EntityReferencePtr;
class ProcessingInstruction;
typedef Ptr<ProcessingInstruction> ProcessingInstructionPtr;
class DocumentFragment;
typedef Ptr<DocumentFragment> DocumentFragmentPtr;
class Document;
typedef Ptr<Document> DocumentPtr;




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
 *  An Exception class.  Not an interface, since this is something that
 *  all implementations must support. 
 */
class DOMException
{

public:

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



    DOMException(const DOMString &reasonMsg)
        { msg = reasonMsg; }

    DOMException(short theCode)
        {
        code = theCode;
        }

    virtual ~DOMException() throw()
       {}

    /**
     *  What type of exception?  One of the ExceptionCodes above.
     */
    unsigned short code;

    /**
     * Some text describing the context that generated this exception.
     */
    DOMString msg;

    /**
     * Get a string, translated from the code.
     * Like std::exception. Not in spec.
     */
    const char *what()
        { return (const char *)msg.c_str(); }



};






/*#########################################################################
## DOMStringList
#########################################################################*/

/**
 *  This holds a list of DOMStrings.  This is likely the response to a query,
 *  or the value of an attribute. 
 */ 
class DOMStringList
{
public:

    /**
     *  Get the nth string of the list
     */
    virtual DOMString item(unsigned long index)
        {
        if (index>=strings.size())
            return "";
        return strings[index];
        }

    /**
     * How many strings in this list?
     */
    virtual unsigned long getLength()
        {
        return (unsigned long) strings.size();
        }

    /**
     *  Is the argument string present in this list?  Lexically, not identically.
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
    DOMStringList &operator=(const DOMStringList &other)
        {
        strings = other.strings;
        return *this;
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


/**
 * Constains a list of namespaced names.
 */ 
class NameList
{
private:

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
	    NamePair &operator=(const NamePair &other)
	        {
	        namespaceURI = other.namespaceURI;
	        name         = other.name;
	        return *this;
	        }
	    virtual ~NamePair() {}
	
	    DOMString namespaceURI;
	    DOMString name;
	};

public:

    /**
     * Returns a name at the given index.  If out of range, return -1.
     */
    virtual DOMString getName(unsigned long index)
        {
        if (index>=namePairs.size())
            return "";
        return namePairs[index].name;
        }

    /**
     * Returns a namespace at the given index.  If out of range, return -1.
     */
    virtual DOMString getNamespaceURI(unsigned long index)
        {
        if (index>=namePairs.size())
            return "";
        return namePairs[index].namespaceURI;
        }

    /**
     * Return the number of entries in this list.
     */
    virtual unsigned long getLength()
        {
        return (unsigned long)namePairs.size();
        }

    /**
     * Return whether the name argument is present in the list.
     * This is done lexically, not identically.     
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
     * Return whether the namespaced name argument is present in the list.
     * This is done lexically, not identically.     
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
    NameList &operator=(const NameList &other)
        {
        namePairs = other.namePairs;
        return *this;
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

/**
 * Contains a list of DOMImplementations, with accessors.
 */ 
class DOMImplementationList
{
public:

    /**
     * Return a DOMImplementation at the given index.  If
     * out of range, return NULL.     
     */
    virtual DOMImplementation *item(unsigned long index)
        {
        if (index >implementations.size())
            return NULL;
        return implementations[index];
        }

    /**
     * Return the number of DOMImplementations in this list.
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
    DOMImplementationList &operator=(const DOMImplementationList &other)
        {
        implementations = other.implementations;
        return *this;
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

/**
 * This is usually the first item to be called when creating a Document.
 * You will either find one DOMImplementation with a given set of features,
 * or return a list that match.  Using "" will get any implementation
 * available.
 */    
class DOMImplementationSource
{
public:

    /**
     *  Return the first DOMImplementation with the given set of features.
     *  Use "" to fetch any implementation. 
     */
    virtual DOMImplementation *getDOMImplementation(const DOMString &features) = 0;

    /**
     *  Return a list of DOMImplementations with the given set of features.
     *  Use "" to fetch any implementation. 
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
 * This is the class that actually creates a Document. 
 */
class DOMImplementation
{
public:

    /**
     *  Determine if this implementation has the given feature and version.
     */
    virtual bool hasFeature(const DOMString& feature, const DOMString& version) = 0;


    /**
     *  Create a document type to be used in creating documents.
     */
    virtual DocumentTypePtr createDocumentType(
	                               const DOMString& qualifiedName,
                                   const DOMString& publicId,
                                   const DOMString& systemId)
                                   throw(DOMException) = 0;

    /**
     *  Create a DOM document.
     */
    virtual DocumentPtr createDocument(const DOMString& namespaceURI,
                             const DOMString& qualifiedName,
                             DocumentTypePtr doctype)
                             throw(DOMException) = 0;
    /**
     *  Return the thing which is the feature of this implementation.  Since
     *  this is a "one size fits all" call, you will need to typecast the
     *  result to the expected type.	      
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
 *  The basic Node class, which is the root of most other
 *  classes in DOM.
 */
class Node
{
public:

    /**
     * Which of the DOM Core node types is this node?
     */	     
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
     * Return the name of this node.
     */
    virtual DOMString getNodeName() = 0;

    /**
     *  Return the value of this node.  The interpretation of the
     *  value is type-specific.     
     */
    virtual DOMString getNodeValue() throw (DOMException) = 0;

    /**
     *  Set the value of this node.  The interpretation of the
     *  value is type-specific.     
     */
    virtual void setNodeValue(const DOMString& val) throw (DOMException) = 0;

    /**
     *  Return the type of this Node.  One of the NodeType values above.
     */
    virtual unsigned short getNodeType() = 0;

    /**
     *  Return the parent which references this node as a child in the DOM
     *  tree.  Return NULL if there is none.     
     */
    virtual NodePtr getParentNode() = 0;

    /**
     * Return a list of the children of this Node.
     * NOTE: the spec expects this to be a "live" list that always
     * reflects an accurate list of what the Node current possesses, not
     * a snapshot.  How do we do this?	 	      
     */
    virtual NodeList getChildNodes() = 0;

    /**
     *
     */
    virtual NodePtr getFirstChild() = 0;

    /**
     *
     */
    virtual NodePtr getLastChild() = 0;

    /**
     *
     */
    virtual NodePtr getPreviousSibling() = 0;

    /**
     *
     */
    virtual NodePtr getNextSibling() = 0;

    /**
     *
     */
    virtual NamedNodeMap &getAttributes() = 0;


    /**
     *
     */
    virtual DocumentPtr getOwnerDocument() = 0;

    /**
     *
     */
    virtual NodePtr insertBefore(const NodePtr newChild,
                       const NodePtr refChild)
                       throw(DOMException) = 0;

    /**
     *
     */
    virtual NodePtr replaceChild(const NodePtr newChild,
                       const NodePtr oldChild)
                       throw(DOMException) = 0;

    /**
     *
     */
    virtual NodePtr removeChild(const NodePtr oldChild)
                      throw(DOMException) = 0;

    /**
     *
     */
    virtual NodePtr appendChild(const NodePtr newChild)
                      throw(DOMException) = 0;

    /**
     *
     */
    virtual bool hasChildNodes() = 0;

    /**
     *
     */
    virtual NodePtr cloneNode(bool deep) = 0;

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
    virtual unsigned short compareDocumentPosition(
	                         const NodePtr other) = 0;

    /**
     *
     */
    virtual DOMString getTextContent() throw(DOMException) = 0;


    /**
     *
     */
    virtual void setTextContent(const DOMString &val) throw(DOMException) = 0;


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
    virtual bool isEqualNode(const NodePtr node) =0;



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
    Node() : _refCnt(0)
        {}

    /**
     *
     */
    virtual ~Node() {}

protected:

    friend void incrementRefCount(Node *p);
    friend void decrementRefCount(Node *p);
 
    /**
     * For the Ptr smart pointer
     */	     
    int _refCnt;

};




/*#########################################################################
## NodeList
#########################################################################*/

/**
 *  Contains a list of Nodes.  This is the standard API container for Nodes,
 *  and is used in lieu of other lists, arrays, etc, in order to provide
 *  a consistent API and algorithm.  
 */
class NodeList
{
public:

    /**
     *  Retrieve the Node at the given index.  Return NULL
     *  if out of range.     
     */
    virtual NodePtr item(unsigned long index)
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
    NodeList &operator=(const NodeList &other)
        {
        nodes = other.nodes;
        return *this;
        }

    /**
     *
     */
    virtual ~NodeList() {}

    /**
     *
     */
    virtual void clear()
        {
        nodes.clear();
        }

protected:

friend class NodeImpl;
friend class ElementImpl;

    /*
     *
     */
    virtual void add(const NodePtr node)
        {
        nodes.push_back(node);
        }

protected:

    std::vector<NodePtr> nodes;

};




/*#########################################################################
## NamedNodeMap
#########################################################################*/

class NamedNodeMapEntry
{
public:
    NamedNodeMapEntry(const DOMString &theNamespaceURI,
                      const DOMString &theName,
                      const NodePtr   theNode)
        {
        namespaceURI = theNamespaceURI;
        name         = theName;
        node         = theNode;
        }
    NamedNodeMapEntry(const NamedNodeMapEntry &other)
        {
        assign(other);
        }
    NamedNodeMapEntry &operator=(const NamedNodeMapEntry &other)
        {
        assign(other);
        return *this;
        }
    virtual ~NamedNodeMapEntry()
        {
        }
    void assign(const NamedNodeMapEntry &other)
        {
        namespaceURI = other.namespaceURI;
        name         = other.name;
        node         = other.node;
        }
    DOMString namespaceURI;
    DOMString name;
    NodePtr   node;
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
    virtual NodePtr getNamedItem(const DOMString& name)
        {
        std::vector<NamedNodeMapEntry>::iterator iter;
        for (iter = entries.begin() ; iter!=entries.end() ; iter++)
            {
            if (iter->name == name)
                {
                NodePtr node = iter->node;
                return node;
                }
            }
        return NULL;
        }

    /**
     *
     */
    virtual NodePtr setNamedItem(NodePtr arg) throw(DOMException)
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
                NodePtr node = iter->node;
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
    virtual NodePtr removeNamedItem(const DOMString& name) throw(DOMException)
        {
        std::vector<NamedNodeMapEntry>::iterator iter;
        for (iter = entries.begin() ; iter!=entries.end() ; iter++)
            {
            if (iter->name == name)
                {
                NodePtr node = iter->node;
                entries.erase(iter);
                return node;
                }
            }
        return NULL;
        }

    /**
     *
     */
    virtual NodePtr item(unsigned long index)
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
    virtual NodePtr getNamedItemNS(const DOMString& namespaceURI,
                                 const DOMString& localName)
        {
        std::vector<NamedNodeMapEntry>::iterator iter;
        for (iter = entries.begin() ; iter!=entries.end() ; iter++)
            {
            if (iter->namespaceURI == namespaceURI && iter->name == localName)
                {
                NodePtr node = iter->node;
                return node;
                }
            }
        return NULL;
        }

    /**
     *
     */
    virtual NodePtr setNamedItemNS(NodePtr arg) throw(DOMException)
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
                NodePtr node = iter->node;
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
    virtual NodePtr removeNamedItemNS(const DOMString& namespaceURI,
                                    const DOMString& localName)
                                    throw(DOMException)
        {
        std::vector<NamedNodeMapEntry>::iterator iter;
        for (iter = entries.begin() ; iter!=entries.end() ; iter++)
            {
            if (iter->namespaceURI == namespaceURI && iter->name == localName)
                {
                NodePtr node = iter->node;
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
    NamedNodeMap &operator=(const NamedNodeMap &other)
        {
        entries = other.entries;
        return *this;
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


typedef Ptr<CharacterData> CharacterDataPtr;




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
    virtual ElementPtr getOwnerElement() = 0;


    /**
     *
     */
    virtual TypeInfo &getSchemaTypeInfo() = 0;


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
    virtual AttrPtr getAttributeNode(const DOMString& name) = 0;

    /**
     *
     */
    virtual AttrPtr setAttributeNode(AttrPtr newAttr)
                          throw(DOMException) = 0;

    /**
     *
     */
    virtual AttrPtr removeAttributeNode(AttrPtr oldAttr)
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
    virtual AttrPtr getAttributeNodeNS(const DOMString& namespaceURI,
                            const DOMString& localName) = 0;

    /**
     *
     */
    virtual AttrPtr setAttributeNodeNS(AttrPtr newAttr)
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
    virtual TypeInfo &getSchemaTypeInfo() = 0;


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
    virtual void setIdAttributeNode(const AttrPtr idAttr,
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
    virtual TextPtr splitText(unsigned long offset)
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
    virtual TextPtr replaceWholeText(const DOMString &content)
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
    virtual DOMString getTypeName()
        { return typeName; }

    /**
     *
     */
    virtual DOMString getTypeNamespace()
        { return typeNameSpace; }

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
                               DerivationMethod derivationMethod)
        { (void)typeNamespaceArg; (void)typeNameArg; (void)derivationMethod; return false; }

    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    TypeInfo() 
	    {}
	    
    /**
     *
     */
    TypeInfo(const TypeInfo &other)
        { assign(other); }
        
    /**
     *
     */
    TypeInfo &operator=(const TypeInfo &other)
        { assign(other); return *this; }
        
    /**
     *
     */
    virtual ~TypeInfo() {}
    
private:

    void assign(const TypeInfo &other)
        {
        typeName      = other.typeName;
        typeNameSpace = other.typeNameSpace;
        }

    DOMString typeName;
    DOMString typeNameSpace;
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
                         const NodePtr src,
                         const NodePtr dst) =0;

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
        SEVERITY_WARNING     = 1,
        SEVERITY_ERROR       = 2,
        SEVERITY_FATAL_ERROR = 3
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
    virtual NodePtr getRelatedNode() =0;


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
                              const DOMUserData *value)
							   throw (DOMException) =0;

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
    virtual DocumentTypePtr getDoctype() = 0;

    /**
     *
     */
    virtual DOMImplementation *getImplementation() = 0;

    /**
     *
     */
    virtual ElementPtr getDocumentElement() = 0;

    /**
     *
     */
    virtual ElementPtr createElement(const DOMString& tagName)
                           throw(DOMException) = 0;

    /**
     *
     */
    virtual DocumentFragmentPtr createDocumentFragment() = 0;

    /**
     *
     */
    virtual TextPtr createTextNode(const DOMString& data) = 0;

    /**
     *
     */
    virtual CommentPtr createComment(const DOMString& data) = 0;

    /**
     *
     */
    virtual CDATASectionPtr createCDATASection(const DOMString& data)
                                     throw(DOMException) = 0;

    /**
     *
     */
    virtual ProcessingInstructionPtr
	           createProcessingInstruction(const DOMString& target,
                                           const DOMString& data)
                                           throw(DOMException) = 0;

    /**
     *
     */
    virtual AttrPtr createAttribute(const DOMString& name)
                          throw(DOMException) = 0;

    /**
     *
     */
    virtual EntityReferencePtr createEntityReference(const DOMString& name)
                                           throw(DOMException) = 0;

    /**
     *
     */
    virtual NodeList getElementsByTagName(const DOMString& tagname) = 0;


    /**
     *
     */
    virtual NodePtr importNode(const NodePtr importedNode,
                     bool deep)
                     throw(DOMException) = 0;

    /**
     *
     */
    virtual ElementPtr createElementNS(const DOMString& namespaceURI,
                             const DOMString& qualifiedName)
                             throw(DOMException) = 0;

    /**
     *
     */
    virtual AttrPtr createAttributeNS(const DOMString& namespaceURI,
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
    virtual ElementPtr getElementById(const DOMString& elementId) = 0;


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
    virtual void setXmlVersion(const DOMString &version)
	                           throw (DOMException) = 0;

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
    virtual NodePtr adoptNode(const NodePtr source) throw (DOMException) = 0;

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
    virtual NodePtr renameNode(const NodePtr n,
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




