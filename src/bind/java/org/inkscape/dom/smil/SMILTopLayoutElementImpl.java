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
 *  Note that the SMIL files are implementations of the Java
 *  interface package found here:
 *      http://www.w3.org/TR/smil-boston-dom/java-binding.html
 */

package org.inkscape.dom.smil;

import org.w3c.dom.DOMException;



public class SMILTopLayoutElementImpl
       extends SMILElementImpl //, ElementLayout
       implements org.w3c.dom.smil.SMILTopLayoutElement
{
//from ElementLayout
public native String getTitle();
public native void setTitle(String title)
                                  throws DOMException;

public native String getBackgroundColor();
public native void setBackgroundColor(String backgroundColor)
                                  throws DOMException;

public native int getHeight();
public native void setHeight(int height)
                                  throws DOMException;

public native int getWidth();
public native void setWidth(int width)
                                  throws DOMException;
//end ElementLayout
}

