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


public class SMILRegionElementImpl
       extends SMILElementImpl
             //ElementLayout
       implements org.w3c.dom.smil.SMILRegionElement
{

public SMILRegionElementImpl()
{
    imbue(_ElementLayout = new ElementLayoutImpl());
}

//from ElementLayout
ElementLayoutImpl _ElementLayout;
public String getTitle()
    { return _ElementLayout.getTitle(); }
public void setTitle(String title) throws DOMException
    { _ElementLayout.setTitle(title); }
public String getBackgroundColor()
    { return _ElementLayout.getBackgroundColor(); }
public void setBackgroundColor(String backgroundColor) throws DOMException
    { _ElementLayout.setBackgroundColor(backgroundColor); }
public int getHeight()
    { return _ElementLayout.getHeight(); }
public void setHeight(int height) throws DOMException
    { _ElementLayout.setHeight(height); }
public int getWidth()
    { return _ElementLayout.getWidth(); }
public void setWidth(int width) throws DOMException
    { _ElementLayout.setWidth(width); }
//end ElementLayout


public native String getFit();
public native void setFit(String fit) throws DOMException;

public native String getTop();
public native void setTop(String top) throws DOMException;

public native int getZIndex();
public native void setZIndex(int zIndex) throws DOMException;

}

