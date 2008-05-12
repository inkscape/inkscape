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
 *  
 * =======================================================================
 *  NOTES:
 * 
 *  Notice that many of the classes defined here are pure virtual.  In other
 *  words, they are purely unimplemented interfaces.  For the implementations
 *  of them, look in domimpl.h and domimpl.cpp.
 *  
 *  Also, note that there is a domptr.cpp file that has a couple of necessary
 *  functions which cannot be in a .h file.
 *  
 *  Some of the comments below are quoted from the W3C spec:
 *  http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/core.html 
 *             
 */

#include <vector>

#include "domptr.h"


/**
 * What type of string do we want?  Pick one of the following
 * Then below, select one of the corresponding typedefs.
 */ 

//#include <glibmm.h>
#include <string>

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
//typedef Glib::ustring DOMString;
//typedef gunichar XMLCh;

typedef std::string DOMString;
typedef unsigned short XMLCh;


/**
 *  At least 64 bit time stamp value.
 */
typedef unsigned long long DOMTimeStamp;

/**
 *  This is used for storing refs to user-supplied data.
 */
typedef void DOMUserData;



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
 *  classes in DOM.  Thus it is by far the most important, and the one
 *  whose implementation we must perform correctly. 
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
     * Return the first sibling of the chidren of this node.  Return
     * null if there is none.     
     */
    virtual NodePtr getFirstChild() = 0;

    /**
     * Return the last sibling of the children of this node.  Return
     * null if there is none.     
     */
    virtual NodePtr getLastChild() = 0;

    /**
     * Return the node that is previous to this one in the parent's
     * list of children.  Return null if there is none.     
     */
    virtual NodePtr getPreviousSibling() = 0;

    /**
     * Return the node that is after this one in the parent's list
     * of children.  Return null if there is none.     
     */
    virtual NodePtr getNextSibling() = 0;

    /**
     * Get the list of all attributes of this node.
     */
    virtual NamedNodeMap &getAttributes() = 0;


    /**
     * Return the document that created or inherited this node.
     */
    virtual DocumentPtr getOwnerDocument() = 0;

    /**
     * Insert a node as a new child.  Place it before the referenced child.
     * Place it at the end if the referenced child does not exist.     
     */
    virtual NodePtr insertBefore(const NodePtr newChild,
                       const NodePtr refChild)
                       throw(DOMException) = 0;

    /**
     * Insert a node as a new child.  Replace the referenced child with it.
     * Place it at the end if the referenced child does not exist.     
     */
    virtual NodePtr replaceChild(const NodePtr newChild,
                       const NodePtr oldChild)
                       throw(DOMException) = 0;

    /**
     * Remove a node from the list of children.  Do nothing if the
     * node is not a member of the child list.     
     */
    virtual NodePtr removeChild(const NodePtr oldChild)
                      throw(DOMException) = 0;

    /**
     *  Add the node to the end of this node's child list.
     */
    virtual NodePtr appendChild(const NodePtr newChild)
                      throw(DOMException) = 0;

    /**
     * Return true if this node has one or more children, else return false.
     */
    virtual bool hasChildNodes() = 0;

    /**
     * Return a new node which has the name, type, value, attributes, and
     * child list as this one.	    
     * If 'deep' is true, continue cloning recursively with this node's children,
     * so that the child list also contains clones of their respective nodes.     
     */
    virtual NodePtr cloneNode(bool deep) = 0;

    /**
     *  Adjust this node and its children to have its namespaces and
     *  prefixes in "canonical" order.     
     */
    virtual void normalize() = 0;

    /**
     *  Return true if the named feature is supported by this node,
     *  else false.     
     */
    virtual bool isSupported(const DOMString& feature,
                     const DOMString& version) = 0;

    /**
     * Return the namespace of this node.  This would be whether the
     * namespace were declared explicitly on this node, it has a namespace
     * prefix, or it is inherits the namespace from an ancestor node.	      
     */
    virtual DOMString getNamespaceURI() = 0;

    /**
     * Return the namespace prefix of this node, if any.  For example, if
     * the tag were <svg:image> then the prefix would be "svg"     
     */
    virtual DOMString getPrefix() = 0;

    /**
     *  Sets the namespace prefix of this node to the given value.  This
     *  does not change the namespaceURI value.     
     */
    virtual void setPrefix(const DOMString& val) throw(DOMException) = 0;

    /**
     * Return the local name of this node.  This is merely the name without
     * any namespace or prefix.     
     */
    virtual DOMString getLocalName() = 0;

    /**
     * Return true if this node has one or more attributes, else false.
     */
    virtual bool hasAttributes() = 0;

    /**
     * Return the base URI of this node.  This is basically the "location" of this
     * node, and is used in resolving the relative locations of other URIs.     
     */
    virtual DOMString getBaseURI() = 0;

    /**
     * DocumentPosition.
     * This is used to describe the position of one node relative
     * to another in a document
     */	 	 	     
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
     * Get the position of this node relative to the node argument.
     */
    virtual unsigned short compareDocumentPosition(
	                         const NodePtr other) = 0;

    /**
     * This is a DOM L3 method.  Return the text value of this node and its
     * children.  This is done by concatenating all of the TEXT_NODE and
     * CDATA_SECTION nodes of this node and its children, in order, together.
     *  Very handy.	 	 	      
     */
    virtual DOMString getTextContent() throw(DOMException) = 0;


    /**
     * This is a DOM L3 method.  Remember, this is a destructive call.  This
     * will replace all of the child nodes of this node with a single TEXT_NODE
     * with the given text value.      
     */
    virtual void setTextContent(const DOMString &val) throw(DOMException) = 0;


    /**
     *  This will search the tree from this node up, for a prefix that
     *  has been assigned to the namespace argument.  Return "" if not found.     
     */
    virtual DOMString lookupPrefix(const DOMString &namespaceURI) =0;


    /**
     *  Return true if this node is in the namespace of the argument, without
     *  requiring an explicit namespace declaration or a suffix.     
     */
    virtual bool isDefaultNamespace(const DOMString &namespaceURI) =0;


    /**
     * This will search the tree from this node up, for a namespace that
     * has been assigned the suffix in the argument. Return "" if not found.     
     */
    virtual DOMString lookupNamespaceURI(const DOMString &prefix) =0;


    /**
     * Return true if the argument node is equal to this one.  Use W3C rules
     * for equality.     
     */
    virtual bool isEqualNode(const NodePtr node) =0;



    /**
     * Return an opaque reference to the named feature.  Return null if
     * not supported.  Using other than "" for the version will look for
     * a feature with the given version.	      
     */
    virtual DOMObject *getFeature(const DOMString &feature,
                                 const DOMString &version) =0;

    /**
     * Store a user data reference in this node, using the given key.
     * A handler is an optional function object that will be called during
     * future settings of this value.  See UserDataHandler for more info.	      
     */
    virtual DOMUserData *setUserData(const DOMString &key,
                                     const DOMUserData *data,
                                     const UserDataHandler *handler) =0;


    /**
     *  Return a reference to the named user data object. Return null
     *  if it does not exist.     
     */
    virtual DOMUserData *getUserData(const DOMString &key) =0;

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
     * Get the number of nodes in this list
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

