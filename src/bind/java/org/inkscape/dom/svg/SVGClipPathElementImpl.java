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




public class SVGClipPathElementImpl
       extends
               SVGElementImpl
               //SVGTests,
               //SVGLangSpace,
               //SVGExternalResourcesRequired,
               //SVGStylable,
               //SVGTransformable,
               //SVGUnitTypes
        implements org.w3c.dom.svg.SVGClipPathElement
{
//from SVGTests
public native SVGStringList getRequiredFeatures( );
public native SVGStringList getRequiredExtensions( );
public native SVGStringList getSystemLanguage( );
public native boolean hasExtension ( String extension );
//end SVGTests

//from SVGLangSpace
public native String getXMLlang( );
public native void setXMLlang( String xmllang )
                       throws DOMException;
public native String getXMLspace( );
public native void setXMLspace( String xmlspace )
                       throws DOMException;
//end SVGLangSpace

//from SVGExternalResourcesRequired
public native SVGAnimatedBoolean getExternalResourcesRequired( );
//end SVGExternalResourcesRequired

//from SVGStylable
public native SVGAnimatedString getClassName( );
public native CSSStyleDeclaration getStyle( );
public native CSSValue getPresentationAttribute ( String name );
//end SVGStylable

//from SVGTransformable
public native SVGAnimatedTransformList getTransform( );
//end SVGTransformable

//from SVGLocatable (from SVGTransformable)
public native SVGElement getNearestViewportElement( );
public native SVGElement getFarthestViewportElement( );

public native SVGRect   getBBox (  );
public native SVGMatrix getCTM (  );
public native SVGMatrix getScreenCTM (  );
public native SVGMatrix getTransformToElement ( SVGElement element )
                  throws SVGException;
//end SVGLocatable


public native SVGAnimatedEnumeration getClipPathUnits( );

}
