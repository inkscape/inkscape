/**
 * This is a simple mechanism to bind Inkscape to Java, and thence
 * to all of the nice things that can be layered upon that.
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2007 Bob Jamison
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

package org.inkscape.dom;

import org.w3c.dom.*;

public class DocumentImpl extends NodeImpl
     implements org.w3c.dom.Document
{

/**
 * Attempts to adopt a node from another document to this document.
 */
public Node adoptNode(Node source)
{
    return null;
}

/**
 * Creates an Attr of the given name.
 */
public Attr createAttribute(String name)
{
    return null;
}

/**
 * Creates an attribute of the given qualified name and namespace URI.
 */
public Attr createAttributeNS(String namespaceURI, String qualifiedName)
{
    return null;
}

/**
 * Creates a CDATASection node whose value is the specified string.
 */
public CDATASection createCDATASection(String data)
{
    return null;
}

/**
 * Creates a Comment node given the specified string.
 */
public Comment createComment(String data)
{
    return null;
}

/**
 * Creates an empty DocumentFragment object.
 */
public DocumentFragment createDocumentFragment()
{
    return null;
}

/**
 * Creates an element of the type specified.
 */
public Element createElement(String tagName)
{
    return null;
}

/**
 * Creates an element of the given qualified name and namespace URI.
 */
public Element createElementNS(String namespaceURI, String qualifiedName)
{
    return null;
}

/**
 * Creates an EntityReference object.
 */
public EntityReference 	createEntityReference(String name)
{
    return null;
}

/**
 * Creates a ProcessingInstruction node given the specified name
 *  and data strings.
 */
public ProcessingInstruction createProcessingInstruction(String target, String data)
{
    return null;
}

/**
 * Creates a Text node given the specified string.
 */
public Text createTextNode(String data)
{
    return null;
}

/**
 * The Document Type Declaration (see DocumentType) associated
 *  with this document.
 */
public DocumentType getDoctype()
{
    return null;
}

/**
 * This is a convenience attribute that allows direct access to the
 *  child node that is the document element of the document.
 */
public Element getDocumentElement()
{
    return null;
}

/**
 * The location of the document or null if undefined or if the Document
 *  was created using DOMImplementation.createDocument.
 */
public String getDocumentURI()
{
    return "";
}

/**
 * The configuration used when Document.normalizeDocument() is invoked.
 */
public DOMConfiguration getDomConfig()
{
    return null;
}

/**
 * Returns the Element that has an ID attribute with the given value.
 */
public Element getElementById(String elementId)
{
    return null;
}

/**
 * Returns a NodeList of all the Elements in document order with a
 *  given tag name and are contained in the document.
 */
public NodeList getElementsByTagName(String tagname)
{
    return null;
}

/**
 * Returns a NodeList of all the Elements with a given local name
 *  and namespace URI in document order.
 */
public NodeList getElementsByTagNameNS(String namespaceURI, String localName)
{
    return null;
}

/**
 * The DOMImplementation object that handles this document.
 */
public DOMImplementation getImplementation()
{
    return null;
}

/**
 * An attribute specifying the encoding used for this document at
 *  the time of the parsing.
 */
public String getInputEncoding()
{
    return "";
}

/**
 * An attribute specifying whether error checking is enforced or not.
 */
public boolean getStrictErrorChecking()
{
    return false;
}

/**
 * An attribute specifying, as part of the XML declaration,
 *  the encoding of this document.
 */
public String getXmlEncoding()
{
    return "";
}

/**
 * An attribute specifying, as part of the XML declaration,
 *  whether this document is standalone.
 */
public boolean getXmlStandalone()
{
    return false;
}

/**
 * An attribute specifying, as part of the XML declaration,
 *  the version number of this document.
 */
public String getXmlVersion()
{
    return "";
}

/**
 * Imports a node from another document to this document,
 *  without altering or removing the source node from the
 *   original document; this method creates a
 *    new copy of the source node.
 */
public Node importNode(Node importedNode, boolean deep)
{
    return null;
}

/**
 * This method acts as if the document was going through
 *  a save and load cycle, putting the document in a "normal" form.
 */
public void normalizeDocument()
{
}

/**
 * Rename an existing node of type ELEMENT_NODE or ATTRIBUTE_NODE.
 */
public Node renameNode(Node n, String namespaceURI, String qualifiedName)
{
    return null;
}

/**
 * The location of the document or null if undefined or if
 *  the Document was created using DOMImplementation.createDocument.
 */
public void setDocumentURI(String documentURI)
{
}

/**
 * An attribute specifying whether error checking is enforced or not.
 */
public void setStrictErrorChecking(boolean strictErrorChecking)
{
}

/**
 * An attribute specifying, as part of the XML declaration,
 *  whether this document is standalone.
 */
public void setXmlStandalone(boolean xmlStandalone)
{
}

/**
 * An attribute specifying, as part of the XML declaration,
 *  the version number of this document.
 */
public void setXmlVersion(String xmlVersion)
{
}


public DocumentImpl()
{
    super();
}

}