/**
 * Contains a mapping from name->NodePtr.  Used for various purposes.  For
 * example, a list of Attributes is a NamedNodeMap. 
 */
class NamedNodeMap
{
private:

    /**
     * table entry.  Not an API item
     */	     
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


public:

    /**
     * Return the named node.  Return nullptr if not found.
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
     * Adds a node using its nodeName attribute. If a node with that name is already
     * present in this map, it is replaced by the new one. Replacing a node by itself
     * has no effect.
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
     * Removes a node specified by name.
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
     *  Retrieves an item at the given index.  If out of bounds, return NULL
     */
    virtual NodePtr item(unsigned long index)
        {
        if (index>=entries.size())
            return NULL;
        return entries[index].node;
        }

    /**
     * Return the number of items in this map
     */
    virtual unsigned long getLength()
        {
        return (unsigned long)entries.size();
        }

    /**
     * Retrieves a node specified by local name and namespace URI.
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
     * Adds a node using its namespaceURI and localName. If a node with that
     * namespace URI and that local name is already present in this map, it is
     * replaced by the new one. Replacing a node by itself has no effect.
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
     * Removes a node specified by local name and namespace URI.
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
 * This is the base class for other text-oriented Nodes, such as TEXT_NODE
 * or CDATA_SECTION_NODE. No DOM objects correspond directly to CharacterData.
 */
class CharacterData : virtual public Node
{
public:

    /**
     * This is an alias for getNodeValue()
     */
    virtual DOMString getData() throw(DOMException) = 0;

    /**
     * This is an alias for setNodeValue()
     */
    virtual void setData(const DOMString& val) throw(DOMException) = 0;

    /**
     * Return the number of characters contained in this node's data
     */
    virtual unsigned long getLength() = 0;

    /**
     * Return a substring of this node's data, starting at offset, and
     * continuing for 'count' characters. Throw an exception if this goes
     * out of range.	   
     */
    virtual DOMString substringData(unsigned long offset,
                            unsigned long count)
                            throw(DOMException) = 0;

    /**
     *  Append the argument string to the end of the node's current data.
     */
    virtual void appendData(const DOMString& arg) throw(DOMException) = 0;

    /**
     * Insert the argument string at the offset position into the node's
     * current data.  If the position is out of range, throw an Exception. 
     */
    virtual void insertData(unsigned long offset,
                    const DOMString& arg)
                    throw(DOMException) = 0;

    /**
     * Delete 'count' characters from the node's data starting from the
     * offset position.  If this goes out of range, throw an Exception.     
     */
    virtual void deleteData(unsigned long offset,
                    unsigned long count)
                    throw(DOMException) = 0;

