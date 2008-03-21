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

import org.w3c.dom.DOMException;

import org.w3c.dom.svg.*;

import org.w3c.dom.css.CSSStyleDeclaration;
import org.w3c.dom.css.CSSValue;


public class SVGGlyphRefElementImpl
       extends
               SVGElementImpl
               //SVGURIReference,
               //SVGStylable
       implements org.w3c.dom.svg.SVGGlyphRefElement
{
//from SVGURIReference
public native SVGAnimatedString getHref( );
//end SVGURIReference

//from SVGStylable
public native SVGAnimatedString getClassName( );
public native CSSStyleDeclaration getStyle( );
public native CSSValue getPresentationAttribute ( String name );
//end SVGStylable


public native String getGlyphRef( );
public native void setGlyphRef( String glyphRef )
                       throws DOMException;
public native String getFormat( );
public native void setFormat( String format )
                       throws DOMException;
public native float getX( );
public native void setX( float x )
                       throws DOMException;
public native float getY( );
public native void setY( float y )
                       throws DOMException;
public native float getDx( );
public native void setDx( float dx )
                       throws DOMException;
public native float getDy( );
public native void setDy( float dy )
                       throws DOMException;
}
