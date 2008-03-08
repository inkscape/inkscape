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


public class DOMImplementationImpl
              extends DOMBase
              implements org.w3c.dom.DOMImplementation
{

/**
 * Creates a DOM Document object of the specified type
 *  with its document element.
 */
public Document createDocument(String namespaceURI,
                               String qualifiedName,
                               DocumentType doctype)
{
    return null;
}

/**
 * Creates an empty DocumentType node.
 */
public DocumentType	createDocumentType(String qualifiedName,
                                       String publicId,
                                       String systemId)
{
    return null;
}

/**
 * This method returns a specialized object which implements the
 * specialized APIs of the specified feature and version, as specified
 * in DOM Features.
 */
public Object getFeature(String feature, String version)
{
    return null;
}

/**
 * Test if the DOM implementation implements a specific
 * feature and version, as specified in DOM Features.
 */
public boolean hasFeature(String feature, String version)
{
    return false;
}


/**
 *
 */
public DOMImplementationImpl()
{
    super();
}

}