    /**
     *  Replace the 'count' characters at the offset position with the given
     *  argument string. If this goes out of range, throw an Exception.
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
 *  The Attr interface represents an attribute in an Element object.
 *  Typically the allowable values for the attribute are defined in a
 *  schema associated with the document.
 *  Since Attrs are not considered to be part of the DOM tree, parent,
 *  previousSibling, and nextSibling are null. 
 */
class Attr : virtual public Node
{
public:

    /**
     * Returns the name of this attribute. If Node.localName is different
     * from null, this attribute is a qualified name.
     */
    virtual DOMString getName() = 0;

    /**
     * True if this attribute was explicitly given a value in the instance document,
     * false otherwise. If the application changed the value of this attribute node
     * (even if it ends up having the same value as the default value) then it is set
     * to true. The implementation may handle attributes with default values from
     * other schemas similarly but applications should use
     * Document.normalizeDocument() to guarantee this information is up-to-date.
     */
    virtual bool getSpecified() = 0;

    /**
     * Returns the value of the attribute
     */
    virtual DOMString getValue() = 0;

    /**
     * Sets the value of the attribute
     */
    virtual void setValue(const DOMString& val) throw(DOMException) = 0;

    /**
     * Return the Element that possesses this attribute
     */
    virtual ElementPtr getOwnerElement() = 0;


    /**
     * The type information associated with this attribute.
     */
    virtual TypeInfo &getSchemaTypeInfo() = 0;


    /**
     * Returns whether this attribute is known to be of type ID (i.e. to contain an
     * identifier for its owner element) or not. When it is and its value is unique,
     * the ownerElement of this attribute can be retrieved using the method
     * Document.getElementById.
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
 * The Element interface represents an element in an XML document. 
 * Elements may have attributes associated with them; since the Element interface 
 * inherits from Node, the generic Node interface attribute attributes may be 
 * used to retrieve the set of all attributes for an element. There are methods 
 * on the Element interface to retrieve either an Attr object by name or an 
 * attribute value by name. In XML, where an attribute value may contain entity 
 * references, an Attr object should be retrieved to examine the possibly fairly 
 * complex sub-tree representing the attribute value. On the other hand, in HTML, 
 * where all attributes have simple string values, methods to directly access an 
 * attribute value can safely be used as a convenience.
 */
class Element : virtual public Node
{
public:


    /**
     * The name of the element. If Node.localName is different from null,
     * this attribute is a qualified name.
     */
    virtual DOMString getTagName() = 0;

    /**
     * Retrieves an attribute value by name.
     */
    virtual DOMString getAttribute(const DOMString& name) = 0;

    /**
     * Adds a new attribute. If an attribute with that name is already present in the
     * element, its value is changed to be that of the value parameter. This value is
     * a simple string; it is not parsed as it is being set. So any markup (such as
     * syntax to be recognized as an entity reference) is treated as literal text,
     * and needs to be appropriately escaped by the implementation when it is written
     * out. In order to assign an attribute value that contains entity references,
     * the user must create an Attr node plus any Text and EntityReference nodes,
     * build the appropriate subtree, and use setAttributeNode to assign it as the
     * value of an attribute.
     */
    virtual void setAttribute(const DOMString& name,
                      const DOMString& value)
                      throw(DOMException) = 0;

    /**
     * Removes an attribute by name. If no attribute with this name is found,
     * this method has no effect.
     */
    virtual void removeAttribute(const DOMString& name)
                         throw(DOMException) = 0;

    /**
     * Retrieves an attribute node by name.
     */
    virtual AttrPtr getAttributeNode(const DOMString& name) = 0;

    /**
     * Adds a new attribute node. If an attribute with that name (nodeName)
     * is already present in the element, it is replaced by the new one.
     * Replacing an attribute node by itself has no effect.
     */
    virtual AttrPtr setAttributeNode(AttrPtr newAttr)
                          throw(DOMException) = 0;

    /**
     * Removes the specified attribute node.
     */
    virtual AttrPtr removeAttributeNode(AttrPtr oldAttr)
                             throw(DOMException) = 0;

    /**
     * Returns a NodeList of all descendant Elements  with a given tag name,
     * in document order.
     */
    virtual NodeList getElementsByTagName(const DOMString& name) = 0;

    /**
     * Retrieves an attribute value by local name and namespace URI.
     * Per [XML Namespaces], applications must use the value null as the
     * namespaceURI parameter for methods if they wish to have no namespace.
     */
    virtual DOMString getAttributeNS(const DOMString& namespaceURI,
                             const DOMString& localName) = 0;

    /**
     * Adds a new attribute. If an attribute with the same local name and namespace
     * URI is already present on the element, its prefix is changed to be the prefix
     * part of the qualifiedName, and its value is changed to be the value parameter.
     * This value is a simple string; it is not parsed as it is being set. So any
     * markup (such as syntax to be recognized as an entity reference) is treated as
     * literal text, and needs to be appropriately escaped by the implementation when
     * it is written out. In order to assign an attribute value that contains entity
     * references, the user must create an Attr node plus any Text and
     * EntityReference nodes, build the appropriate subtree, and use
     * setAttributeNodeNS or setAttributeNode to assign it as the value of an
     * attribute.
     */
    virtual void setAttributeNS(const DOMString& namespaceURI,
                        const DOMString& qualifiedName,
                        const DOMString& value)
                        throw(DOMException) = 0;

