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



public class ElementImpl extends NodeImpl
                implements org.w3c.dom.Element
{


/**
 * Retrieves an attribute value by name.
 */
public String getAttribute(String name)
{
    return "";
}

/**
 * Retrieves an attribute node by name.
 */
public Attr getAttributeNode(String name)
{
    return null;
}

/**
 * Retrieves an Attr node by local name and namespace URI.
 */
public Attr getAttributeNodeNS(String namespaceURI, String localName)
{
    return null;
}

/**
 * Retrieves an attribute value by local name and namespace URI.
 */
public String getAttributeNS(String namespaceURI, String localName)
{
    return "";
}

/**
 * Returns a NodeList of all descendant Elements with a given
 * tag name, in document order.
 */
public NodeList getElementsByTagName(String name)
{
    return null;
}

/**
 * Returns a NodeList of all the descendant Elements with a given
 *  local name and namespace URI in document order.
 */
public NodeList getElementsByTagNameNS(String namespaceURI,
                                       String localName)
{
    return null;
}

/**
 * The type information associated with this element.
 */
public TypeInfo getSchemaTypeInfo()
{
    return null;
}

/**
 * The name of the element.
 */
public String getTagName()
{
    return "";
}

/**
 * Returns true when an attribute with a given name is
 * specified on this element or has a default value, false otherwise.
 */
public boolean hasAttribute(String name)
{
    return false;
}

/**
 * Returns true when an attribute with a given local name and
 * namespace URI is specified on this element or has a default value,
 * false otherwise.
 */
public boolean hasAttributeNS(String namespaceURI, String localName)
{
    return false;
}

/**
 * Removes an attribute by name.
 */
public void removeAttribute(String name)
{
}

/**
 * Removes the specified attribute node.
 */
public Attr removeAttributeNode(Attr oldAttr)
{
    return null;
}

/**
 * Removes an attribute by local name and namespace URI.
 */
public void removeAttributeNS(String namespaceURI, String localName)
{
}

/**
 * Adds a new attribute.
 */
public void setAttribute(String name, String value)
{
}

/**
 * Adds a new attribute node.
 */
public Attr setAttributeNode(Attr newAttr)
{
    return null;
}

/**
 * Adds a new attribute.
 */
public Attr setAttributeNodeNS(Attr newAttr)
{
    return null;
}

/**
 * Adds a new attribute.
 */
public void setAttributeNS(String namespaceURI,
                           String qualifiedName, String value)
{
}

/**
 * If the parameter isId is true, this method declares the
 * specified attribute to be a user-determined ID attribute .
 */
public void setIdAttribute(String name, boolean isId)
{
}

/**
 * If the parameter isId is true, this method declares the
 * specified attribute to be a user-determined ID attribute .
 */
public void setIdAttributeNode(Attr idAttr, boolean isId)
{
}

/**
 * If the parameter isId is true, this method declares the specified
 * attribute to be a user-determined ID attribute .
 */
public void setIdAttributeNS(String namespaceURI,
                             String localName, boolean isId)
{
}



public ElementImpl()
{
    super();
}



}
