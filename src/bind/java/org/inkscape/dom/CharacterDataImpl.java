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




public class CharacterDataImpl
       extends NodeImpl
       implements org.w3c.dom.CharacterData
{

public native String getData()
                        throws DOMException;
public native void setData(String data)
                        throws DOMException;

public native int getLength();

public native String substringData(int offset,
                            int count)
                            throws DOMException;

public native void appendData(String arg)
                       throws DOMException;

public native void insertData(int offset,
                       String arg)
                       throws DOMException;

public native void deleteData(int offset,
                       int count)
                       throws DOMException;

public native void replaceData(int offset,
                        int count,
                        String arg)
                        throws DOMException;

}