    /**
     * Removes an attribute by local name and namespace URI.
     */
    virtual void removeAttributeNS(const DOMString& namespaceURI,
                           const DOMString& localName)
                           throw(DOMException) = 0;

    /**
     * Retrieves an Attr node by local name and namespace URI.
     */
    virtual AttrPtr getAttributeNodeNS(const DOMString& namespaceURI,
                            const DOMString& localName) = 0;

    /**
     * Adds a new attribute. If an attribute with that local name and
     * that namespace URI is already present in the element, it is
     * replaced by the new one. Replacing an attribute node by itself has no effect.
     */
    virtual AttrPtr setAttributeNodeNS(AttrPtr newAttr)
                            throw(DOMException) = 0;

    /**
     * Returns a NodeList of all the descendant Elements  with a given
     * local name and namespace URI in document order.
     */
    virtual NodeList getElementsByTagNameNS(const DOMString& namespaceURI,
                                    const DOMString& localName) = 0;

    /**
     * Returns true when an attribute with a given name is specified on
     * this element or has a default value, false  otherwise.
     */
    virtual bool hasAttribute(const DOMString& name) = 0;

    /**
     * Returns true when an attribute with a given local name and namespace
     * URI is specified on this element or has a default value, false otherwise.
     */
    virtual bool hasAttributeNS(const DOMString& namespaceURI,
                        const DOMString& localName) = 0;

    /**
     * The type information associated with this element.
     */
    virtual TypeInfo &getSchemaTypeInfo() = 0;


    /**
     * If the parameter isId is true, this method declares the specified
     * attribute to be a user-determined ID attribute.
     */
    virtual void setIdAttribute(const DOMString &name,
                                bool isId) throw (DOMException) = 0;

    /**
     * If the parameter isId is true, this method declares the specified
     * attribute to be a user-determined ID attribute.
     */
    virtual void setIdAttributeNS(const DOMString &namespaceURI,
                                  const DOMString &localName,
                                  bool isId) throw (DOMException) = 0;

    /**
     * If the parameter isId is true, this method declares the specified
     * attribute to be a user-determined ID attribute.
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
 * The Text interface inherits from CharacterData and represents the textual 
 * content (termed character data in XML) of an Element or Attr. If there is no 
 * markup inside an element's content, the text is contained in a single object 
 * implementing the Text interface that is the only child of the element. If 
 * there is markup, it is parsed into the information items (elements, comments, 
 * etc.) and Text nodes that form the list of children of the element.
 */
class Text : virtual public CharacterData
{
public:

    /**
     * Breaks this node into two nodes at the specified offset, keeping both in the
     * tree as siblings. After being split, this node will contain all the content up
     * to the offset point. A new node of the same type, which contains all the
     * content at and after the offset point, is returned. If the original node had a
     * parent node, the new node is inserted as the next sibling of the original
     * node. When the offset is equal to the length of this node, the new node has no
     * data.
     */
    virtual TextPtr splitText(unsigned long offset)
                    throw(DOMException) = 0;

    /**
     * Returns whether this text node contains element content whitespace, often
     * abusively called "ignorable whitespace". The text node is determined to
     * contain whitespace in element content during the load of the document or if
     * validation occurs while using Document.normalizeDocument().
     */
    virtual bool getIsElementContentWhitespace()= 0;

    /**
     * Returns all text of Text nodes logically-adjacent text nodes
     * to this node, concatenated in document order.
     */
    virtual DOMString getWholeText() = 0;


    /**
     * Replaces the text of the current node and all logically-adjacent text nodes
     * with the specified text. All logically-adjacent text nodes are removed
     * including the current node unless it was the recipient of the replacement text.
     *
     * This method returns the node which received the replacement text. The returned
     * node is:
     *    o  null, when the replacement text is the empty string;
     *    o  the current node, except when the current node is read-only;
     *    o  a new Text node of the same type (Text or CDATASection) as
     *          the current node inserted at the location of the replacement.
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
 * This interface inherits from CharacterData and represents the content of a 
 * comment, i.e., all the characters between the starting '<!--' and ending '-->'.
 * Note that this is the definition of a comment in XML, and, in practice, 
 * HTML, although some HTML tools may implement the full SGML comment structure.
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
 * The TypeInfo interface represents a type referenced from Element or Attr nodes,
 *  specified in the schemas associated with the document. The type is a pair of 
 * a namespace URI and name properties, and depends on the document's schema.
 */
class TypeInfo
{
public:

    /**
     * The name of a type declared for the associated element or attribute,
     *  or null if unknown.
     */
    virtual DOMString getTypeName()
        { return typeName; }

