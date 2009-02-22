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
import org.w3c.dom.svg.SVGException;
import org.w3c.dom.svg.SVGMatrix;


public class SVGMatrixImpl
       implements org.w3c.dom.svg.SVGMatrix
{
public native float getA( );
public native void setA( float a )
                     throws DOMException;
public native float getB( );
public native void setB( float b )
                     throws DOMException;
public native float getC( );
public native void setC( float c )
                     throws DOMException;
public native float getD( );
public native void setD( float d )
                     throws DOMException;
public native float getE( );
public native void setE( float e )
                     throws DOMException;
public native float getF( );
public native void setF( float f )
                     throws DOMException;

public native SVGMatrix multiply ( SVGMatrix secondMatrix );
public native SVGMatrix inverse (  )
                throws SVGException;
public native SVGMatrix translate ( float x, float y );
public native SVGMatrix scale ( float scaleFactor );
public native SVGMatrix scaleNonUniform ( float scaleFactorX, float scaleFactorY );
public native SVGMatrix rotate ( float angle );
public native SVGMatrix rotateFromVector ( float x, float y )
                throws SVGException;
public native SVGMatrix flipX (  );
public native SVGMatrix flipY (  );
public native SVGMatrix skewX ( float angle );
public native SVGMatrix skewY ( float angle );
}
