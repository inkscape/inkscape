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
 *  Note that these files are implementations of the Java
 *  interface package found here:
 *      http://www.w3.org/TR/DOM-Level-2-Style
 */


package org.inkscape.dom.css;

import org.w3c.dom.DOMException;
import org.w3c.dom.css.Counter;
import org.w3c.dom.css.RGBColor;
import org.w3c.dom.css.Rect;


public class CSSPrimitiveValueImpl
       extends CSSValueImpl
       implements org.w3c.dom.css.CSSPrimitiveValue
{

public native short getPrimitiveType();

public native void setFloatValue(short unitType,
                          float floatValue)
                          throws DOMException;

public native float getFloatValue(short unitType)
                           throws DOMException;

public native void setStringValue(short stringType,
                           String stringValue)
                           throws DOMException;

public native String getStringValue()
                             throws DOMException;

public native Counter getCounterValue()
                               throws DOMException;

public native Rect getRectValue()
                         throws DOMException;

public native RGBColor getRGBColorValue()
                                 throws DOMException;

}