    /**
     * The namespace of the type declared for the associated element
     * or attribute or null if the element does not have declaration or
     * if no namespace information is available.
     */
    virtual DOMString getTypeNamespace()
        { return typeNameSpace; }

    /**
     * These are the available values for the derivationMethod parameter used by the
     * method TypeInfo.isDerivedFrom(). It is a set of possible types of derivation,
     * and the values represent bit positions. If a bit in the derivationMethod
     * parameter is set to 1, the corresponding type of derivation will be taken into
     * account when evaluating the derivation between the reference type definition
     * and the other type definition. When using the isDerivedFrom method, combining
     * all of them in the derivationMethod parameter is equivalent to invoking the
     * method for each of them separately and combining the results with the OR
     * boolean function. This specification only defines the type of derivation for
     * XML Schema.
     */	     
    typedef enum
        {
        DERIVATION_RESTRICTION = 0x00000001,
        DERIVATION_EXTENSION   = 0x00000002,
        DERIVATION_UNION       = 0x00000004,
        DERIVATION_LIST        = 0x00000008
        } DerivationMethod;


    /**
     * This method returns if there is a derivation between the reference
     * type definition, i.e. the TypeInfo on which the method is being called,
     * and the other type definition, i.e. the one passed as parameters.
     */
    virtual bool isDerivedFrom(const DOMString &/*typeNamespaceArg*/,
                               const DOMString &/*typeNameArg*/,
                               DerivationMethod /*derivationMethod*/)
        { return false; }

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
 * When associating an object to a key on a node using Node.setUserData() the 
 * application can provide a handler that gets called when the node the object is 
 * associated to is being cloned, imported, or renamed. This can be used by the 
 * application to implement various behaviors regarding the data it associates to 
 * the DOM nodes. This interface defines that handler.
 */
class UserDataHandler
{
public:

    /**
     * An integer indicating the type of operation being performed on a node.
     */
    typedef enum
        {
        NODE_CLONED     = 1,
        NODE_IMPORTED   = 2,
        NODE_DELETED    = 3,
        NODE_RENAMED    = 4,
        NODE_ADOPTED    = 5
        } OperationType;


    /**
     * This method is called whenever the node for which this handler
     * is registered is imported or cloned.
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
 * DOMError is an interface that describes an error. 
 */
class DOMError
{
public:

    /**
     * An integer indicating the severity of the error.
     */
    typedef enum
        {
        SEVERITY_WARNING     = 1,
        SEVERITY_ERROR       = 2,
        SEVERITY_FATAL_ERROR = 3
        } ErrorSeverity;


    /**
     * The severity of the error, either SEVERITY_WARNING, SEVERITY_ERROR,
     * or SEVERITY_FATAL_ERROR.
     */
    virtual unsigned short getSeverity() =0;

    /**
     * An implementation specific string describing the error that occurred.
     */
    virtual DOMString getMessage() =0;

    /**
     * A DOMString indicating which related data is expected in relatedData.
     * Users should refer to the specification of the error in order to find
     * its DOMString type and relatedData  definitions if any.
     */
    virtual DOMString getType() =0;

    /**
     * The related platform dependent exception if any.
     */
    virtual DOMObject *getRelatedException() =0;

    /**
     * The related DOMError.type dependent data if any.
     */
    virtual DOMObject *getRelatedData() =0;

    /**
     * The location of the error.
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
 * DOMErrorHandler is a callback interface that the DOM implementation can call 
 * when reporting errors that happens while processing XML data, or when doing 
 * some other processing (e.g. validating a document). A DOMErrorHandler object 
 * can be attached to a Document using the "error-handler" on the 
 * DOMConfiguration interface. If more than one error needs to be reported during 
 * an operation, the sequence and numbers of the errors passed to the error 
 * handler are implementation dependent.
 */
class DOMErrorHandler
{
public:

    /**
     * This method is called on the error handler when an error occurs.
     * If an exception is thrown from this method, it is considered to be
     * equivalent of returning true.
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
 * DOMLocator is an interface that describes a location (e.g. where an error occurred). 
 */
class DOMLocator
{
public:

    /**
     * The line number this locator is pointing to, or -1 if there is
     * no column number available.
     */
    virtual long getLineNumber() =0;

    /**
     * The column number this locator is pointing to, or -1 if there is
     * no column number available.
     */
    virtual long getColumnNumber() =0;

    /**
     * The byte offset into the input source this locator is pointing to
     * or -1 if there is no byte offset available.
     */
    virtual long getByteOffset() =0;

    /**
     * The UTF-16, as defined in [Unicode] and Amendment 1 of [ISO/IEC 10646],
     * offset into the input source this locator is pointing to or -1
     * if there is no UTF-16 offset available.
     */
    virtual long getUtf16Offset() =0;


    /**
     * The node this locator is pointing to, or null if no node is available.
     */
    virtual NodePtr getRelatedNode() =0;


