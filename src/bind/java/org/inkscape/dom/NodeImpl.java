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



public class NodeImpl extends DOMBase
                      implements org.w3c.dom.Node
{

/**
 * Adds the node newChild to the end of the list of children of this node.
 */
public Node appendChild(Node newChild)
{
    return null;
}


/**
 * Returns a duplicate of this node, i.e., serves as a generic copy
 *  constructor for nodes.
 */
public Node cloneNode(boolean deep)
{
    return null;
}

/**
 * Compares the reference node, i.e.
 */
public short compareDocumentPosition(Node other)
{
    return 0;
}

/**
 * A NamedNodeMap containing the attributes of this node (if it is
 *  an Element) or null otherwise.
 */
public NamedNodeMap getAttributes()
{
    return null;
}

/**
 * The absolute base URI of this node or null if the
 *  implementation wasn't able to obtain an absolute URI.
 */
public String getBaseURI()
{
    return "";
}

/**
 * A NodeList that contains all children of this node.
 */
public NodeList getChildNodes()
{
    return null;
}

/**
 * This method returns a specialized object which implements the
 * specialized APIs of the specified feature and version, as specified in .
 */
public Object getFeature(String feature, String version)
{
    return null;
}

/**
 * The first child of this node.
 */
public Node getFirstChild()
{
    return null;
}

/**
 * The last child of this node.
 */
public Node getLastChild()
{
    return null;
}

/**
 * Returns the local part of the qualified name of this node.
 */
public String getLocalName()
{
    return "";
}

/**
 * The namespace URI of this node, or null if it is unspecified (see ).
 */
public String getNamespaceURI()
{
    return "";
}

/**
 * The node immediately following this node.
 */
public Node getNextSibling()
{
    return null;
}

/**
 * The name of this node, depending on its type; see the table above.
 */
public String getNodeName()
{
    return "";
}

/**
 * A code representing the type of the underlying object,
 *  as defined above.
 */
public short getNodeType()
{
    return 0;
}

/**
 * The value of this node, depending on its type; see the table above.
 */
public String getNodeValue()
{
    return "";
}

/**
 * The Document object associated with this node.
 */
public Document getOwnerDocument()
{
    return null;
}

/**
 * The parent of this node.
 */
public Node getParentNode()
{
    return null;
}

/**
 * The namespace prefix of this node, or null if it is unspecified.
 */
public String getPrefix()
{
    return "";
}

/**
 * The node immediately preceding this node.
 */
public Node getPreviousSibling()
{
    return null;
}

/**
 * This attribute returns the text content of this node
 *  and its descendants.
 */
public String getTextContent()
{
    return "";
}

/**
 * Retrieves the object associated to a key on a this node.
 */
public Object getUserData(String key)
{
    return null;
}

/**
 * Returns whether this node (if it is an element) has any attributes.
 */
public boolean hasAttributes()
{
    return false;
}

/**
 * Returns whether this node has any children.
 */
public boolean hasChildNodes()
{
    return false;
}

/**
 * Inserts the node newChild before the existing child node refChild.
 */
public Node insertBefore(Node newChild, Node refChild)
{
    return null;
}

/**
 * This method checks if the specified namespaceURI is the
 *  default namespace or not.
 */
public boolean isDefaultNamespace(String namespaceURI)
{
    return false;
}

/**
 * Tests whether two nodes are equal.
 */
public boolean isEqualNode(Node arg)
{
    return false;
}

/**
 * Returns whether this node is the same node as the given one.
 */
public boolean isSameNode(Node other)
{
    return false;
}

/**
 * Tests whether the DOM implementation implements a specific feature
 *  and that feature is supported by this node, as specified in .
 */
public boolean isSupported(String feature, String version)
{
    return false;
}

/**
 * Look up the namespace URI associated to the given prefix,
 *  starting from this node.
 */
public String lookupNamespaceURI(String prefix)
{
    return "";
}

/**
 * Look up the prefix associated to the given namespace URI, starting
 *  from this node.
 */
public String lookupPrefix(String namespaceURI)
{
    return "";
}

/**
 * Puts all Text nodes in the full depth of the sub-tree underneath
 *  this Node, including attribute nodes, into a "normal" form where only structure (e.g., elements, comments, processing instructions, CDATA sections, and entity references) separates Text nodes, i.e., there are neither adjacent Text nodes nor empty Text nodes.
 */
public void normalize()
{
}

/**
 * Removes the child node indicated by oldChild from the list of
 *  children, and returns it.
 */
public Node removeChild(Node oldChild)
{
    return null;
}

/**
 * Replaces the child node oldChild with newChild in the list of
 *  children, and returns the oldChild node.
 */
public Node replaceChild(Node newChild, Node oldChild)
{
    return null;
}

/**
 * The value of this node, depending on its type; see the table above.
 */
public void setNodeValue(String nodeValue)
{
}

/**
 * The namespace prefix of this node, or null if it is unspecified.
 */
public void setPrefix(String prefix)
{
}

/**
 * This attribute returns the text content of this node and
 *  its descendants.
 */
public void setTextContent(String textContent)
{
}

/**
 * Associate an object to a key on this node.
 */
public Object setUserData(String key, Object data, UserDataHandler handler)
{
    return null;
}


public NodeImpl()
{
    super();
}



}
