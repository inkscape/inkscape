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
import org.w3c.dom.DocumentType;



public class DOMImplementationImpl
       implements org.w3c.dom.DOMImplementation
{

public native boolean hasFeature(String feature,
                              String version);


public native DocumentType createDocumentType(String qualifiedName,
                                           String publicId, 
                                           String systemId)
                                           throws DOMException;


public native Document createDocument(String namespaceURI,
                                   String qualifiedName, 
                                   DocumentType doctype)
                                   throws DOMException;


public native Object getFeature(String feature,
                             String version);

}