    /**
     * The URI this locator is pointing to, or null if no URI is available.
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
 * The DOMConfiguration interface represents the configuration of a document and 
 * maintains a table of recognized parameters. Using the configuration, it is 
 * possible to change Document.normalizeDocument() behavior, such as replacing 
 * the CDATASection nodes with Text nodes or specifying the type of the schema 
 * that must be used when the validation of the Document is requested. 
 * DOMConfiguration objects are also used in [DOM Level 3 Load and Save] in the 
 * DOMParser and DOMSerializer interfaces.
 *
 * Look here for a list of valid parameters:
 * http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/core.html#DOMConfiguration
 */
class DOMConfiguration
{
public:

    /**
     * Set the value of a parameter.
     */
    virtual void setParameter(const DOMString &name,
                              const DOMUserData *value)
							   throw (DOMException) =0;

    /**
     * Return the value of a parameter if known.
     */
    virtual DOMUserData *getParameter(const DOMString &name)
                                      throw (DOMException) =0;

    /**
     * Check if setting a parameter to a specific value is supported.
     */
    virtual bool canSetParameter(const DOMString &name,
                                 const DOMUserData *data) =0;

    /**
     * The list of the parameters supported by this DOMConfiguration
     * object and for which at least one value can be set by the application.
     * Note that this list can also contain parameter names defined outside
     * this specification.
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
 * CDATA sections are used to escape blocks of text containing characters that 
 * would otherwise be regarded as markup. The only delimiter that is recognized 
 * in a CDATA section is the "]]>" string that ends the CDATA section. CDATA 
 * sections cannot be nested. Their primary purpose is for including material 
 * such as XML fragments, without needing to escape all the delimiters.
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
 * Each Document has a doctype attribute whose value is either null or a 
 * DocumentType object. The DocumentType interface in the DOM Core provides an 
 * interface to the list of entities that are defined for the document, and 
 * little else because the effect of namespaces and the various XML schema 
 * efforts on DTD representation are not clearly understood as of this writing.
 */
class DocumentType : virtual public Node
{
public:

    /**
     * The name of DTD; i.e., the name immediately following the DOCTYPE keyword.
     */
    virtual DOMString getName() = 0;

    /**
     * A NamedNodeMap containing the general entities, both external and
     * internal, declared in the DTD. Parameter entities are not contained.
     * Duplicates are discarded.
     */
    virtual NamedNodeMap getEntities() = 0;

    /**
     * A NamedNodeMap containing the notations declared in the DTD. Duplicates
     * are discarded. Every node in this map also implements the
     * Notation interface.
     */
    virtual NamedNodeMap getNotations() = 0;

    /**
     * The public identifier of the external subset.
     */
    virtual DOMString getPublicId() = 0;

    /**
     * The system identifier of the external subset. This may be an
     * absolute URI or not.
     */
    virtual DOMString getSystemId() = 0;

    /**
     * The internal subset as a string, or null if there is none. This
     * does not contain the delimiting square brackets.
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
 * This interface represents a notation declared in the DTD. A notation either 
 * declares, by name, the format of an unparsed entity (see section 4.7 of the 
 * XML 1.0 specification [XML 1.0]), or is used for formal declaration of 
 * processing instruction targets (see section 2.6 of the XML 1.0 specification 
 * [XML 1.0]). The nodeName attribute inherited from Node is set to the declared 
 * name of the notation.
 */
class Notation : virtual public Node
{
public:

    /**
     * The public identifier of this notation. If the public identifier was
     * not specified, this is null.
     */
    virtual DOMString getPublicId() = 0;

    /**
     * The system identifier of this notation. If the system identifier was
     * not specified, this is null. This may be an absolute URI or not.
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
 * This interface represents a known entity, either parsed or unparsed, in an XML 
 * document. Note that this models the entity itself not the entity declaration.
 */
class Entity : virtual public Node
{
public:

    /**
     * The public identifier associated with the entity if specified,
     *      and null otherwise.
     */
    virtual DOMString getPublicId() = 0;

    /**
     * The system identifier associated with the entity if specified,
     * and null otherwise. This may be an absolute URI or not.
     */
    virtual DOMString getSystemId() = 0;

    /**
     * For unparsed entities, the name of the notation for the entity.
     * For parsed entities, this is null.
     */
    virtual DOMString getNotationName() = 0;

    /**
     * An attribute specifying the encoding used for this entity at the
     * time of parsing, when it is an external parsed entity. This is null
     * if it an entity from the internal subset or if it is not known.
     */
    virtual DOMString getInputEncoding() = 0;

    /**
     * An attribute specifying, as part of the text declaration, the encoding
     * of this entity, when it is an external parsed entity. This is null otherwise.
     */
    virtual DOMString getXmlEncoding() = 0;

    /**
     * An attribute specifying, as part of the text declaration, the version
     * number of this entity, when it is an external parsed entity.
     * This is null otherwise.
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
 * EntityReference nodes may be used to represent an entity reference in the tree.
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
 * The ProcessingInstruction interface represents a "processing instruction", 
 * used in XML as a way to keep processor-specific information in the text of the 
 * document.
 */
class ProcessingInstruction : virtual public Node
{
public:

