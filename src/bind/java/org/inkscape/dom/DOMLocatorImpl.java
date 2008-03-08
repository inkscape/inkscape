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



public class DOMLocatorImpl
             extends DOMBase
             implements org.w3c.dom.DOMLocator
{


/**
 * The byte offset into the input source this locator is pointing to
 * or -1 if there is no byte offset available.
 */
public int getByteOffset()
{
    return 0;
}


/**
 * The column number this locator is pointing to,
 * or -1 if there is no column number available.
 */
public int getColumnNumber()
{
    return 0;
}


/**
 * The line number this locator is pointing to, or -1 if
 * there is no column number available.
 */
public int getLineNumber()
{
    return 0;
}


/**
 * The node this locator is pointing to, or null if no node is available.
 */
public Node getRelatedNode()
{
    return null;
}


/**
 * The URI this locator is pointing to, or null if no URI is available.
 */
public String getUri()
{
    return "";
}


/**
 * The UTF-16, as defined in [Unicode] and
 * Amendment 1 of [ISO/IEC 10646], offset into the input source
 * this locator is pointing to or -1 if there is no UTF-16
 * offset available.
 */
public int getUtf16Offset()
{
    return 0;
}



public DOMLocatorImpl()
{
    super();
}

}
