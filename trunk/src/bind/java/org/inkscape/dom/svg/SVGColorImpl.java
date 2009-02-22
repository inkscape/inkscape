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
 *  Note that these SVG files are implementations of the Java
 *  interface package found here:
 *      http://www.w3.org/TR/SVG/java.html
 */


package org.inkscape.dom.svg;

import org.w3c.dom.svg.SVGException;
import org.w3c.dom.css.RGBColor;
import org.w3c.dom.css.CSSValue;

import org.w3c.dom.svg.SVGICCColor;

public class SVGColorImpl
       extends
               org.inkscape.dom.css.CSSValueImpl
       implements org.w3c.dom.svg.SVGColor
{

public native short getColorType( );
public native RGBColor getRGBColor( );
public native SVGICCColor getICCColor( );

public native void setRGBColor ( String rgbColor )
                  throws SVGException;
public native void setRGBColorICCColor ( String rgbColor, String iccColor )
                  throws SVGException;
public native void setColor ( short colorType, String rgbColor, String iccColor )
                  throws SVGException;
}
