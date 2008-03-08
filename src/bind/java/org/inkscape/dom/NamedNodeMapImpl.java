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



public class NamedNodeMapImpl
             extends DOMBase
             implements org.w3c.dom.NamedNodeMap
{



/**
 * The number of nodes in this map.
 */
public int getLength()
{
    return 0;
}

/**
 * Retrieves a node specified by name.
 */
public Node getNamedItem(String name)
{
    return null;
}

/**
 * Retrieves a node specified by local name and namespace URI.
 */
public Node getNamedItemNS(String namespaceURI, String localName)
{
    return null;
}

/**
 * Returns the indexth item in the map.
 */
public Node item(int index)
{
    return null;
}

/**
 * Removes a node specified by name.
 */
public Node removeNamedItem(String name)
{
    return null;
}

/**
 * Removes a node specified by local name and namespace URI.
 */
public Node removeNamedItemNS(String namespaceURI, String localName)
{
    return null;
}

/**
 * Adds a node using its nodeName attribute.
 */
public Node setNamedItem(Node arg)
{
    return null;
}

/**
 * Adds a node using its namespaceURI and localName.
 */
public Node setNamedItemNS(Node arg)
{
    return null;
}



public NamedNodeMapImpl()
{
    super();
}

}
