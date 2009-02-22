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
import org.w3c.dom.CDATASection;
import org.w3c.dom.Comment;
import org.w3c.dom.DocumentFragment;
import org.w3c.dom.DocumentType;
import org.w3c.dom.DOMConfiguration;
import org.w3c.dom.DOMImplementation;
import org.w3c.dom.DOMStringList;
import org.w3c.dom.Element;
import org.w3c.dom.EntityReference;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.w3c.dom.ProcessingInstruction;
import org.w3c.dom.Text;



public class DocumentImpl
       extends NodeImpl
       implements org.w3c.dom.Document
{

public native DocumentType getDoctype();

public native DOMImplementation getImplementation();

public native Element getDocumentElement();

public native Element createElement(String tagName)
                             throws DOMException;

public native DocumentFragment createDocumentFragment();

public native Text createTextNode(String data);

public native Comment createComment(String data);

public native CDATASection createCDATASection(String data)
                                       throws DOMException;

public native ProcessingInstruction createProcessingInstruction(String target,
                                                         String data)
                                                         throws DOMException;

public native Attr createAttribute(String name)
                            throws DOMException;

public native EntityReference createEntityReference(String name)
                                             throws DOMException;

public native NodeList getElementsByTagName(String tagname);

public native Node importNode(Node importedNode,
                       boolean deep)
                       throws DOMException;

public native Element createElementNS(String namespaceURI,
                               String qualifiedName)
                               throws DOMException;

public native Attr createAttributeNS(String namespaceURI,
                              String qualifiedName)
                              throws DOMException;

public native NodeList getElementsByTagNameNS(String namespaceURI,
                                       String localName);

public native Element getElementById(String elementId);

public native String getInputEncoding();

public native String getXmlEncoding();

public native boolean getXmlStandalone();

public native void setXmlStandalone(boolean xmlStandalone)
                              throws DOMException;

public native String getXmlVersion();

public native void setXmlVersion(String xmlVersion)
                              throws DOMException;

public native boolean getStrictErrorChecking();

public native void setStrictErrorChecking(boolean strictErrorChecking);

public native String getDocumentURI();

public native void setDocumentURI(String documentURI);

public native Node adoptNode(Node source)
                      throws DOMException;

public native DOMConfiguration getDomConfig();

public native void normalizeDocument();

public native Node renameNode(Node n,
                       String namespaceURI,
                       String qualifiedName)
                       throws DOMException;

}