    /**
     * The target of this processing instruction. XML defines this as being
     * the first token following the markup that begins the processing instruction.
     */
    virtual DOMString getTarget() = 0;

    /**
     * The content of this processing instruction. This is from the first non
     * white space character after the target to the character immediately
     * preceding the ?>.
     */
    virtual DOMString getData() = 0;

    /**
     *  Sets the content above.
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
 * DocumentFragment is a "lightweight" or "minimal" Document object. It is very 
 * common to want to be able to extract a portion of a document's tree or to 
 * create a new fragment of a document. Imagine implementing a user command like 
 * cut or rearranging a document by moving fragments around. It is desirable to 
 * have an object which can hold such fragments and it is quite natural to use a 
 * Node for this purpose. While it is true that a Document object could fulfill 
 * this role, a Document object can potentially be a heavyweight object, 
 * depending on the underlying implementation. What is really needed for this is 
 * a very lightweight object. DocumentFragment is such an object.
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
 * From the spec:
 *
 * The Document interface represents the entire HTML or XML document. 
 * Conceptually, it is the root of the document tree, and provides the primary 
 * access to the document's data.
 *
 * Since elements, text nodes, comments, processing instructions, etc. cannot 
 * exist outside the context of a Document, the Document interface also contains 
 * the factory methods needed to create these objects. The Node objects created 
 * have a ownerDocument attribute which associates them with the Document within 
 * whose context they were created.
 *
 */
class Document : virtual public Node
{
public:

    /**
     * The Document Type Declaration (see DocumentType) associated with this document. 
     */
    virtual DocumentTypePtr getDoctype() = 0;

    /**
     * The DOMImplementation object that handles this document. A DOM application
     * may use objects from multiple implementations.
     */
    virtual DOMImplementation *getImplementation() = 0;

    /**
     * This is a convenience attribute that allows direct access to the child
     * node that is the document element of the document.
     */
    virtual ElementPtr getDocumentElement() = 0;

    /**
     * Creates an element of the type specified.
     */
    virtual ElementPtr createElement(const DOMString& tagName)
                           throw(DOMException) = 0;

    /**
     * Creates a new, empty DocumentFragment
     */
    virtual DocumentFragmentPtr createDocumentFragment() = 0;

    /**
     * Creates an Text node with the text data specified.
     */
    virtual TextPtr createTextNode(const DOMString& text) = 0;

    /**
     *  Creates a new Comment node with the argument text
     */
    virtual CommentPtr createComment(const DOMString& text) = 0;

    /**
     * Creates a new CDATASection node with the argument text
     */
    virtual CDATASectionPtr createCDATASection(const DOMString& text)
                                     throw(DOMException) = 0;

    /**
     * Creates a new ProcessingInstruction
     */
    virtual ProcessingInstructionPtr
	           createProcessingInstruction(const DOMString& target,
                                           const DOMString& data)
                                           throw(DOMException) = 0;

    /**
     *  Creates a new Attr with the given name, but no value.
     */
    virtual AttrPtr createAttribute(const DOMString& name)
                          throw(DOMException) = 0;

    /**
     * Creates a new EntityReference
     */
    virtual EntityReferencePtr createEntityReference(const DOMString& name)
                                           throw(DOMException) = 0;

    /**
     * Searches the Document in document order for all elements with the given
     *      tag name
     */
    virtual NodeList getElementsByTagName(const DOMString& tagname) = 0;


    /**
     * Imports a node from another document to this document, without altering or
     * removing the source node from the original document; this method creates a new
     * copy of the source node. The returned node has no parent; (parentNode is
     * null). For all nodes, importing a node creates a node object owned by the
     * importing document, with attribute values identical to the source node's
     * nodeName and nodeType, plus the attributes related to namespaces (prefix,
     * localName, and namespaceURI). As in the cloneNode operation, the source node
     * is not altered. User data associated to the imported node is not carried over.
     * However, if any UserDataHandlers has been specified along with the associated
     * data these handlers will be called with the appropriate parameters before this
     * method returns. Additional information is copied as appropriate to the
     * nodeType, attempting to mirror the behavior expected if a fragment of XML or
     * HTML source was copied from one document to another, recognizing that the two
     * documents may have different DTDs in the XML case. The following list
     * describes the specifics for each type of node.
     */
    virtual NodePtr importNode(const NodePtr importedNode,
                     bool deep)
                     throw(DOMException) = 0;

    /**
     *  Creates a new Element with the given namespace and qualifiedName.
     *  Use "" for no namespace
     */
    virtual ElementPtr createElementNS(const DOMString& namespaceURI,
                             const DOMString& qualifiedName)
                             throw(DOMException) = 0;

    /**
     * Creates a new Attr with the given namespace and qualifiedName.
     */
    virtual AttrPtr createAttributeNS(const DOMString& namespaceURI,
                            const DOMString& qualifiedName)
                            throw(DOMException) = 0;

