/**
 * This is a simple mechanism to bind Inkscape to Java, and thence
 * to all of the nice things that can be layered upon that.
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (c) 2007-2008 Inkscape.org
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
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
 *  Note that these DOM files are implementations of the Java
 *  interface package found here:
 *      http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/java-binding.html
 */

package org.inkscape.dom;

import org.w3c.dom.DOMException;
import org.w3c.dom.Attr;
import org.w3c.dom.NodeList;
import org.w3c.dom.TypeInfo;




public class ElementImpl
       extends NodeImpl
       implements org.w3c.dom.Element

{

public native String getTagName();

public native String getAttribute(String name);

public native void setAttribute(String name,
                         String value)
                         throws DOMException;

public native void removeAttribute(String name)
                            throws DOMException;

public native Attr getAttributeNode(String name);

public native Attr setAttributeNode(Attr newAttr)
                             throws DOMException;

public native Attr removeAttributeNode(Attr oldAttr)
                                throws DOMException;

public native NodeList getElementsByTagName(String name);

public native String getAttributeNS(String namespaceURI,
                             String localName)
                             throws DOMException;

public native void setAttributeNS(String namespaceURI,
                           String qualifiedName,
                           String value)
                           throws DOMException;

public native void removeAttributeNS(String namespaceURI,
                              String localName)
                              throws DOMException;

public native Attr getAttributeNodeNS(String namespaceURI,
                               String localName)
                               throws DOMException;

public native Attr setAttributeNodeNS(Attr newAttr)
                               throws DOMException;

public native NodeList getElementsByTagNameNS(String namespaceURI,
                                       String localName)
                                       throws DOMException;

public native boolean hasAttribute(String name);

public native boolean hasAttributeNS(String namespaceURI,
                              String localName)
                              throws DOMException;

public native TypeInfo getSchemaTypeInfo();

public native void setIdAttribute(String name,
                           boolean isId)
                           throws DOMException;

public native void setIdAttributeNS(String namespaceURI,
                             String localName,
                             boolean isId)
                             throws DOMException;

public native void setIdAttributeNode(Attr idAttr,
                               boolean isId)
                               throws DOMException;

}
