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

import org.w3c.dom.Document;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.w3c.dom.UserDataHandler;




public class NodeImpl
       implements org.w3c.dom.Node
{

public native String getNodeName();

public native String getNodeValue()
                          throws DOMException;
public native void setNodeValue(String nodeValue)
                          throws DOMException;

public native short getNodeType();

public native Node getParentNode();

public native NodeList getChildNodes();

public native Node getFirstChild();

public native Node getLastChild();

public native Node getPreviousSibling();

public native Node getNextSibling();

public native NamedNodeMap getAttributes();

public native Document getOwnerDocument();

public native Node insertBefore(Node newChild,
                         Node refChild)
                         throws DOMException;

public native Node replaceChild(Node newChild,
                         Node oldChild)
                         throws DOMException;

public native Node removeChild(Node oldChild)
                        throws DOMException;

public native Node appendChild(Node newChild)
                        throws DOMException;

public native boolean hasChildNodes();

public native Node cloneNode(boolean deep);

public native void normalize();

public native boolean isSupported(String feature,
                           String version);

public native String getNamespaceURI();

public native String getPrefix();

public native void setPrefix(String prefix)
                           throws DOMException;

public native String getLocalName();

public native boolean hasAttributes();

public native String getBaseURI();


public native short compareDocumentPosition(Node other)
                                     throws DOMException;


public native String getTextContent()
                                     throws DOMException;

public native void setTextContent(String textContent)
                                     throws DOMException;


public native boolean isSameNode(Node other);

public native String lookupPrefix(String namespaceURI);

public native boolean isDefaultNamespace(String namespaceURI);

public native String lookupNamespaceURI(String prefix);

public native boolean isEqualNode(Node arg);

public native Object getFeature(String feature,
                         String version);

public native Object setUserData(String key,
                          Object data,
                          UserDataHandler handler);

public native Object getUserData(String key);

}