    /**
     * Searches the Document in document order for all elements with the given
     *    namespace and tag name
     */
    virtual NodeList getElementsByTagNameNS(const DOMString& namespaceURI,
                                     const DOMString& localName) = 0;

    /**
     *  Gets the element with the given id if it exists, else null.
     */
    virtual ElementPtr getElementById(const DOMString& elementId) = 0;


    /**
     * Return the input encoding of this Document
     */
    virtual DOMString getInputEncoding() = 0;


    /**
     * Return the XML encoding of this Document
     */
    virtual DOMString getXmlEncoding() = 0;

    /**
     * An attribute specifying, as part of the XML declaration, whether
     *   this document is standalone. This is false when unspecified.
     */
    virtual bool getXmlStandalone() = 0;

    /**
     * Sets whether this is a standalone XML document.  No validation is
     *    done here.
     */
    virtual void setXmlStandalone(bool val) throw (DOMException) = 0;

    /**
     *  Gets the version (1.0, 1.1, etc) of this document.
     */
    virtual DOMString getXmlVersion() = 0;

    /**
     *  Sets the XML version of this document.
     */
    virtual void setXmlVersion(const DOMString &version)
	                           throw (DOMException) = 0;

    /**
     * An attribute specifying whether error checking is enforced or not. When set to 
     * false, the implementation is free to not test every possible error case 
     * normally defined on DOM operations, and not raise any DOMException on DOM 
     * operations or report errors while using Document.normalizeDocument(). In case 
     * of error, the behavior is undefined. This attribute is true by default.   
     */
    virtual bool getStrictErrorChecking() = 0;

    /**
     * Sets the value described above.
     */
    virtual void setStrictErrorChecking(bool val) = 0;


    /**
     * Gets the document URI (the base location) of this Document.
     */
    virtual DOMString getDocumentURI() = 0;

    /**
     * Sets the document URI (the base location) of this Document to the
     * argument uri.     
     */
    virtual void setDocumentURI(const DOMString &uri) = 0;

    /**
     * Attempts to adopt a node from another document to this document. If supported, 
     * it changes the ownerDocument of the source node, its children, as well as the 
     * attached attribute nodes if there are any. If the source node has a parent it 
     * is first removed from the child list of its parent. This effectively allows 
     * moving a subtree from one document to another (unlike importNode() which 
     * create a copy of the source node instead of moving it). When it fails, 
     * applications should use Document.importNode() instead. Note that if the 
     * adopted node is already part of this document (i.e. the source and target 
     * document are the same), this method still has the effect of removing the 
     * source node from the child list of its parent, if any.   
     */
    virtual NodePtr adoptNode(const NodePtr source) throw (DOMException) = 0;

    /**
     *  Get the configuration item associated with this Document
     */
    virtual DOMConfiguration *getDomConfig() = 0;

    /**
     * This method acts as if the document was going through a save and load cycle, 
     * putting the document in a "normal" form. As a consequence, this method updates 
     * the replacement tree of EntityReference nodes and normalizes Text nodes, as 
     * defined in the method Node.normalize(). Otherwise, the actual result depends 
     * on the features being set on the Document.domConfig object and governing what 
     * operations actually take place. Noticeably this method could also make the 
     * document namespace well-formed according to the algorithm described in 
     * Namespace Normalization, check the character normalization, remove the 
     * CDATASection nodes, etc. See DOMConfiguration for details.
     */
    virtual void normalizeDocument() = 0;

    /**
     * 
     * Rename an existing node of type ELEMENT_NODE or ATTRIBUTE_NODE. When possible 
     * this simply changes the name of the given node, otherwise this creates a new 
     * node with the specified name and replaces the existing node with the new node 
     * as described below. If simply changing the name of the given node is not 
     * possible, the following operations are performed: a new node is created, any 
     * registered event listener is registered on the new node, any user data 
     * attached to the old node is removed from that node, the old node is removed 
     * from its parent if it has one, the children are moved to the new node, if the 
     * renamed node is an Element its attributes are moved to the new node, the new 
     * node is inserted at the position the old node used to have in its parent's 
     * child nodes list if it has one, the user data that was attached to the old 
     * node is attached to the new node. When the node being renamed is an Element 
     * only the specified attributes are moved, default attributes originated from 
     * the DTD are updated according to the new element name. In addition, the 
     * implementation may update default attributes from other schemas. Applications 
     * should use Document.normalizeDocument() to guarantee these attributes are 
     * up-to-date. When the node being renamed is an Attr that is attached to an 
     * Element, the node is first removed from the Element attributes map. Then, once 
     * renamed, either by modifying the existing node or creating a new one as 
     * described above, it is put back.
     *
     * In addition,
     * a user data event NODE_RENAMED is fired,
     * when the implementation supports the feature "MutationNameEvents",
     * each mutation operation involved in this method fires the appropriate
     * event, and in the end the event {http://www.w3.org/2001/xml-events,
     * DOMElementNameChanged} or {http://www.w3.org/2001/xml-events,
     * DOMAttributeNameChanged} is fired.
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




