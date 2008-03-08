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



public class DocumentTypeImpl extends NodeImpl
       implements org.w3c.dom.DocumentType
{


/**
 * A NamedNodeMap containing the general entities, both
 *  external and internal, declared in the DTD.
 */
public NamedNodeMap getEntities()
{
    return null;
}

/**
 * The internal subset as a string, or null if there is none.
 */
public String getInternalSubset()
{
    return "";
}

/**
 * The name of DTD; i.e., the name immediately following the
 *  DOCTYPE keyword.
 */
public String getName()
{
    return "";
}

/**
 * A NamedNodeMap containing the notations declared in the DTD.
 */
public NamedNodeMap getNotations()
{
    return null;
}

/**
 * The public identifier of the external subset.
 */
public String getPublicId()
{
    return null;
}

/**
 * The system identifier of the external subset.
 */
public String getSystemId()
{
    return "";
}



public DocumentTypeImpl()
{
    super();
}

}
